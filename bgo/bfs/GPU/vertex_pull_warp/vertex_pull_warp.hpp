#ifndef BFS_GPU_VERTEX_PULL_WARP
#define BFS_GPU_VERTEX_PULL_WARP

#include "../bfs.hpp"
#include "datastructures.hpp"

int bfs_vertex_pull_warp(CSR &g, CArray<int32_t> *levels);

#endif