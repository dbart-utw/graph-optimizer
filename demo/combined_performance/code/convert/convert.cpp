#include "GraphBLAS.h"
#include "LAGraph.h"

#include "datastructures.hpp"

#include "../../../bgo/bc/bc_lagr/bc_lagr.hpp"
#include "../../../bgo/find_max/find_max_gb/find_max_gb.hpp"
#include "../../../bgo/convert/convert/convert.hpp"
#include "../../../bgo/bfs/bfs_naive/bfs_naive.hpp"

#include "convert.hpp"

int workflow_convert(LAGraph_Graph G, CArray<GrB_Index> sources, CArray<int> *level, CArray<int> *parent) {
    GrB_Vector centrality;
    GrB_Index max_node;

    bc_lagr(G, sources, &centrality);

    find_max_index_GB(centrality, &max_node);

    // CONVERSION
    CMatrix<int> G_converted;
    GrB2CM(G->A, &G_converted);

    bfs_naive(G_converted, (int)max_node, level, parent);

    return 0;
}
