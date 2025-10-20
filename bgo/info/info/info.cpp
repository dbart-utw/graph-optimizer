#include <iostream>

#include "GraphBLAS.h"


int graph_info_GB(GrB_Matrix M) {
    GrB_Index nrows, nvals;
    GrB_Matrix_nrows(&nrows, M);
    GrB_Matrix_nvals(&nvals, M);

    std::cout << "(" << nrows << ", " << nvals << ")" << std::endl;
    return 0;
}
