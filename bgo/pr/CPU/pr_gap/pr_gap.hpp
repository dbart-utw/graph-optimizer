#ifndef PR_GAP_HPP
#define PR_GAP_HPP

#include "datastructures.hpp"

typedef CSRGraph<int32_t> PRGraph;

int PR_gap(const PRGraph &G, CArray<float> *PR);

#endif