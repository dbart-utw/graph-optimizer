#include "edgelist.hpp"

int pr_edgelist(EdgeListStruct &els, CArray<float> *pr) {
    pr->init(els.num_nodes);
    int max_iters = 100;
    PageRankGPU(els, max_iters, pr->data, EDGELIST);
    
    return 0;
}
