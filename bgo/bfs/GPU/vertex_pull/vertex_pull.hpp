#ifndef BFS_GPU_VERTEX_PULL
#define BFS_GPU_VERTEX_PULL

#include "../bfs.hpp"
#include "datastructures.hpp"

int bfs_vertex_pull(CSR &g, CArray<int32_t> *levels);

#endif