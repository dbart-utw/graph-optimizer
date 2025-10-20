#ifndef GPU_UTILS_HPP
#define GPU_UTILS_HPP

#include <cuda_runtime.h>

#include "utils.hpp"
#include "gpu_datastructures.hpp"

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

#endif
