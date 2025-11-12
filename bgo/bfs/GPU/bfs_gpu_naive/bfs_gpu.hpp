#ifndef BFS_GPU_HPP
#define BFS_GPU_HPP

#include "gpu_datastructures.hpp"

extern "C" int bfs_gpu(GPU_CMatrix<int> G, int source, GPU_CArray<int> *level, GPU_CArray<int> *parent);

#endif
