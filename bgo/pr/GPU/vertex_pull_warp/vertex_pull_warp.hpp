#ifndef PR_GPU_VERTEX_PULL_WARP
#define PR_GPU_VERTEX_PULL_WARP

#include "../pagerank.hpp"
#include "datastructures.hpp"

int pr_vertex_pull_warp(GPUPRGraph &g, CArray<float> *pr);

#endif