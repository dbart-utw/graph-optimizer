#include "rev_edgelist.hpp"

int pr_rev_edgelist(EdgeListStruct &els, CArray<float> *pr) {
    pr->init(els.num_nodes);
    int max_iters = 100;
    PageRankGPU(els, max_iters, pr->data, REV_EDGELIST);
    
    return 0;
}
