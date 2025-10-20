#ifndef BC_BRANDES_HPP
#define BC_BRANDES_HPP

#include "datastructures.hpp"

int bc_brandes(CMatrix<int> G, CArray<int> sources, CArray<int> *centrality);

#endif