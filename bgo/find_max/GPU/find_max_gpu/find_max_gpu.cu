/*
 * This file is based on https://developer.download.nvidia.com/assets/cuda/files/reduction.pdf
 * and https://github.com/OrangeOwlSolutions/General-CUDA-programming/blob/master/Reductions.cu
 */

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <string>
#include "find_max_gpu.hpp"

struct MaxValue {
    float value;
    int index;
};

__host__ __device__ MaxValue max(MaxValue a, MaxValue b) {
    return a.value > b.value ? a : b;
}

/*
 * CUDA error check
 */
#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, std::string file, int line, bool abort=true)
{
   if (code != cudaSuccess)
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) { getchar(); exit(code); }
   }
}

/*
 * Calculating the next power of 2 from a certain number
*/
unsigned int nextPow2(unsigned int x)
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
}

/*
 * Check if a number is a power of 2.
 */
bool isPow2(unsigned int x)
{
    return ((x&(x-1))==0);
}

template <class T, unsigned int blockSize, bool nIsPow2>
__global__ void reduce(T *g_idata, MaxValue *g_odata, unsigned int N)
{
    extern __shared__ MaxValue sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*(blockDim.x*2) + threadIdx.x;
    unsigned int gridSize = blockSize*2*gridDim.x;

    /* Performs the first level of reduction in registers when reading from global memory on multiple elements per thread.
     * More blocks will result in a larger gridSize and therefore fewer elements per thread */
    MaxValue myMax = {-1, -1};
    while (i < N) {
        myMax = max(myMax, (MaxValue) {g_idata[i], i});
        /* Ensure we don't read out of bounds, this is optimized away for powerOf2 sized arrays */
        if (nIsPow2 || i + blockSize < N) myMax = max(myMax, (MaxValue) {g_idata[i+blockSize], i+blockSize});
        i += gridSize;
    }

    /* Each thread puts its local sum into shared memory */
    sdata[tid] = myMax; __syncthreads();

    /* Reduction in shared memory. Fully unrolled loop */
    if ((blockSize >= 512) && (tid < 256)) sdata[tid] = myMax = max(myMax, sdata[tid + 256]); __syncthreads();
    if ((blockSize >= 256) && (tid < 128)) sdata[tid] = myMax = max(myMax, sdata[tid + 128]); __syncthreads();
    if ((blockSize >= 128) && (tid <  64)) sdata[tid] = myMax = max(myMax, sdata[tid +  64]); __syncthreads();
    if ((blockSize >=  64) && (tid <  32)) sdata[tid] = myMax = max(myMax, sdata[tid +  32]); __syncthreads();
    if ((blockSize >=  32) && (tid <  16)) sdata[tid] = myMax = max(myMax, sdata[tid +  16]); __syncthreads();
    if ((blockSize >=  16) && (tid <   8)) sdata[tid] = myMax = max(myMax, sdata[tid +   8]); __syncthreads();
    if ((blockSize >=   8) && (tid <   4)) sdata[tid] = myMax = max(myMax, sdata[tid +   4]); __syncthreads();
    if ((blockSize >=   4) && (tid <   2)) sdata[tid] = myMax = max(myMax, sdata[tid +   2]); __syncthreads();
    if ((blockSize >=   2) && (tid <   1)) sdata[tid] = myMax = max(myMax, sdata[tid +   1]); __syncthreads();

    /* Write result for this block to global memory. At the end of the kernel,
     * global memory will contain the results for the summations of individual blocks */
    if (tid == 0) g_odata[blockIdx.x] = myMax;
}


template <class T>
void reduce_wrapper(T *g_idata, MaxValue *g_odata, unsigned int N, int NumBlocks, int NumThreads, int smemSize) {
    if (isPow2(N)) {
        switch (NumThreads) {
            case 512: reduce<T, 512, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 256: reduce<T, 256, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 128: reduce<T, 128, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 64:  reduce<T,  64, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 32:  reduce<T,  32, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 16:  reduce<T,  16, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case  8:  reduce<T,   8, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case  4:  reduce<T,   4, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case  2:  reduce<T,   2, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case  1:  reduce<T,   1, true><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
        }
        gpuErrchk(cudaPeekAtLastError());
        gpuErrchk(cudaDeviceSynchronize());
    }
    else {
        switch (NumThreads) {
            case 512: reduce<T, 512, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 256: reduce<T, 256, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 128: reduce<T, 128, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 64:  reduce<T,  64, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 32:  reduce<T,  32, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case 16:  reduce<T,  16, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case  8:  reduce<T,   8, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case  4:  reduce<T,   4, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case  2:  reduce<T,   2, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
            case  1:  reduce<T,   1, false><<< NumBlocks, NumThreads, smemSize >>>(g_idata, g_odata, N); break;
        }
        gpuErrchk(cudaPeekAtLastError());
        gpuErrchk(cudaDeviceSynchronize());
    }
}

int find_max_gpu(CArray<int> v, int *index) {
    int n = v.size;
    thrust::device_vector<int> d_vec(v.data, v.data+n);

    int NumThreads = (n < BLOCKSIZE) ? nextPow2(n) : BLOCKSIZE;
    int NumBlocks = (n + NumThreads - 1) / NumThreads;

    /* when there is only one warp per block, we need to allocate two warps
     * worth of shared memory so that we don't index shared memory out of bounds */
    int smemSize = (NumThreads <= 32) ? 2 * NumThreads * sizeof(MaxValue) : NumThreads * sizeof(MaxValue);

    thrust::device_vector<MaxValue> d_vec_block(NumBlocks);

    reduce_wrapper(thrust::raw_pointer_cast(d_vec.data()), thrust::raw_pointer_cast(d_vec_block.data()), n, NumBlocks, NumThreads, smemSize);

    /* The last part of the reduction, which would be expensive to perform on the device, is executed on the host */
    thrust::host_vector<MaxValue> h_vec_block(d_vec_block);
    MaxValue max_val = {-1, -1};
    for (int i=0; i<NumBlocks; i++)
        max_val = max(max_val, h_vec_block[i]);

    *index = max_val.index;
    return 0;
}


// int main()
// {
//     srand(42);
//     const int n = 5000000;
//     float *v = new float[n];

//     for (int i = 0; i < n; i++) {
//         v[i] = static_cast <float> (rand());
//     }

//     thrust::device_vector<float> d_vec(v, v+n);

//     int NumThreads = (n < BLOCKSIZE) ? nextPow2(n) : BLOCKSIZE;
//     int NumBlocks = (n + NumThreads - 1) / NumThreads;

//     /* when there is only one warp per block, we need to allocate two warps
//      * worth of shared memory so that we don't index shared memory out of bounds */
//     int smemSize = (NumThreads <= 32) ? 2 * NumThreads * sizeof(MaxValue) : NumThreads * sizeof(MaxValue);

//     /* Creating events for timing */
//     float time;
//     cudaEvent_t start, stop;
//     cudaEventCreate(&start);
//     cudaEventCreate(&stop);

//     thrust::device_vector<MaxValue> d_vec_block(NumBlocks);

//     cudaEventRecord(start, 0);
//     reduce_wrapper(thrust::raw_pointer_cast(d_vec.data()), thrust::raw_pointer_cast(d_vec_block.data()), n, NumBlocks, NumThreads, smemSize);
//     cudaEventRecord(stop, 0);
//     cudaEventSynchronize(stop);
//     cudaEventElapsedTime(&time, start, stop);
//     printf("Elapsed time:  %3.3f ms \n", time);

//     /* The last part of the reduction, which would be expensive to perform on the device, is executed on the host */
//     thrust::host_vector<MaxValue> h_vec_block(d_vec_block);
//     MaxValue max_val = {-1, -1};
//     for (int i=0; i<NumBlocks; i++)
//         max_val = max(max_val, h_vec_block[i]);
//     printf("Result: %i\n", max_val.index);
//     printf("Result value: %f\n", max_val.value);
//     delete[] v;
// }