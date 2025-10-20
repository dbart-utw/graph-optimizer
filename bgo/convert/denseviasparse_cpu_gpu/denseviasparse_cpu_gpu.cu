#include "datastructures.hpp"
#include "gpu_datastructures.hpp"

#include "denseviasparse_cpu_gpu.hpp"

#define cudaDeviceScheduleBlockingSync 0x04 
#define BLOCK_SIZE 32


__global__
void sparse2denseGPU(
    int *dst, size_t w,
    int *src_data, size_t *src_rows, size_t *src_cols, size_t n
) {
    size_t i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < n) {
        dst[src_rows[i] * w + src_cols[i]] = src_data[i];
    }
}

extern "C" int denseviasparseCPU2GPU(CMatrix<int> M, GPU_CMatrix<int> *N) {
    N->init(M.size_m, M.size_n);
    cudaMemsetAsync(N->data, 0, M.size_m * M.size_n * sizeof(int));

    int *data = new int[M.size_m * M.size_n];
    size_t *rows = new size_t[M.size_m * M.size_n];
    size_t *cols = new size_t[M.size_m * M.size_n];
    size_t n = 0;

    for (size_t y = 0; y < M.size_m; y++) {
        for (size_t x = 0; x < M.size_n; x++) {
            int value = M.data[y * M.size_n + x];
            if (value == 0) { continue; }

            data[n] = value;
            rows[n] = y;
            cols[n] = x;
            n++;
        }
    }

    int *d_data;
    size_t *d_rows;
    size_t *d_cols;

    cudaMalloc((void **) &d_data, n * sizeof(int));
    cudaMalloc((void **) &d_rows, n * sizeof(size_t));
    cudaMalloc((void **) &d_cols, n * sizeof(size_t));

    cudaMemcpy(d_data, data, n * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_rows, rows, n * sizeof(size_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_cols, cols, n * sizeof(size_t), cudaMemcpyHostToDevice);
    cudaDeviceSynchronize();

    delete data;
    delete rows;
    delete cols;

    const size_t blocks = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;
    sparse2denseGPU<<<blocks, BLOCK_SIZE>>>(
        N->data, N->size_n,
        d_data, d_rows, d_cols, n
    );
    cudaDeviceSynchronize();

    cudaFree(d_data);
    cudaFree(d_rows);
    cudaFree(d_cols);
    
    return 0;
}
