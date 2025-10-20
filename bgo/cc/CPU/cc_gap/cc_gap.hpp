#ifndef CC_GAP_HPP
#define CC_GAP_HPP

#include "datastructures.hpp"

typedef CSRGraph<int32_t> CCGraph;

int cc_gap(const CCGraph &G, CArray<int> *components);

#endif