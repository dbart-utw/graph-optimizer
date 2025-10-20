#include "pagerank.hpp"

__device__ float diff = 0.0;

void resetDiff()
{
    const float val = 0.0;
    cudaMemcpyToSymbol(diff, &val, sizeof val);
}

float getDiff()
{
    float val;
    cudaMemcpyFromSymbol(&val, diff, sizeof val);
    return val;
}

inline void (*getCSRKernel(GPU_Implementation impl))(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, float*, float*) {
    switch (impl) {
        case VERTEX_PULL:
        return vertexPullPageRank;
        case VERTEX_PULL_NODIV:
        return vertexPullNoDivPageRank;
        case VERTEX_PULL_WARP:
        return vertexPullWarpPageRank;
        case VERTEX_PULL_WARP_NODIV:
        return vertexPullWarpNoDivPageRank;
        case VERTEX_PUSH:
            return vertexPushPageRank;
        case VERTEX_PUSH_WARP:
            return vertexPushWarpPageRank;
        default:
            return nullptr; // Handle invalid cases
    }
}

inline void (*getEdgeListStructKernel(GPU_Implementation impl))(int32_t, int32_t*, int32_t*, int32_t*, float*, float*) {
    switch (impl) {
        case EDGELIST:
            return edgeListPageRank;
        case REV_EDGELIST:
            return revEdgeListPageRank;
        default:
            return nullptr; // Handle invalid cases
    }
}

inline void (*getStructEdgeListKernel(GPU_Implementation impl))(int32_t, EdgeStruct*, int32_t*, float*, float*) {
    switch (impl) {
        case STRUCT_EDGELIST:
            return structEdgeListPageRank;
        case REV_STRUCT_EDGELIST:
            return revStructEdgeListPageRank;
        default:
            return nullptr; // Handle invalid cases
    }
}

template <typename T>
inline int32_t *getNormalizedIndex(CSRGraph<T> &g, GPU_Implementation impl) {
    int32_t *normalized_index = new int32_t[g.num_nodes() + 1];
    int32_t **index;
    switch (impl) {
        case VERTEX_PUSH:
            index = g.out_index();
        case VERTEX_PULL:
            index = g.in_index();
        case VERTEX_PULL_NODIV:
            index = g.in_index();
        case VERTEX_PULL_WARP:
            index = g.in_index();
        case VERTEX_PULL_WARP_NODIV:
            index = g.in_index();
        case VERTEX_PUSH_WARP:
            index = g.out_index();
    }

    for (int i = 0; i < g.num_nodes()+1; i++) {
        normalized_index[i] = index[i] - index[0];
    }

    return normalized_index;
}

template <typename T>
inline T *getNeighs(CSRGraph<T> &g, GPU_Implementation impl) {
    switch (impl) {
        case VERTEX_PUSH:
            return g.out_neighs();
        case VERTEX_PULL:
            return g.in_neighs();
        case VERTEX_PULL_NODIV:
            return g.in_neighs();
        case VERTEX_PULL_WARP:
            return g.in_neighs();
        case VERTEX_PULL_WARP_NODIV:
            return g.in_neighs();
        case VERTEX_PUSH_WARP:
            return g.out_neighs();
        default:
            return nullptr; // Handle invalid cases
    }
}

template <typename T>
inline int32_t *getOutDegrees(CSRGraph<T> &g) {
    int32_t *out_degrees = new int32_t[g.num_nodes()];
    for (int i = 0; i < g.num_nodes(); i++) {
        out_degrees[i] = (int32_t)g.out_degree(i);
    }

    return out_degrees;
}

static __device__ __forceinline__
void updateDiff(float val)
{
    int lane = threadIdx.x % warpSize;

    for (int offset = warpSize/2; offset > 0; offset /= 2) {
        val += __shfl_down_sync(0xffffffff, val, offset);
    }

    if (lane == 0) atomicAdd(&diff, val);
}

__global__ void consolidateRank(int32_t size, float *pagerank, float *new_pagerank) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        float new_rank = ((1.0 - dampening) / size) + (dampening * new_pagerank[idx]);
        float my_diff = fabsf(new_rank - pagerank[idx]);

        pagerank[idx] = new_rank;
        new_pagerank[idx] = 0.0f;
        updateDiff(my_diff);
    }
}

__global__ void consolidateRankNoDiv(int32_t size, int32_t* degrees, float *pagerank, float *new_pagerank, bool notLast) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        float new_rank = ((1 - dampening) / size) + (dampening * new_pagerank[idx]);
        float my_diff = fabsf(new_rank - pagerank[idx]);

        int32_t degree = degrees[idx];

        if (degree != 0 && notLast) new_rank = new_rank / degree;
        pagerank[idx] = new_rank;
        new_pagerank[idx] = 0.0f;

        updateDiff(my_diff);
    }
}

__global__ void edgeListPageRank(int32_t size, int32_t *sources, int32_t *destinations, int32_t *degrees, float *pagerank, float *new_pagerank) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        int32_t source = sources[idx];
        int32_t destination = destinations[idx];

        int32_t degree = degrees[source];
        float new_rank = 0.0f;

        if (degree != 0) new_rank = pagerank[source] / degree;

        atomicAdd(&new_pagerank[destination], new_rank);
    }
}

__global__ void revEdgeListPageRank(int32_t size, int32_t *sources, int32_t *destinations, int32_t *degrees, float *pagerank, float *new_pagerank) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        int32_t source = destinations[idx];
        int32_t destination = sources[idx];

        int32_t degree = degrees[source];
        float new_rank = 0.0f;

        if (degree != 0) new_rank = pagerank[source] / degree;

        atomicAdd(&new_pagerank[destination], new_rank);
    }
}

__global__ void revStructEdgeListPageRank(int32_t size, EdgeStruct *edges, int32_t *degrees, float *pagerank, float *new_pagerank) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        EdgeStruct edge = edges[idx];
        int32_t origin = edge.v;
        int32_t destination = edge.u;

        int32_t degree = degrees[origin];
        float new_rank = 0.0f;
        if (degree != 0) new_rank = pagerank[origin] / degree;
        atomicAdd(&new_pagerank[destination], new_rank);
    }
}

__global__ void structEdgeListPageRank(int32_t size, EdgeStruct *edges, int32_t *degrees, float *pagerank, float *new_pagerank) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        EdgeStruct edge = edges[idx];
        int32_t origin = edge.u;
        int32_t destination = edge.v;

        int32_t degree = degrees[origin];
        float new_rank = 0.0f;
        if (degree != 0) new_rank = pagerank[origin] / degree;
        atomicAdd(&new_pagerank[destination], new_rank);
    }
}

__global__ void vertexPullPageRank(size_t, size_t, int32_t size, int32_t *in_index, int32_t *in_neighs, int32_t *degrees, float *pagerank, float *new_pagerank) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;
    float newRank = 0.0f;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        int32_t start = in_index[idx];
        int32_t end = in_index[idx + 1];

        for (int32_t i = start; i < end; i++) {
            int32_t edge = in_neighs[i];
            newRank += pagerank[edge] / degrees[edge];
        }

        new_pagerank[idx] = newRank;
    }
}

__global__ void vertexPullNoDivPageRank(size_t, size_t, int32_t size, int32_t *in_index, int32_t *in_neighs, int32_t *, float *pagerank, float *new_pagerank) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;
    float newRank = 0.0f;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        int32_t start = in_index[idx];
        int32_t end = in_index[idx + 1];

        for (int32_t i = start; i < end; i++) {
            newRank += pagerank[in_neighs[i]];
        }

        new_pagerank[idx] = newRank;
    }
}

__global__ void vertexPullWarpPageRank(size_t warp_size, size_t chunk_size, int32_t size, int32_t *in_index, int32_t *in_neigs, int32_t *degrees, float *pagerank, float *new_pagerank) {
    const int THREAD_ID = (blockIdx.x * blockDim.x) + threadIdx.x;

    const int32_t warpsPerBlock = blockDim.x / warp_size;

    const int32_t WARP_ID = THREAD_ID / warp_size;
    const int W_OFF = THREAD_ID % warp_size;
    const size_t BLOCK_W_ID = threadIdx.x / warp_size;
    const size_t sharedOffset = chunk_size * BLOCK_W_ID;

    extern __shared__ int32_t MEM1[];
    int32_t *myVertices = &MEM1[sharedOffset + BLOCK_W_ID];

    for ( int32_t chunkIdx = WARP_ID; chunk_size * chunkIdx < size; chunkIdx += warpsPerBlock * gridDim.x){
        const size_t v_ = min(static_cast<int32_t>(chunkIdx * chunk_size), size);
        const size_t end = min(chunk_size, (size - v_));

        memcpy_SIMD<int32_t>(warp_size, W_OFF, end + 1, myVertices, &in_index[v_]);

        for (int v = 0; v < end; v++) {
            float my_new_rank = 0;
            const int32_t num_nbr = myVertices[v+1] - myVertices[v];
            const int32_t *nbrs = &in_neigs[myVertices[v]];
            for (int i = W_OFF; i < num_nbr; i += warp_size) {
                int their_num_nbr = degrees[nbrs[i]];
                my_new_rank += pagerank[nbrs[i]] / their_num_nbr;
            }
            atomicAdd(&new_pagerank[v_ + v], my_new_rank);
        }
    }
}

__global__ void vertexPullWarpNoDivPageRank(size_t warp_size, size_t chunk_size, int32_t size, int32_t *in_index, int32_t *in_neigs, int32_t *, float *pagerank, float *new_pagerank) {
    const int THREAD_ID = (blockIdx.x * blockDim.x) + threadIdx.x;
    const int32_t warpsPerBlock = blockDim.x / warp_size;

    const int32_t WARP_ID = THREAD_ID / warp_size;
    const int W_OFF = THREAD_ID % warp_size;
    const size_t BLOCK_W_ID = threadIdx.x / warp_size;
    const size_t sharedOffset = chunk_size * BLOCK_W_ID;

    extern __shared__ int32_t MEM2[];
    int32_t *myVertices = &MEM2[sharedOffset + BLOCK_W_ID];

    for ( int32_t chunkIdx = WARP_ID; chunk_size * chunkIdx < size; chunkIdx += warpsPerBlock * gridDim.x){
        const size_t v_ = min(static_cast<int32_t>(chunkIdx * chunk_size), size);
        const size_t end = min(chunk_size, (size - v_));

        memcpy_SIMD(warp_size, W_OFF, end + 1, myVertices, &in_index[v_]);

        for (int v = 0; v < end; v++) {
            float my_new_rank = 0;
            const int32_t num_nbr = myVertices[v+1] - myVertices[v];
            const int32_t *nbrs = &in_neigs[myVertices[v]];
            for (int i = W_OFF; i < num_nbr; i += warp_size) {
                my_new_rank += pagerank[nbrs[i]];
            }
            atomicAdd(&new_pagerank[v_ + v], my_new_rank);
        }
    }
}

__global__ void vertexPushPageRank(size_t, size_t, int32_t size, int32_t *out_index, int32_t *out_neighs, int32_t *, float *pagerank, float *new_pagerank) {
    int32_t startIdx = (blockIdx.x * blockDim.x) + threadIdx.x;
    int32_t degree;
    float outgoingRank = 0.0f;

    for (int32_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x) {
        int32_t start = out_index[idx];
        int32_t end = out_index[idx + 1];

        degree = end - start;

        if (degree != 0) outgoingRank = pagerank[idx] / degree;

        for (int32_t i = start; i < end; i++) {
            atomicAdd(&new_pagerank[out_neighs[i]], outgoingRank);
        }
    }

}

__global__ void vertexPushWarpPageRank(size_t warp_size, size_t chunk_size, int32_t size, int32_t *out_index, int32_t *out_neighs, int32_t *, float *pagerank, float *new_pagerank) {
    const int THREAD_ID = (blockIdx.x * blockDim.x) + threadIdx.x;
    const int32_t warpsPerBlock = blockDim.x / warp_size;

    const int32_t WARP_ID = THREAD_ID / warp_size;
    const int W_OFF = THREAD_ID % warp_size;
    const size_t BLOCK_W_ID = threadIdx.x / warp_size;
    const size_t sharedOffset = chunk_size * BLOCK_W_ID;

    extern __shared__ float MEM3[];
    float *myRanks = &MEM3[sharedOffset];
    int32_t *vertices = (int32_t*) &MEM3[warpsPerBlock * chunk_size];
    int32_t *myVertices = &vertices[sharedOffset + BLOCK_W_ID];

    for (int32_t chunkIdx = WARP_ID; chunk_size * chunkIdx < size; chunkIdx += warpsPerBlock * gridDim.x) {
        const size_t v_ = min(static_cast<int32_t>(chunkIdx * chunk_size), size);
        const size_t end = min(chunk_size, (size - v_));

        memcpy_SIMD<float>(warp_size, W_OFF, end, myRanks, &pagerank[v_]);
        memcpy_SIMD<int32_t>(warp_size, W_OFF, end + 1, myVertices, &out_index[v_]);

        for (int v = 0; v < end; v++) {
            const int32_t num_nbr = myVertices[v+1] - myVertices[v];
            const int32_t *nbrs = &out_neighs[myVertices[v]];
            const float my_rank = myRanks[v] / num_nbr;
            for (int i = W_OFF; i < num_nbr; i += warp_size) {
                atomicAdd(&new_pagerank[nbrs[i]], my_rank);
            }
        }
    }
}

// Returns the kernel time in microseconds
double PageRankGPU(GPUPRGraph &g, int max_iters, float *pagerank, GPU_Implementation impl) {
    const int32_t num_nodes = g.num_nodes();
    size_t warp_size = 16;
    size_t chunk_size = 64;
    float *new_pagerank = new float[num_nodes];
    Timer t;

    // Initialize pagerank and new_pagerank arrays
    for (int i = 0; i < num_nodes; ++i) {
        pagerank[i] = 1.0f / num_nodes;
        new_pagerank[i] = 0.0f;
    }

    // Allocate and copy pagerank arrays
    float *d_pagerank = nullptr, *d_new_pagerank = nullptr;
    cudaMalloc(&d_pagerank, num_nodes * sizeof(float));
    cudaMalloc(&d_new_pagerank, num_nodes * sizeof(float));
    cudaMemcpy(d_pagerank, pagerank, num_nodes * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_new_pagerank, new_pagerank, num_nodes * sizeof(float), cudaMemcpyHostToDevice);

    // Kernel launch config
    int threads_per_block = 256;
    int num_blocks = (num_nodes + threads_per_block - 1) / threads_per_block;

    // Calculate indices from CSR graph
    int32_t *index = getNormalizedIndex(g, impl);
    int32_t *neighs = getNeighs(g, impl);
    int32_t *out_degrees = getOutDegrees(g);

    // Allocate device memory for index, neighs, and degrees
    int32_t *d_index = nullptr, *d_neighs = nullptr; int32_t *d_degrees = nullptr;
    cudaMalloc(&d_index, (num_nodes + 1) * sizeof(int32_t));
    cudaMalloc(&d_neighs, g.num_edges() * sizeof(int32_t));
    cudaMalloc(&d_degrees, num_nodes * sizeof(int32_t));
    cudaMemcpy(d_index, index, (num_nodes + 1) * sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_neighs, neighs, g.num_edges() * sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_degrees, out_degrees, num_nodes * sizeof(int32_t), cudaMemcpyHostToDevice);

    // Get the kernel function based on the implementation
    auto kernel = getCSRKernel(impl);
    if (!kernel) {
        std::cerr << "Invalid GPU implementation!" << std::endl;
        return 0;
    }

    int warps_per_block = threads_per_block / warp_size;
    size_t shared_mem_size = warps_per_block * chunk_size * sizeof(float) + warps_per_block * (chunk_size + 1) * sizeof(int32_t);

    t.Start();
    // Pass the pointer to the CSRGraph to the kernel
    for (int iter = 0; iter < max_iters; ++iter) {
        kernel<<<num_blocks, threads_per_block, shared_mem_size>>>(warp_size, chunk_size, num_nodes, d_index, d_neighs, d_degrees, d_pagerank, d_new_pagerank);
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            printf("CUDA Error: %s, with %d nodes and %d edges, and %d shared memory size\n", cudaGetErrorString(err), num_nodes, g.num_edges(), shared_mem_size);
            break;
        }
        cudaDeviceSynchronize();
        if (impl == VERTEX_PULL_NODIV || impl == VERTEX_PULL_WARP_NODIV) {
            consolidateRankNoDiv<<<num_blocks, threads_per_block>>>(num_nodes, d_degrees, d_pagerank, d_new_pagerank, iter != max_iters - 1);
        } else {
            consolidateRank<<<num_blocks, threads_per_block>>>(num_nodes, d_pagerank, d_new_pagerank);
        }
        cudaDeviceSynchronize();
    }
    t.Stop();

    // Copy result back to host
    cudaMemcpy(pagerank, d_pagerank, num_nodes * sizeof(float), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_pagerank);
    cudaFree(d_new_pagerank);
    cudaFree(d_index);
    cudaFree(d_neighs);
    cudaFree(d_degrees);
    delete[] new_pagerank;

    return t.Millisecs();
}

double PageRankGPU(EdgeListStruct &els, int max_iters, float *pagerank, GPU_Implementation impl) {
    const int32_t num_nodes = els.num_nodes;
    const int32_t num_edges = els.num_edges;

    float *new_pagerank = new float[num_nodes];
    Timer t;

    // Initialize pagerank and new_pagerank arrays
    for (int i = 0; i < num_nodes; ++i) {
        pagerank[i] = 1.0f / num_nodes;
        new_pagerank[i] = 0.0f;
    }

    // Allocate and copy pagerank arrays
    float *d_pagerank = nullptr, *d_new_pagerank = nullptr;
    cudaMalloc(&d_pagerank, num_nodes * sizeof(float));
    cudaMalloc(&d_new_pagerank, num_nodes * sizeof(float));
    cudaMemcpy(d_pagerank, pagerank, num_nodes * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_new_pagerank, new_pagerank, num_nodes * sizeof(float), cudaMemcpyHostToDevice);

    // Kernel launch config
    int threads_per_block = 256;
    int num_blocks = (num_nodes + threads_per_block - 1) / threads_per_block;

    // Allocate device memory for edge list
    int32_t *d_sources = nullptr, *d_destinations = nullptr;
    int32_t *d_degrees = nullptr;
    cudaMalloc(&d_sources, num_edges * sizeof(int32_t));
    cudaMalloc(&d_destinations, num_edges * sizeof(int32_t));
    cudaMalloc(&d_degrees, num_nodes * sizeof(int32_t));
    cudaMemcpy(d_sources, els.sources, num_edges * sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_destinations, els.destinations, num_edges * sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_degrees, els.degrees, num_nodes * sizeof(int32_t), cudaMemcpyHostToDevice);

    // Get the kernel function based on the implementation
    auto kernel = getEdgeListStructKernel(impl);
    if (!kernel) {
        std::cerr << "Invalid GPU implementation!" << std::endl;
        return 0;
    }

    t.Start();
    // Pass the pointer to the EdgeListStruct to the kernel
    for (int iter = 0; iter < max_iters; ++iter) {
        kernel<<<num_blocks, threads_per_block>>>(num_edges, d_sources, d_destinations, d_degrees, d_pagerank, d_new_pagerank);
        cudaDeviceSynchronize();
        consolidateRank<<<num_blocks, threads_per_block>>>(num_nodes, d_pagerank, d_new_pagerank);
        cudaDeviceSynchronize();
    }
    t.Stop();

    // Copy result back to host
    cudaMemcpy(pagerank, d_pagerank, num_nodes * sizeof(float), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_pagerank);
    cudaFree(d_new_pagerank);
    cudaFree(d_sources);
    cudaFree(d_destinations);
    cudaFree(d_degrees);
    delete[] new_pagerank;

    return t.Millisecs();
}

double PageRankGPU(EdgeStructList &esl, int max_iters, float *pagerank, GPU_Implementation impl) {
    const int32_t num_nodes = esl.num_nodes;
    const int32_t num_edges = esl.num_edges;

    float *new_pagerank = new float[num_nodes];
    Timer t;

    // Initialize pagerank and new_pagerank arrays
    for (int i = 0; i < num_nodes; ++i) {
        pagerank[i] = 1.0f / num_nodes;
        new_pagerank[i] = 0.0f;
    }

    // Allocate and copy pagerank arrays
    float *d_pagerank = nullptr, *d_new_pagerank = nullptr;
    cudaMalloc(&d_pagerank, num_nodes * sizeof(float));
    cudaMalloc(&d_new_pagerank, num_nodes * sizeof(float));
    cudaMemcpy(d_pagerank, pagerank, num_nodes * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_new_pagerank, new_pagerank, num_nodes * sizeof(float), cudaMemcpyHostToDevice);

    // Kernel launch config
    int threads_per_block = 256;
    int num_blocks = (num_nodes + threads_per_block - 1) / threads_per_block;

    // Allocate device memory for edge list
    EdgeStruct *d_edges = nullptr;
    int32_t *d_degrees = nullptr;
    cudaMalloc(&d_edges, num_edges * sizeof(EdgeStruct));
    cudaMalloc(&d_degrees, num_nodes * sizeof(int32_t));
    cudaMemcpy(d_edges, esl.edges, num_edges * sizeof(EdgeStruct), cudaMemcpyHostToDevice);
    cudaMemcpy(d_degrees, esl.degrees, num_nodes * sizeof(int32_t), cudaMemcpyHostToDevice);

    // Get the kernel function based on the implementation
    auto kernel = getStructEdgeListKernel(impl);
    if (!kernel) {
        std::cerr << "Invalid GPU implementation!" << std::endl;
        return 0;
    }

    t.Start();
    for (int iter = 0; iter < max_iters; ++iter) {
        kernel<<<num_blocks, threads_per_block>>>(num_edges, d_edges, d_degrees, d_pagerank, d_new_pagerank);
        cudaDeviceSynchronize();
        consolidateRank<<<num_blocks, threads_per_block>>>(num_nodes, d_pagerank, d_new_pagerank);
        cudaDeviceSynchronize();
    }
    t.Stop();

    // Copy result back to host
    cudaMemcpy(pagerank, d_pagerank, num_nodes * sizeof(float), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_pagerank);
    cudaFree(d_new_pagerank);
    cudaFree(d_edges);
    cudaFree(d_degrees);
    delete[] new_pagerank;

    return t.Millisecs();
}
