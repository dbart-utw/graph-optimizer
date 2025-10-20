#ifndef BFS_GAP_HPP
#define BFS_GAP_HPP

#include "datastructures.hpp"

typedef CSRGraph<int32_t> BFSGraph;

int bfs_gap(BFSGraph &G, int source, CArray<int> *level, CArray<int> *parent);

#endif