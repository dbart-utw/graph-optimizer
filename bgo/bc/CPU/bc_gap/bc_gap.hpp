#ifndef BC_GAP_HPP
#define BC_GAP_HPP

#include "datastructures.hpp"

typedef CSRGraph<int32_t> BCGraph;

int bc_gap(const BCGraph &G, CArray<int> sources, CArray<int> *centrality);

#endif