#include "bfs.hpp"

void resetFrontier()
{
    const unsigned val = 0;
    CUDA_CHK(cudaMemcpyToSymbol(frontier, &val, sizeof val));
}


unsigned getFrontier()
{
    unsigned val;
    CUDA_CHK(cudaMemcpyFromSymbol(&val, frontier, sizeof val));
    return val;
}


template<typename BFSVariant>
__global__ void
edgeListBfs(int32_t size, int32_t *sources, int32_t *destinations, int32_t *levels, int32_t depth)
{
    uint64_t startIdx = blockIdx.x * blockDim.x + threadIdx.x;
    BFSVariant bfs;
    int newDepth = depth + 1;

    for (uint64_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x)
    {
        if (levels[sources[idx]] == depth) {
            if (atomicMin(&levels[destinations[idx]], newDepth) > newDepth) {
                bfs.update();
            }
        }
    }
    bfs.finalise();
}
template __global__ void edgeListBfs<Reduction<normal>>(int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void edgeListBfs<Reduction<bulk>>(int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void edgeListBfs<Reduction<warpreduce>>(int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void edgeListBfs<Reduction<blockreduce>>(int32_t, int32_t*, int32_t*, int32_t*, int32_t);


template<typename BFSVariant>
__global__ void
revEdgeListBfs(int32_t size, int32_t *sources, int32_t *destinations, int32_t *levels, int32_t depth)
{
    uint64_t startIdx = blockIdx.x * blockDim.x + threadIdx.x;
    BFSVariant bfs;
    int newDepth = depth + 1;

    for (uint64_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x)
    {
        if (levels[destinations[idx]] == depth) {
            if (atomicMin(&levels[sources[idx]], newDepth) > newDepth) {
                bfs.update();
            }
        }
    }
    bfs.finalise();
}
template __global__ void revEdgeListBfs<Reduction<normal>>(int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void revEdgeListBfs<Reduction<bulk>>(int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void revEdgeListBfs<Reduction<warpreduce>>(int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void revEdgeListBfs<Reduction<blockreduce>>(int32_t, int32_t*, int32_t*, int32_t*, int32_t);


template<typename BFSVariant>
__global__ void
revStructEdgeListBfs(int32_t size, EdgeStruct *edges, int32_t *levels, int32_t depth)
{
    uint64_t startIdx = blockIdx.x * blockDim.x + threadIdx.x;
    BFSVariant bfs;
    int newDepth = depth + 1;

    for (uint64_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x)
    {
        EdgeStruct edge = edges[idx];
        if (levels[edge.v] == depth) {
            if (atomicMin(&levels[edge.u], newDepth) > newDepth) {
                bfs.update();
            }
        }
    }
    bfs.finalise();
}
template __global__ void revStructEdgeListBfs<Reduction<normal>>(int32_t, EdgeStruct*, int32_t*, int32_t);
template __global__ void revStructEdgeListBfs<Reduction<bulk>>(int32_t, EdgeStruct*, int32_t*, int32_t);
template __global__ void revStructEdgeListBfs<Reduction<warpreduce>>(int32_t, EdgeStruct*, int32_t*, int32_t);
template __global__ void revStructEdgeListBfs<Reduction<blockreduce>>(int32_t, EdgeStruct*, int32_t*, int32_t);


template<typename BFSVariant>
__global__ void
structEdgeListBfs(int32_t size, EdgeStruct *edges, int32_t *levels, int32_t depth)
{
    uint64_t startIdx = blockIdx.x * blockDim.x + threadIdx.x;
    BFSVariant bfs;
    int newDepth = depth + 1;

    for (uint64_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x)
    {
        EdgeStruct edge = edges[idx];
        if (levels[edge.u] == depth) {
            if (atomicMin(&levels[edge.v], newDepth) > newDepth) {
                bfs.update();
            }
        }
    }
    bfs.finalise();
}
template __global__ void structEdgeListBfs<Reduction<normal>>(int32_t, EdgeStruct*, int32_t*, int32_t);
template __global__ void structEdgeListBfs<Reduction<bulk>>(int32_t, EdgeStruct*, int32_t*, int32_t);
template __global__ void structEdgeListBfs<Reduction<warpreduce>>(int32_t, EdgeStruct*, int32_t*, int32_t);
template __global__ void structEdgeListBfs<Reduction<blockreduce>>(int32_t, EdgeStruct*, int32_t*, int32_t);


template<typename BFSVariant>
__global__ void vertexPullBfs(size_t, size_t, int32_t size, int32_t *in_index, int32_t *in_neighs, int32_t *levels, int32_t depth) {
    uint64_t startIdx = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned offset = blockDim.x * gridDim.x;
    unsigned newDepth = depth + 1;
    BFSVariant bfs;

    for (uint64_t idx = startIdx; idx < size; idx += offset)
    {
        if (levels[idx] > newDepth) {
            int32_t start = in_index[idx];
            int32_t end = in_index[idx + 1];

            for (unsigned i = start; i < end; i++) {
                if (levels[in_neighs[i]] == depth) {
                    levels[idx] = newDepth;
                    bfs.update();
                    break;
                }
            }
        }
    }
    bfs.finalise();
}
template __global__ void vertexPullBfs<Reduction<normal>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPullBfs<Reduction<bulk>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPullBfs<Reduction<warpreduce>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPullBfs<Reduction<blockreduce>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);

static __device__ inline int
expand_bfs_vertex_pull_warp(unsigned mask, int W_SZ, unsigned W_OFF, unsigned cnt, const int32_t *edges, int32_t *levels, int32_t curr)
{
    int result = 0;
    for (unsigned IDX = W_OFF; IDX < cnt; IDX += W_SZ) {
        if (levels[edges[IDX]] == curr) {
            result = 1;
        }
    }
    return __any_sync(mask, result);
}


template<typename BFSVariant>
__global__ void
vertexPullWarpBfs( size_t warp_size, size_t chunk_size, int32_t size, int32_t *in_index, int32_t *in_neighs, int *levels, int depth)
{
    BFSVariant bfs;
    const int THREAD_ID = (blockIdx.x * blockDim.x) + threadIdx.x;
    const uint64_t warpsPerBlock = blockDim.x / warp_size;

    const unsigned SUB_WARP_OFFSET = THREAD_ID & (32 - warp_size);
    const unsigned mask = ((1 << warp_size) - 1) << SUB_WARP_OFFSET;

    const uint64_t WARP_ID = THREAD_ID / warp_size;
    const int W_OFF = THREAD_ID % warp_size;
    const size_t BLOCK_W_ID = threadIdx.x / warp_size;
    const size_t sharedOffset = chunk_size * BLOCK_W_ID;

    extern __shared__ int MEM[];
    int *myLevels = &MEM[sharedOffset];
    int32_t *vertices = &MEM[warpsPerBlock * chunk_size];
    int32_t *myVertices = &vertices[sharedOffset + BLOCK_W_ID];
    int newDepth = depth + 1;

    for ( uint64_t chunkIdx = WARP_ID; chunk_size * chunkIdx < size; chunkIdx += warpsPerBlock * gridDim.x) {
        const size_t v_ = min(static_cast<int32_t>(chunkIdx * chunk_size), size);
        const size_t end = min(chunk_size, (size - v_));

        memcpy_SIMD(warp_size, W_OFF, end, myLevels, &levels[v_]);
        memcpy_SIMD(warp_size, W_OFF, end + 1, myVertices, &in_index[v_]);

        for (int v = 0; v < end; v++) {
            const int32_t num_nbr = myVertices[v+1] - myVertices[v];
            const int32_t *nbrs = &in_neighs[myVertices[v]];
            if (myLevels[v] > depth) {
                if (expand_bfs_vertex_pull_warp(mask, warp_size, W_OFF, num_nbr, nbrs, levels, depth)) {
                    myLevels[v] = newDepth;
                }
            }
        }

        __syncwarp(mask);

        for (unsigned IDX = W_OFF; IDX < end; IDX += warp_size) {
            if (myLevels[IDX] == newDepth) {
                bfs.update();
            }
        }
        memcpy_SIMD(warp_size, W_OFF, end, &levels[v_], myLevels);
    }
    bfs.finalise();
}
template __global__ void vertexPullWarpBfs<Reduction<normal>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPullWarpBfs<Reduction<bulk>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPullWarpBfs<Reduction<warpreduce>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPullWarpBfs<Reduction<blockreduce>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);

template<typename BFSVariant>
__global__ void vertexPushBfs(size_t, size_t, int32_t size, int32_t *out_index, int32_t *out_neighs, int32_t *levels, int32_t depth) {
    uint64_t startIdx = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned newDepth = depth + 1;
    BFSVariant bfs;

    for (uint64_t idx = startIdx; idx < size; idx += blockDim.x * gridDim.x)
    {
        if (levels[idx] == depth) {
            int32_t start = out_index[idx];
            int32_t end = out_index[idx + 1];

            for (unsigned i = start; i < end; i++) {
                if (atomicMin(&levels[out_neighs[i]], newDepth) > newDepth) {
                    bfs.update();
                }
            }
        }
    }
    bfs.finalise();
}
template __global__ void vertexPushBfs<Reduction<normal>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPushBfs<Reduction<bulk>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPushBfs<Reduction<warpreduce>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPushBfs<Reduction<blockreduce>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);

template<typename BFSVariant>
static __device__ inline void expand_bfs_vertex_push_warp(BFSVariant &bfs, int W_SZ, unsigned W_OFF, unsigned cnt, const int32_t *edges, int *levels, int curr) {
    int newDepth = curr + 1;
    for (unsigned IDX = W_OFF; IDX < cnt; IDX += W_SZ) {
        int32_t v = edges[IDX];
        if (atomicMin(&levels[v], newDepth) > newDepth) {
            bfs.update();
        }
    }
    __threadfence_block();
}


template<typename BFSVariant>
__global__ void vertexPushWarpBfs(size_t warp_size, size_t chunk_size, int32_t size, int32_t *out_index, int32_t *out_neighs, int *levels, int depth) {
    BFSVariant bfs;
    const int THREAD_ID = (blockIdx.x * blockDim.x) + threadIdx.x;
    const uint64_t warpsPerBlock = blockDim.x / warp_size;

    const uint64_t WARP_ID = THREAD_ID / warp_size;
    const int W_OFF = THREAD_ID % warp_size;
    const size_t BLOCK_W_ID = threadIdx.x / warp_size;
    const size_t sharedOffset = chunk_size * BLOCK_W_ID;

    extern __shared__ int MEM2[];
    int *myLevels = &MEM2[sharedOffset];
    int32_t *vertices = &MEM2[warpsPerBlock * chunk_size];
    int32_t *myVertices = &vertices[sharedOffset + BLOCK_W_ID];

    for (uint64_t chunkIdx = WARP_ID; chunk_size * chunkIdx < size; chunkIdx += warpsPerBlock * gridDim.x) {
        const size_t v_ = min(static_cast<int32_t>(chunkIdx * chunk_size), size);
        const size_t end = min(chunk_size, (size - v_));

        memcpy_SIMD(warp_size, W_OFF, end, myLevels, &levels[v_]);
        memcpy_SIMD(warp_size, W_OFF, end + 1, myVertices, &out_index[v_]);

        for (int v = 0; v < end; v++) {
            const int32_t num_nbr = myVertices[v+1] - myVertices[v];
            const int32_t *nbrs = &out_neighs[myVertices[v]];
            if (myLevels[v] == depth) {
                expand_bfs_vertex_push_warp(bfs, warp_size, W_OFF, num_nbr, nbrs, levels, depth);
            }
        }
    }
    bfs.finalise();
}
template __global__ void vertexPushWarpBfs<Reduction<normal>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPushWarpBfs<Reduction<bulk>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPushWarpBfs<Reduction<warpreduce>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);
template __global__ void vertexPushWarpBfs<Reduction<blockreduce>>(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t);

template<typename BFSVariant>
inline void (*getCSRKernel(GPU_Implementation impl))(size_t, size_t, int32_t, int32_t*, int32_t*, int32_t*, int32_t) {
    switch (impl) {
        case VERTEX_PULL:
        return vertexPullBfs<BFSVariant>;
        case VERTEX_PULL_WARP:
        return vertexPullWarpBfs<BFSVariant>;
        case VERTEX_PUSH:
            return vertexPushBfs<BFSVariant>;
        case VERTEX_PUSH_WARP:
            return vertexPushWarpBfs<BFSVariant>;
        default:
            return nullptr; // Handle invalid cases
    }
}

template<typename BFSVariant>
inline void (*getEdgeListStructKernel(GPU_Implementation impl))(int32_t, int32_t*, int32_t*, int32_t*, int32_t) {
    switch (impl) {
        case EDGELIST:
            return edgeListBfs<BFSVariant>;
        case REV_EDGELIST:
            return revEdgeListBfs<BFSVariant>;
        default:
            return nullptr; // Handle invalid cases
    }
}

template<typename BFSVariant>
inline void (*getStructEdgeListKernel(GPU_Implementation impl))(int32_t, EdgeStruct*, int32_t*, int32_t) {
    switch (impl) {
        case STRUCT_EDGELIST:
            return structEdgeListBfs<BFSVariant>;
        case REV_STRUCT_EDGELIST:
            return revStructEdgeListBfs<BFSVariant>;
        default:
            return nullptr; // Handle invalid cases
    }
}

template<typename BFSVariant>
double BFSGPU(CSR &g, int32_t *levels, GPU_Implementation impl) {
    const int32_t num_nodes = g.num_nodes();
    const int32_t num_edges = g.num_edges();
    size_t warp_size = 16;
    size_t chunk_size = 64;
    Timer t;

    // Initialize pagerank and new_pagerank arrays
    levels[0] = 0;
    for (int i = 1; i < num_nodes; ++i) {
        levels[i] = std::numeric_limits<int>::max();
    }

    // Allocate and copy pagerank arrays
    int32_t *d_levels = nullptr;
    cudaMalloc(&d_levels, num_nodes * sizeof(int32_t));
    cudaMemcpy(d_levels, levels, num_nodes * sizeof(int32_t), cudaMemcpyHostToDevice);

    // Kernel launch config
    int threads_per_block = 256;
    int num_blocks = (num_nodes + threads_per_block - 1) / threads_per_block;

    // Calculate indices from CSR graph
    int32_t *index = getNormalizedIndex(g, impl);
    int32_t *neighs = getNeighs(g, impl);

    // Allocate device memory for index, neighs, and degrees
    int32_t *d_index = nullptr, *d_neighs = nullptr;
    cudaMalloc(&d_index, (num_nodes + 1) * sizeof(int32_t));
    cudaMalloc(&d_neighs, g.num_edges() * sizeof(int32_t));
    cudaMemcpy(d_index, index, (num_nodes + 1) * sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_neighs, neighs, g.num_edges() * sizeof(int32_t), cudaMemcpyHostToDevice);

    // Get the kernel function based on the implementation
    auto kernel = getCSRKernel<BFSVariant>(impl);
    if (!kernel) {
        std::cerr << "Invalid GPU implementation!" << std::endl;
        return 0;
    }

    int warps_per_block = threads_per_block / warp_size;
    size_t shared_mem_size = warps_per_block * chunk_size * sizeof(int32_t) + warps_per_block * (chunk_size + 1) * sizeof(int32_t);

    int depth = 0;
    unsigned frontier_size = 1;
	double total_millisecond = 0;
	double iteration_time = 0;
    do {
    	t.Start();
        resetFrontier();
		printf("Frontier size on depth %d: %d. ", depth, frontier_size);
        kernel<<<num_blocks, threads_per_block, shared_mem_size>>>(warp_size, chunk_size, num_nodes, d_index, d_neighs, d_levels, depth++);
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            printf("CUDA Error: %s, with %d nodes and %d edges, and %d shared memory size\n", cudaGetErrorString(err), num_nodes, num_edges, shared_mem_size);
        }
        frontier_size = getFrontier();
    	t.Stop();

		iteration_time = t.Millisecs();
		printf("Took %f milliseconds\n", iteration_time);
		total_millisecond += iteration_time;
    } while (frontier_size > 0);

    // Copy result back to host
    cudaMemcpy(levels, d_levels, num_nodes * sizeof(int32_t), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_levels);
    cudaFree(d_index);
    cudaFree(d_neighs);

    return t.Millisecs();
}
template double BFSGPU<Reduction<normal>>(CSR &g, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<bulk>>(CSR &g, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<warpreduce>>(CSR &g, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<blockreduce>>(CSR &g, int32_t *levels, GPU_Implementation impl);


template<typename BFSVariant>
double BFSGPU(EdgeListStruct &els, int32_t *levels, GPU_Implementation impl) {
    const int32_t num_nodes = els.num_nodes;
    const int32_t num_edges = els.num_edges;

    size_t warp_size = 16;
    size_t chunk_size = 64;
    Timer t;

    // Initialize pagerank and new_pagerank arrays
    levels[0] = 0;
    for (int i = 1; i < num_nodes; ++i) {
        levels[i] = std::numeric_limits<int>::max();
    }

    // Allocate and copy pagerank arrays
    int32_t *d_levels = nullptr;
    cudaMalloc(&d_levels, num_nodes * sizeof(int32_t));
    cudaMemcpy(d_levels, levels, num_nodes * sizeof(int32_t), cudaMemcpyHostToDevice);

    // Kernel launch config
    int threads_per_block = 256;
    int num_blocks = (num_nodes + threads_per_block - 1) / threads_per_block;

    // Allocate device memory for index, neighs, and degrees
    int32_t *d_sources = nullptr, *d_destinations = nullptr;
    cudaMalloc(&d_sources, num_edges * sizeof(int32_t));
    cudaMalloc(&d_destinations, num_edges * sizeof(int32_t));
    cudaMemcpy(d_sources, els.sources, num_edges * sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_destinations, els.destinations, num_edges * sizeof(int32_t), cudaMemcpyHostToDevice);

    // Get the kernel function based on the implementation
    auto kernel = getEdgeListStructKernel<BFSVariant>(impl);
    if (!kernel) {
        std::cerr << "Invalid GPU implementation!" << std::endl;
        return 0;
    }

    int warps_per_block = threads_per_block / warp_size;
    size_t shared_mem_size = warps_per_block * chunk_size * sizeof(int32_t) + warps_per_block * (chunk_size + 1) * sizeof(int32_t);

    int depth = 0;
    unsigned frontier_size = 1;
    t.Start();
    do {
        resetFrontier();
        kernel<<<num_blocks, threads_per_block, shared_mem_size>>>(num_edges, d_sources, d_destinations, d_levels, depth++);
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            printf("CUDA Error: %s, with %d nodes and %d edges, and %d shared memory size\n", cudaGetErrorString(err), num_nodes, num_edges, shared_mem_size);
        }
        frontier_size = getFrontier();
    } while (frontier_size > 0);
    t.Stop();

    // Copy result back to host
    cudaMemcpy(levels, d_levels, num_nodes * sizeof(int32_t), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_levels);
    cudaFree(d_sources);
    cudaFree(d_destinations);

    return t.Millisecs();
}
template double BFSGPU<Reduction<normal>>(EdgeListStruct &els, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<bulk>>(EdgeListStruct &els, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<warpreduce>>(EdgeListStruct &els, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<blockreduce>>(EdgeListStruct &els, int32_t *levels, GPU_Implementation impl);

template<typename BFSVariant>
double BFSGPU(EdgeStructList &esl, int32_t *levels, GPU_Implementation impl) {
    const int32_t num_nodes = esl.num_nodes;
    const int32_t num_edges = esl.num_edges;
    
    size_t warp_size = 16;
    size_t chunk_size = 64;
    Timer t;

    // Initialize pagerank and new_pagerank arrays
    levels[0] = 0;
    for (int i = 1; i < num_nodes; ++i) {
        levels[i] = std::numeric_limits<int>::max();
    }

    // Allocate and copy pagerank arrays
    int32_t *d_levels = nullptr;
    cudaMalloc(&d_levels, num_nodes * sizeof(int32_t));
    cudaMemcpy(d_levels, levels, num_nodes * sizeof(int32_t), cudaMemcpyHostToDevice);

    // Kernel launch config
    int threads_per_block = 256;
    int num_blocks = (num_nodes + threads_per_block - 1) / threads_per_block;

    // Allocate device memory for index, neighs, and degrees
    EdgeStruct *d_edges = nullptr;
    cudaMalloc(&d_edges, num_edges * sizeof(EdgeStruct));
    cudaMemcpy(d_edges, esl.edges, num_edges * sizeof(EdgeStruct), cudaMemcpyHostToDevice);

    // Get the kernel function based on the implementation
    auto kernel = getStructEdgeListKernel<BFSVariant>(impl);
    if (!kernel) {
        std::cerr << "Invalid GPU implementation!" << std::endl;
        return 0;
    }

    int warps_per_block = threads_per_block / warp_size;
    size_t shared_mem_size = warps_per_block * chunk_size * sizeof(int32_t) + warps_per_block * (chunk_size + 1) * sizeof(int32_t);

    int depth = 0;
    unsigned frontier_size = 1;
    t.Start();
    do {
        resetFrontier();
        kernel<<<num_blocks, threads_per_block, shared_mem_size>>>(num_edges, d_edges, d_levels, depth++);
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            printf("CUDA Error: %s, with %d nodes and %d edges, and %d shared memory size\n", cudaGetErrorString(err), num_nodes, num_edges, shared_mem_size);
        }
        frontier_size = getFrontier();
    } while (frontier_size > 0);
    t.Stop();

    // Copy result back to host
    cudaMemcpy(levels, d_levels, num_nodes * sizeof(int32_t), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_levels);
    cudaFree(d_edges);

    return t.Millisecs();
}
template double BFSGPU<Reduction<normal>>(EdgeStructList &esl, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<bulk>>(EdgeStructList &esl, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<warpreduce>>(EdgeStructList &esl, int32_t *levels, GPU_Implementation impl);
template double BFSGPU<Reduction<blockreduce>>(EdgeStructList &esl, int32_t *levels, GPU_Implementation impl);
