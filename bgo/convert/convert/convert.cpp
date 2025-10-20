#include "GraphBLAS.h"
#include "datastructures.hpp"

#include "convert.hpp"

int GrB2CM(GrB_Matrix M, CMatrix<int> *N) {
    GrB_Index nrows, ncols, nvals;
    GrB_Matrix_nrows(&nrows, M);
    GrB_Matrix_ncols(&ncols, M);
    GrB_Matrix_nvals(&nvals, M);

    N->init(nrows, ncols);

    GrB_Index row_indices[nvals], col_indices[nvals];
    int values[nvals];
    GrB_Matrix_extractTuples_INT32(row_indices, col_indices, values, &nvals, M);

    for (GrB_Index i = 0; i < nvals; i++) {
        N->data[row_indices[i] * ncols + col_indices[i]] = values[i];
    }

    return 0;
}
