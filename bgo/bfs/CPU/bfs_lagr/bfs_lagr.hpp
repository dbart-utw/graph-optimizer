#ifndef BFS_LAGR_HPP
#define BFS_LAGR_HPP

#include "GraphBLAS.h"
#include "LAGraph.h"

int bfs_lagr(LAGraph_Graph G, GrB_Index source, GrB_Vector *level, GrB_Vector *parent);

#endif