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
void pretty_print_array(T *arr, int n, std::string name) {
    std::cout << name.c_str() << ": [";
    for (int i = 0; i < n - 1; i++) {
        std::cout << std::to_string(arr[i]) << ", ";
    }
    std::cout << std::to_string(arr[n - 1]) << "]" << std::endl;
}

/*
 * Convert a matrix M to a vector array G.
 */
template <typename T>
void GB_matrix_to_vector_array(GrB_Matrix M, std::vector<T> *G) {
    GrB_Index num_nodes;
    GrB_Matrix_nrows(&num_nodes, M);

    G->resize(num_nodes);
    for (uint i = 0; i < num_nodes; i++) {
        G[i].resize(num_nodes);
    }

    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, M);

    GrB_Index row_indices[nvals];
    GrB_Index col_indices[nvals];
    float values[nvals];

    GrB_Matrix_extractTuples_FP32(row_indices, col_indices, values, &nvals, M);

    for (uint i = 0; i < nvals; i++) {
        G[row_indices[i]][col_indices[i]] = values[i];
    }
}

/*
 * Read a graph into matrix M given a specified fileName.
 */
void read_graph_GB(GrB_Matrix *M, char *fileName) {
    FILE *fd = fopen(fileName, "r");
    char msg[LAGRAPH_MSG_LEN];

    if (GrB_SUCCESS != LAGraph_MMRead(M, fd, msg))
    {
        fprintf(stderr, "ERROR: Failed to load graph: %s\n", fileName);
        fprintf(stderr, "message: %s\n", msg);
        exit(-1);
    }

    if (fd != NULL)
        fclose(fd);
}

void write_graph_GB(GrB_Matrix M, char *filename) {
    FILE *fd = fopen(filename, "w");
    char msg[LAGRAPH_MSG_LEN];

    // NOTE: This output is garbage when matrix is dense!
    if (GrB_SUCCESS != LAGraph_MMWrite(M, fd, NULL, msg)) {
        fprintf(stderr, "ERROR: Failed to save graph: %s\n", filename);
        exit(-1);
    }

    if (fd != NULL) {
        fclose(fd);
    }
}

void read_graph_LA(LAGraph_Graph *G, char *filename) {
    char msg[LAGRAPH_MSG_LEN];
    GrB_Matrix M;
    read_graph_GB(&M, filename);
    LAGraph_New(G, &M, LAGraph_ADJACENCY_DIRECTED, msg);

    // Set G->AT to the transpose of G->A
    GrB_Matrix AT;
    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);  // Create a descriptor for transposition
    GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);  // Set input to transpose

    // Compute the transpose of the matrix G->A
    GrB_Matrix_dup(&AT, (*G)->A);  // Duplicate the adjacency matrix
    GrB_transpose(AT, NULL, NULL, (*G)->A, desc);  // Perform the transpose
    (*G)->AT = AT;

    uint64_t n_triangles;
    LAGraph_TriangleCount(&n_triangles, *G, msg);

    // Clean up the descriptor
    GrB_Descriptor_free(&desc);
}

void write_graph_LA(LAGraph_Graph G, char *filename) {
    write_graph_GB(G->A, filename);
}

template <typename T>
void read_graph_CMatrix(CMatrix<T> *G, char* filename) {

}

template <>
void read_graph_CMatrix<int>(CMatrix<int> *G, char* filename) {
    GrB_Matrix M;
    read_graph_GB(&M, filename);

    GrB_Index nrows;
    GrB_Matrix_nrows(&nrows, M);

    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, M);

    GrB_Index row_indices[nvals];
    GrB_Index col_indices[nvals];
    int values[nvals] = {0};
    GrB_Matrix_extractTuples_INT32(row_indices, col_indices, values, &nvals, M);

    G->init(nrows, nrows);
    for (GrB_Index i = 0; i < nvals; i++) {
        G->data[row_indices[i] * nrows + col_indices[i]] = values[i];
    }
}

template <>
void read_graph_CMatrix<float>(CMatrix<float> *G, char* filename) {
    GrB_Matrix M;
    read_graph_GB(&M, filename);

    GrB_Index nrows;
    GrB_Matrix_nrows(&nrows, M);

    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, M);

    GrB_Index row_indices[nvals];
    GrB_Index col_indices[nvals];
    float values[nvals] = {0.0};
    GrB_Matrix_extractTuples_FP32(row_indices, col_indices, values, &nvals, M);

    G->init(nrows, nrows);
    for (GrB_Index i = 0; i < nvals; i++) {
        G->data[row_indices[i] * nrows + col_indices[i]] = values[i];
    }
}

template <typename T>
void write_graph_CMatrix(CMatrix<T> G, char* filename) {
    GrB_Matrix M;
    if (GrB_SUCCESS != GrB_Matrix_new(&M, GrB_FP32, G.size_m, G.size_n)) {
        std::cerr << "Failed to create matrix!" << std::endl;
        exit(-1);
    }

    for (size_t y = 0; y < G.size_m; y++) {
        for (size_t x = 0; x < G.size_n; x++) {
            T value = G.data[y * G.size_n + y];
            if (value == 0.0) { continue; }

            if (GrB_SUCCESS != GrB_Matrix_setElement_FP64(M, value, y, x)) {
                std::cerr << "Failed to set matrix element at (" << x << ", " << y << ")" << std::endl;
                exit(-1);
            }
        }
    }

    write_graph_GB(M, filename);
    GrB_Matrix_free(&M);
}

template <typename T>
void read_vector_CArray(CArray<T> *V, char* filename) {
    // This would be the generic implementation (if needed)
}

// Specialization for float
template <>
void read_vector_CArray<float>(CArray<float> *V, char* filename) {
    GrB_Matrix M;
    read_graph_GB(&M, filename);

    GrB_Index ncols;
    GrB_Matrix_ncols(&ncols, M);

    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, M);

    GrB_Index row_indices[nvals];
    GrB_Index col_indices[nvals];
    float values[nvals];
    GrB_Matrix_extractTuples_FP32(row_indices, col_indices, values, &nvals, M);

    V->init(ncols);
    for (GrB_Index i = 0; i < nvals; i++) {
        V->data[col_indices[i]] = values[i];
    }
}

// Specialization for int
template <>
void read_vector_CArray<int>(CArray<int> *V, char* filename) {
    GrB_Matrix M;
    read_graph_GB(&M, filename);

    GrB_Index ncols;
    GrB_Matrix_ncols(&ncols, M);

    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, M);

    GrB_Index row_indices[nvals];
    GrB_Index col_indices[nvals];
    int values[nvals];
    GrB_Matrix_extractTuples_INT32(row_indices, col_indices, values, &nvals, M);

    V->init(ncols);
    for (GrB_Index i = 0; i < nvals; i++) {
        V->data[col_indices[i]] = values[i];
    }
}

template <typename T>
void write_vector_CArray(CArray<T> V, char *filename) {
    CMatrix<T> M;
    M.init(1, V.size);
    std::memcpy(M.data, V.data, V.size * sizeof(T));
    write_graph_CMatrix(M, filename);
}

void read_vector_GB(GrB_Vector *V, char *filename) {
    GrB_Matrix M;
    read_graph_GB(&M, filename);

    GrB_Index nrows, ncols, nvals;
    GrB_Matrix_nrows(&nrows, M);
    GrB_Matrix_ncols(&ncols, M);
    GrB_Matrix_nvals(&nvals, M);
    assert(nrows == 1 && ncols > 0);
    GrB_Vector_new(V, GrB_INT32, ncols);

    GrB_Index row_indices[nvals];
    GrB_Index col_indices[nvals];
    int values[nvals];
    GrB_Matrix_extractTuples_INT32(row_indices, col_indices, values, &nvals, M);

    for (GrB_Index i = 0; i < nvals; i++) {
        GrB_Vector_setElement_INT32(*V, values[i], col_indices[i]);
    }

    GrB_Matrix_free(&M);
}

void write_vector_GB(GrB_Vector V, char *filename) {
    GrB_Index size;
    GrB_Vector_size(&size, V);

    GrB_Matrix M;
    GrB_Matrix_new(&M, GrB_INT32, 1, size);

    GrB_Index indices[size];
    int values[size];
    GrB_Vector_extractTuples_INT32(indices, values, &size, V);

    for (GrB_Index i = 0; i < size; i++) {
        if (values[i] == 0) { continue; }
        GrB_Matrix_setElement_INT32(M, values[i], 0, indices[i]);
    }

    write_graph_GB(M, filename);
    GrB_Matrix_free(&M);
}

/*
 * Read a graph into vector array G given a specified fileName.
 */
template <typename T>
void read_graph_vector_array(std::vector<T> *G, char *fileName) {
    GrB_Matrix A;
    read_graph_GB(&A, fileName);

    GB_matrix_to_vector_array<T>(A, G);
}

template <typename T>
void pretty_print_vector(GrB_Vector v, int n, std::string name) {
    GrB_Index N = n;
    GrB_Index indices[n];
    T values[n];
    GrB_Vector_extractTuples_FP32(indices, values, &N, v);
    std::cout << name.c_str() << ": [";
    for (uint i = 0; i < n - 1; i++) {
        std::cout << std::to_string(values[i]) << ", ";
    }
    std::cout << std::to_string(values[n-1]) << "]" << std::endl;
}

template <typename T>
void pretty_print_CMatrix(CMatrix<T> M) {
    for (size_t y = 0; y < M.size_m; y++) {
        for (size_t x = 0; x < M.size_n; x++) {
            std::cout << M.data[y * M.size_n + x] << " ";
        }
        std::cout << std::endl;
    }
}

void pretty_print_CMatrix(CMatrix<float> M) {
    for (size_t y = 0; y < M.size_m; y++) {
        for (size_t x = 0; x < M.size_n; x++) {
            std::cout << M.data[y * M.size_n + x] << " ";
        }
        std::cout << std::endl;
    }
}

#endif
