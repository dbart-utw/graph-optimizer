/* All PageRank implementations. */
#ifndef PAGE_RANK_HPP
#define PAGE_RANK_HPP

#include <algorithm>
#include <cuda_runtime.h>
#include "gpu_utils.hpp"
#include "gap/graph.h"
#include "gap/timer.h"

const float dampening = 0.85f;
const float epsilon = 0.001f;

void resetDiff();
float getDiff();

__global__ void consolidateRank(int32_t, float *pagerank, float *new_pagerank);
__global__ void consolidateRankNoDiv(int32_t size, int32_t* degrees, float *pagerank, float *new_pagerank, bool notLast);

__global__ void edgeListPageRank(int32_t size, int32_t *sources, int32_t *destinations, int32_t *degrees, float *pagerank, float *new_pagerank);
__global__ void revEdgeListPageRank(int32_t size, int32_t *sources, int32_t *destinations, int32_t *degrees, float *pagerank, float *new_pagerank);
__global__ void structEdgeListPageRank(int32_t size, EdgeStruct *edges, int32_t *degrees, float *pagerank, float *new_pagerank);
__global__ void revStructEdgeListPageRank(int32_t size, EdgeStruct *edges, int32_t *degrees, float *pagerank, float *new_pagerank);
__global__ void vertexPullPageRank(size_t, size_t, int32_t size, int32_t *in_index, int32_t *in_neighs, int32_t *degrees, float *pagerank, float *new_pagerank);
__global__ void vertexPullNoDivPageRank(size_t, size_t, int32_t size, int32_t *in_index, int32_t *in_neighs, int32_t *, float *pagerank, float *new_pagerank);
__global__ void vertexPullWarpPageRank(size_t warp_size, size_t chunk_size, int32_t size, int32_t *in_index, int32_t *in_neigs, int32_t *degrees, float *pagerank, float *new_pagerank);
__global__ void vertexPullWarpNoDivPageRank(size_t warp_size, size_t chunk_size, int32_t size, int32_t *in_index, int32_t *in_neigs, int32_t *, float *pagerank, float *new_pagerank);
__global__ void vertexPushPageRank(size_t, size_t, int32_t size, int32_t *out_index, int32_t *out_neighs, int32_t *, float *pagerank, float *new_pagerank);
__global__ void vertexPushWarpPageRank(size_t warp_size, size_t chunk_size, int32_t size, int32_t *out_index, int32_t *out_neighs, int32_t *, float *pagerank, float *new_pagerank);


double PageRankGPU(CSR &g, int max_iters, float *pagerank, GPU_Implementation impl);
double PageRankGPU(EdgeListStruct &els, int max_iters, float *pagerank, GPU_Implementation impl);
double PageRankGPU(EdgeStructList &esl, int max_iters, float *pagerank, GPU_Implementation impl);


void PageRankGAP(CSR &g, int max_iters, float *pagerank);

#endif