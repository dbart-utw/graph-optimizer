#include "vertex_pull_nodiv.hpp"

int pr_vertex_pull_nodiv(GPUPRGraph &g, CArray<float> *pr) {
    pr->init(g.num_nodes());
    int max_iters = 100;
    PageRankGPU(g, max_iters, pr->data, VERTEX_PULL_NODIV);
    
    return 0;
}
