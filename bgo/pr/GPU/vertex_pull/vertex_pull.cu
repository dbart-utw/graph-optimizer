#include "vertex_pull.hpp"

int pr_vertex_pull(GPUPRGraph &g, CArray<float> *pr) {
    pr->init(g.num_nodes());
    int max_iters = 100;
    PageRankGPU(g, max_iters, pr->data, VERTEX_PULL);
    
    return 0;
}
