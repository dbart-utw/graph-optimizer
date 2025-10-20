#ifndef PR_SEQUENTIAL_HPP
#define PR_SEQUENTIAL_HPP

#include "datastructures.hpp"

const float d = 0.85;
const float epsilon = 0.0001;
const int max_iter = 100;

int pagerank(CMatrix<int> G, CArray<float> *PR);

#endif