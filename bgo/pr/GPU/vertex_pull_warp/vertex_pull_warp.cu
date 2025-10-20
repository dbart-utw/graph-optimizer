#include "vertex_pull_warp.hpp"

int pr_vertex_pull_warp(GPUPRGraph &g, CArray<float> *pr) {
    pr->init(g.num_nodes());
    int max_iters = 100;
    PageRankGPU(g, max_iters, pr->data, VERTEX_PULL_WARP);
    
    return 0;
}
