#ifndef PR_GPU_VERTEX_PULL
#define PR_GPU_VERTEX_PULL

#include "../pagerank.hpp"
#include "datastructures.hpp"

int pr_vertex_pull(GPUPRGraph &g, CArray<float> *pr);

#endif