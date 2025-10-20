#ifndef TC_GAP_HPP
#define TC_GAP_HPP

#include "datastructures.hpp"
#include "gap/generator.h"
#include "gap/benchmark.h"

typedef CSRGraph<int32_t> TCGraph;
int TC_gap(const TCGraph &g, int *triangles);

#endif