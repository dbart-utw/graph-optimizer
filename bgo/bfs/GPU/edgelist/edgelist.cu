#include "edgelist.hpp"

int bfs_edgelist(EdgeListStruct &els, CArray<int32_t> *levels) {
    levels->init(els.num_nodes);
    BFSGPU<Reduction<normal>>(els, levels->data, EDGELIST);
    
    return 0;
}