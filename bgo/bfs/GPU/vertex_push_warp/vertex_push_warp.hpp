#ifndef BFS_GPU_VERTEX_PUSH_WARP
#define BFS_GPU_VERTEX_PUSH_WARP

#include "../bfs.hpp"
#include "datastructures.hpp"

int bfs_vertex_push_warp(CSR &g, CArray<int32_t> *levels);

#endif