// Modified LAGraph implementation of PageRank for the GAP benchmark.
// Removed error checks, and set a fixed number of iterations.

#include "GraphBLAS.h"
#include "LAGraph.h"
#include "datastructures.hpp"
#include "pr_gb.hpp"

#define LG_FREE_WORK                \
{                                   \
    GrB_Vector_free (&d1) ;                \
    GrB_Vector_free (&d) ;                 \
    GrB_Vector_free (&t) ;                 \
    GrB_Vector_free (&w) ;                 \
}

#define LG_FREE_ALL                 \
{                                   \
    LG_FREE_WORK ;                  \
    GrB_Vector_free (&r) ;                 \
}

int pr_gb(LAGraph_Graph G, GrB_Vector *PR) {
    float damping = 0.85;
    int iter = 10000;

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Vector r = NULL, d = NULL, t = NULL, w = NULL, d1 = NULL ;
    GrB_Matrix AT ;
    if (G->kind == LAGraph_ADJACENCY_UNDIRECTED ||
        G->is_symmetric_structure == LAGraph_TRUE)
    {
        // A and A' have the same structure
        AT = G->A ;
    }
    else
    {
        // A and A' differ
        AT = G->AT ;
    }
    GrB_Vector d_out = G->out_degree ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Index n ;
    (*PR) = NULL ;
    GrB_Matrix_nrows(&n, AT);

    const float scaled_damping = (1 - damping) / n ;
    const float teleport = scaled_damping ; // teleport = (1 - damping) / n

    // r = 1 / n
    GrB_Vector_new(&t, GrB_FP32, n);
    GrB_Vector_new(&r, GrB_FP32, n);
    GrB_Vector_new(&w, GrB_FP32, n);
    GrB_Vector_assign_FP32(r, NULL, NULL, (float) (1.0 / n), GrB_ALL, n, NULL);

    // prescale with damping factor, so it isn't done each iteration
    // d = d_out / damping ;
    GrB_Vector_new(&d, GrB_FP32, n);
    GrB_Vector_apply_BinaryOp2nd_FP32(d, NULL, NULL, GrB_DIV_FP32, d_out, damping, NULL);

    // d1 = 1 / damping
    float dmin = 1.0 / damping ;
    GrB_Vector_new(&d1, GrB_FP32, n);
    GrB_Vector_assign_FP32(d1, NULL, NULL, dmin, GrB_ALL, n, NULL);
    // d = max (d1, d)
    GrB_Vector_eWiseAdd_BinaryOp(d, NULL, NULL, GrB_MAX_FP32, d1, d, NULL);
    GrB_Vector_free(&d1) ;

    //--------------------------------------------------------------------------
    // pagerank iterations
    //--------------------------------------------------------------------------
    for (int i = 0 ; i < iter; i++)
    {
        // swap t and r ; now t is the old score
        GrB_Vector temp = t ; t = r ; r = temp ;
        // w = t ./ d
        GrB_Vector_eWiseMult_BinaryOp(w, NULL, NULL, GrB_DIV_FP32, t, d, NULL);
        // r = teleport
        GrB_Vector_assign_FP32(r, NULL, NULL, teleport, GrB_ALL, n, NULL);
        // r += A'*w
        GrB_mxv(r, NULL, GrB_PLUS_FP32, LAGraph_plus_second_fp32, AT, w, NULL);
        // t -= r
        GrB_Vector_assign(t, NULL, GrB_MINUS_FP32, r, GrB_ALL, n, NULL);
        // t = abs (t)
        GrB_Vector_apply(t, NULL, NULL, GrB_ABS_FP32, t, NULL);
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    (*PR) = r;
    LG_FREE_WORK;
    return GrB_SUCCESS;
}
