#ifndef SSSP_GAP_HPP
#define SSSP_GAP_HPP

#include "datastructures.hpp"
#include "benchmark.h"

typedef CSRGraph<NodeID, WNode> SSSPGraph;

int SSSP_gap(const SSSPGraph &G, int source, int delta, CArray<int> *distances);

#endif