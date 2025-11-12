#ifndef BFS_GPU_REV_STRUCT_EDGELIST
#define BFS_GPU_REV_STRUCT_EDGELIST

#include "../bfs.hpp"
#include "datastructures.hpp"

int bfs_rev_struct_edgelist(EdgeStructList &esl, CArray<int32_t> *levels);

#endif