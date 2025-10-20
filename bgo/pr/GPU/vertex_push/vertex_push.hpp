#ifndef PR_GPU_VERTEX_PUSH
#define PR_GPU_VERTEX_PUSH

#include "../pagerank.hpp"
#include "datastructures.hpp"

int pr_vertex_push(GPUPRGraph &g, CArray<float> *pr);

#endif