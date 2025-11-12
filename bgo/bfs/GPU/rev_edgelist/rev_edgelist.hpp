#ifndef BFS_GPU_REV_EDGELIST
#define BFS_GPU_REV_EDGELIST

#include "../bfs.hpp"
#include "datastructures.hpp"

int bfs_rev_edgelist(EdgeListStruct &g, CArray<int32_t> *levels);

#endif