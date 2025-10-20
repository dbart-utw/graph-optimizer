#ifndef DENSE_CPU_GPU_HPP
#define DENSE_CPU_GPU_HPP

#include "datastructures.hpp"
#include "gpu_datastructures.hpp"


extern "C" int denseCPU2GPU(CMatrix<int> M, GPU_CMatrix<int> *N);

#endif
