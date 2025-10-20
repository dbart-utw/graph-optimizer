#include "GraphBLAS.h"
#include "LAGraph.h"

#include "datastructures.hpp"

#include "bc_lagr.hpp"

int bc_lagr(LAGraph_Graph G, CArray<int> sources, GrB_Vector *centrality) {
    char msg[LAGRAPH_MSG_LEN];

    GrB_Index sources_converted[sources.size];
    for (unsigned int i = 0; i < sources.size; i++) {
        sources_converted[i] = (GrB_Index) sources.data[i];
    }

    if (G->AT == NULL)
        LAGraph_Cached_AT(G, msg);

    return LAGr_Betweenness(centrality, G, sources_converted, sources.size, msg);
}
