#ifndef PR_GPU_VERTEX_PUSH_WARP
#define PR_GPU_VERTEX_PUSH_WARP

#include "../pagerank.hpp"
#include "datastructures.hpp"

int pr_vertex_push_warp(GPUPRGraph &g, CArray<float> *pr);

#endif