#ifndef CONVERT_HPP
#define CONVERT_HPP

#include "GraphBLAS.h"
#include "LAGraph.h"

#include "datastructures.hpp"

int workflow_convert(LAGraph_Graph G, CArray<GrB_Index> sources, CArray<int> *level, CArray<int> *parent);

#endif
