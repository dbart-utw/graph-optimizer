#include "GraphBLAS.h"

#include "find_max_gb.hpp"


int find_max_index_GB(GrB_Vector values, GrB_Index *index) {
    GrB_Index nvals;
    GrB_Vector_nvals(&nvals, values);
    float value;
    int max_value = 0;
    for (GrB_Index i = 0; i < nvals; i++) {
        GrB_Info info = GrB_Vector_extractElement_FP32(&value, values, i);
        if (info != GrB_SUCCESS) { continue; }
        if (value <= max_value) { continue; }

        max_value = value;
        *index = i;
    }
    return 0;
}
