#include "rev_struct_edgelist.hpp"

int pr_rev_struct_edgelist(EdgeStructList  &esl, CArray<float> *pr) {
    pr->init(esl.num_nodes);
    int max_iters = 100;
    PageRankGPU(esl, max_iters, pr->data, REV_STRUCT_EDGELIST);
    
    return 0;
}
