#include "GraphBLAS.h"
#include "LAGraph.h"

#include "datastructures.hpp"

#include "../../../bgo/bc/bc_lagr/bc_lagr.hpp"
#include "../../../bgo/find_max/find_max_gb/find_max_gb.hpp"
#include "../../../bgo/bfs/bfs_lagr/bfs_lagr.hpp"

#include "simple.hpp"

int workflow_simple(LAGraph_Graph G, CArray<GrB_Index> sources, GrB_Vector *level, GrB_Vector *parent) {
    GrB_Vector centrality;
    GrB_Index max_node;

    bc_lagr(G, sources, &centrality);

    find_max_index_GB(centrality, &max_node);

    bfs_lagr(G, max_node, level, parent);

    return 0;
}
