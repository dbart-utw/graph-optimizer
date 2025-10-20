#ifndef DENSEVIASPARSE_CPU_GPU_HPP
#define DENSEVIASPARSE_CPU_GPU_HPP

#include "datastructures.hpp"
#include "gpu_datastructures.hpp"


extern "C" int denseviasparseCPU2GPU(CMatrix<int> M, GPU_CMatrix<int> *N);

#endif
