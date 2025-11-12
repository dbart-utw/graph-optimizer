#ifndef PR_GPU_VERTEX_PULL_WARP_NODIV
#define PR_GPU_VERTEX_PULL_WARP_NODIV

#include "../pagerank.hpp"
#include "datastructures.hpp"

int pr_vertex_pull_warp_nodiv(CSR &g, CArray<float> *pr);

#endif