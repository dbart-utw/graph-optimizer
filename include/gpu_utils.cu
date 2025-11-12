#include "gpu_utils.hpp"

int32_t *getNormalizedIndex(CSR &g, GPU_Implementation impl) {
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

int32_t *getNeighs(CSR &g, GPU_Implementation impl) {
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

int32_t *getOutDegrees(CSR &g) {
    int32_t *out_degrees = new int32_t[g.num_nodes()];
    for (int i = 0; i < g.num_nodes(); i++) {
        out_degrees[i] = (int32_t)g.out_degree(i);
    }

    return out_degrees;
}

template <typename T>
void read_graph_GPU_CMatrix(GPU_CMatrix<T> *G, char *filename) {
    CMatrix<T> G_temp;
    read_graph_CMatrix(&G_temp, filename);

    G->init(G_temp.size_m, G_temp.size_n);
    cudaMemcpy(G->data, G_temp.data, G_temp.size_m * G_temp.size_n * sizeof(T), cudaMemcpyHostToDevice);
}

template <typename T>
void read_vector_GPU_CArray(GPU_CArray<T> *V, char* filename) {
    GPU_CArray<T> V_temp;
    read_vector_CArray<T>(&V_temp, filename);

    V->init(V_temp.size);
    cudaMemcpy(V->data, V_temp.data, V_temp.size * sizeof(T));
}

void cudaAssert(const cudaError_t code, const char *file, const int line) {
    if (code != cudaSuccess) {
        std::cout.flush();
        std::cerr << "CUDA error #"
                << code
                << " ("
                << file
                << ":"
                << line
                << "):"
                << std::endl
                << cudaGetErrorString(code)
                << std::endl;
    }
}