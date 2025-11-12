#ifndef GPU_UTILS_HPP
#define GPU_UTILS_HPP

#include <cuda_runtime.h>

#include "utils.hpp"
#include "gpu_datastructures.hpp"
#include "gap/graph.h"

enum GPU_Implementation {
    EDGELIST,
    REV_EDGELIST,
    STRUCT_EDGELIST,
    REV_STRUCT_EDGELIST,
    VERTEX_PULL,
    VERTEX_PULL_NODIV,
    VERTEX_PULL_WARP,
    VERTEX_PULL_WARP_NODIV,
    VERTEX_PUSH,
    VERTEX_PUSH_WARP
};

int32_t *getNormalizedIndex(CSR &g, GPU_Implementation impl);

int32_t *getNeighs(CSR &g, GPU_Implementation impl);

int32_t *getOutDegrees(CSR &g);

template <typename T>
void read_graph_GPU_CMatrix(GPU_CMatrix<T> *G, char *filename);

template <typename T>
void read_vector_GPU_CArray(GPU_CArray<T> *V, char* filename);

void cudaAssert(const cudaError_t code, const char *file, const int line);
#define CUDA_CHK(ans) cudaAssert((ans), __FILE__, __LINE__)

#ifdef __CUDACC__
template<typename T>
static __device__ inline void
memcpy_SIMD(size_t warp_size, size_t warp_offset, int cnt, T *dest, T *src)
{
    for (int i = warp_offset; i < cnt; i += warp_size) {
        dest[i] = src[i];
    }
    __threadfence_block();
}
#endif
#endif
