#include "rev_edgelist.hpp"

int bfs_rev_edgelist(EdgeListStruct &els, CArray<int32_t> *levels) {
    levels->init(els.num_nodes);
    BFSGPU<Reduction<normal>>(els, levels->data, REV_EDGELIST);
    
    return 0;
}