#ifndef BFS_GPU_EDGELIST
#define BFS_GPU_EDGELIST

#include "../bfs.hpp"
#include "datastructures.hpp"

int bfs_edgelist(EdgeListStruct &g, CArray<int32_t> *levels);

#endif