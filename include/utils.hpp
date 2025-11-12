#ifndef UTILS_HPP
#define UTILS_HPP

#include "GraphBLAS.h"
#include "LAGraph.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <cassert>

#include "datastructures.hpp"

template <typename T>
void pretty_print_array(T *arr, int n, std::string name);
template <typename T>
void GB_matrix_to_vector_array(GrB_Matrix M, std::vector<T> *G);
void read_graph_GB(GrB_Matrix *M, char *fileName);
void write_graph_GB(GrB_Matrix M, char *filename);
void read_graph_LA(LAGraph_Graph *G, char *filename);
void write_graph_LA(LAGraph_Graph G, char *filename);
template <typename T>
void read_graph_CMatrix(CMatrix<T> *G, char* filename);
template <>
void read_graph_CMatrix<int>(CMatrix<int> *G, char* filename);
template <>
void read_graph_CMatrix<float>(CMatrix<float> *G, char* filename);
template <typename T>
void write_graph_CMatrix(CMatrix<T> G, char* filename);
template <typename T>
void read_vector_CArray(CArray<T> *V, char* filename);
template <>
void read_vector_CArray<float>(CArray<float> *V, char* filename);
template <>
void read_vector_CArray<int>(CArray<int> *V, char* filename);
template <typename T>
void write_vector_CArray(CArray<T> V, char *filename);
void read_vector_GB(GrB_Vector *V, char *filename);
void write_vector_GB(GrB_Vector V, char *filename);
template <typename T>
void read_graph_vector_array(std::vector<T> *G, char *fileName);
template <typename T>
void pretty_print_vector(GrB_Vector v, int n, std::string name);
template <typename T>
void pretty_print_CMatrix(CMatrix<T> M);
void pretty_print_CMatrix(CMatrix<float> M);

#endif
