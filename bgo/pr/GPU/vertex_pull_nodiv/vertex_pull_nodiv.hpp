#ifndef PR_GPU_VERTEX_PULL_NODIV
#define PR_GPU_VERTEX_PULL_NODIV

#include "../pagerank.hpp"
#include "datastructures.hpp"

int pr_vertex_pull_nodiv(GPUPRGraph &g, CArray<float> *pr);

#endif