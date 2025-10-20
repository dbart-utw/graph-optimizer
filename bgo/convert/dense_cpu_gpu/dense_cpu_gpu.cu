#include "datastructures.hpp"
#include "gpu_datastructures.hpp"

#include "dense_cpu_gpu.hpp"

#include <iostream>

extern "C" int denseCPU2GPU(CMatrix<int> M, GPU_CMatrix<int> *N) {
    N->init(M.size_m, M.size_n);
    
    cudaMemcpy(N->data, M.data, M.size_m * M.size_n * sizeof(int), cudaMemcpyHostToDevice);
    cudaDeviceSynchronize();
    
    return 0;
}
