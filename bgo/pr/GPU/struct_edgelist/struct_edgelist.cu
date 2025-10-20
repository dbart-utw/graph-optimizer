#include "struct_edgelist.hpp"

int pr_struct_edgelist(EdgeStructList  &esl, CArray<float> *pr) {
    pr->init(esl.num_nodes);
    int max_iters = 100;
    PageRankGPU(esl, max_iters, pr->data, STRUCT_EDGELIST);
    
    return 0;
}
