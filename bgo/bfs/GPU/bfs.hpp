#ifndef BFS_HPP
#define BFS_HPP

#include <algorithm>
#include <cuda_runtime.h>
#include "gap/timer.h"
#include "gap/graph.h"
#include "gpu_utils.hpp"

void resetFrontier();
unsigned getFrontier();

enum bfs_variant {
    normal,
    bulk,
    warpreduce,
    blockreduce
};

#ifdef __CUDACC__
extern __device__ unsigned frontier = 0;

__device__ __forceinline__
unsigned warpReduceSum(unsigned val)
{
    for (int offset = warpSize/2; offset > 0; offset /= 2) {
        val += __shfl_down_sync(0xffffffff, val, offset);
     }

    return val;
}
#endif

template<bfs_variant variant>
struct Reduction {
    static constexpr const char * suffix = "";
    __device__ __forceinline__ void update()
    {
#ifdef __CUDACC__
        atomicAdd(&frontier, 1U);
#endif
    }

    __device__ __forceinline__ void finalise() {}
};

template<>
struct Reduction<bulk> {
    static constexpr const char * suffix = "-bulk";
    unsigned count;

    __device__ Reduction() : count(0U) {}

    __device__ __forceinline__ void update()
    { count++; }

    __device__ __forceinline__ void finalise()
    {
#ifdef __CUDACC__
        atomicAdd(&frontier, count);
#endif
    }
};

template<>
struct Reduction<warpreduce> {
    static constexpr const char * suffix = "-warpreduce";
    unsigned count;

    __device__ Reduction() : count(0U) {}

    __device__ __forceinline__ void update()
    { count++; }

    __device__ __forceinline__ void finalise()
    {
#ifdef __CUDACC__
        int lane = threadIdx.x % warpSize;

        count = warpReduceSum(count);
        if (lane == 0) atomicAdd(&frontier, count);
#endif
    }
};

template<>
struct Reduction<blockreduce> {
    static constexpr const char * suffix = "-blockreduce";
    unsigned count;

    __device__ Reduction() : count(0U) {}

    __device__ __forceinline__ void update()
    { count++; }

    __device__ __forceinline__ void finalise()
    {
#ifdef __CUDACC__
        static __shared__ unsigned shared[32]; // Shared mem for 32 partial sums
        int lane = threadIdx.x % warpSize;
        int wid = threadIdx.x / warpSize;

        count = warpReduceSum(count);     // Each warp performs partial reduction

        if (lane==0) shared[wid]=count; // Write reduced value to shared memory

        __syncthreads();              // Wait for all partial reductions

        //read from shared memory only if that warp existed
        count = (threadIdx.x < blockDim.x / warpSize) ? shared[lane] : 0;

        if (wid==0) {
            count = warpReduceSum(count); //Final reduce within first warp
            if (lane==0) atomicAdd(&frontier, count);
        }
#endif
    }
};

template<typename BFSVariant>
__global__ void vertexPushBfs(size_t, size_t, int32_t size, int32_t *out_index, int32_t *out_neighs, int32_t *levels, int32_t depth);

template<typename BFSVariant>
__global__ void vertexPullBfs(size_t, size_t, int32_t size, int32_t *in_index, int32_t *in_neighs, int32_t *levels, int32_t depth);

template<typename BFSVariant>
__global__ void vertexPushWarpBfs(size_t warp_size, size_t chunk_size, int32_t size, int32_t *out_index, int32_t *out_neighs, int32_t *levels, int32_t depth);

template<typename BFSVariant>
__global__ void vertexPullWarpBfs(size_t warp_size, size_t chunk_size, int32_t size, int32_t *in_index, int32_t *in_neighs, int32_t *levels, int32_t depth);

template<typename BFSVariant>
__global__ void edgeListBfs(int32_t size, int32_t *sources, int32_t *destinations, int32_t *levels, int32_t depth);

template<typename BFSVariant>
__global__ void structEdgeListBfs(int32_t size, EdgeStruct *edges, int32_t *levels, int32_t depth);

template<typename BFSVariant>
__global__ void revStructEdgeListBfs(int32_t size, EdgeStruct *edges, int32_t *levels, int32_t depth);

template<typename BFSVariant>
__global__ void revEdgeListBfs(int32_t size, int32_t *sources, int32_t *destinations, int32_t *levels, int32_t depth);

template<typename BFSVariant>
double BFSGPU(CSR &g, int32_t *levels, GPU_Implementation impl);
template<typename BFSVariant>
double BFSGPU(EdgeListStruct &els, int32_t *levels, GPU_Implementation impl);
template<typename BFSVariant>
double BFSGPU(EdgeStructList &esl, int32_t *levels, GPU_Implementation impl);

#endif
