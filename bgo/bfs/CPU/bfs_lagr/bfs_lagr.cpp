#include "GraphBLAS.h"
#include "LAGraph.h"

#include "bfs_lagr.hpp"

int bfs_lagr(LAGraph_Graph G, GrB_Index source, GrB_Vector *level, GrB_Vector *parent) {
    char msg[LAGRAPH_MSG_LEN];
    return LAGr_BreadthFirstSearch(level, parent, G, source, msg);
}
