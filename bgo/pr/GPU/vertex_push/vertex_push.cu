#include "vertex_push.hpp"

int pr_vertex_push(GPUPRGraph &g, CArray<float> *pr) {
    pr->init(g.num_nodes());
    int max_iters = 100;
    PageRankGPU(g, max_iters, pr->data, VERTEX_PUSH);
    
    return 0;
}
