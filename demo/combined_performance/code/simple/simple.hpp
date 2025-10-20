#ifndef SIMPLE_HPP
#define SIMPLE_HPP

#include "GraphBLAS.h"
#include "LAGraph.h"

#include "datastructures.hpp"

int workflow_simple(LAGraph_Graph G, CArray<GrB_Index> sources, GrB_Vector *level, GrB_Vector *parent);

#endif