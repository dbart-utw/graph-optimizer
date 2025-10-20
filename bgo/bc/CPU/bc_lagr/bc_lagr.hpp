#ifndef BC_LAGR_HPP
#define BC_LAGR_HPP

#include "GraphBLAS.h"
#include "LAGraph.h"

#include "datastructures.hpp"

int bc_lagr(LAGraph_Graph G, CArray<int> sources, GrB_Vector *centrality);

#endif
