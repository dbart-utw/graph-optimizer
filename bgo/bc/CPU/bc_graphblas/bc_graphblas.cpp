
#include "GraphBLAS.h"

#include "datastructures.hpp"

#include  "bc_graphblas.hpp"

GrB_Info BC_GB(GrB_Vector *delta, GrB_Matrix A, GrB_Index *s, GrB_Index nsver)
{
    GrB_Index n;
    GrB_Matrix_nrows(&n, A);                             // n = # of vertices in graph
    GrB_Vector_new(delta,GrB_FP32,n);                    // Vector<float> delta(n)

    GrB_Monoid Int32Add;                                 // Monoid <int32_t,+,0>
    GrB_Monoid_new_INT32(&Int32Add,GrB_PLUS_INT32,0);
    GrB_Semiring Int32AddMul;                            // Semiring <int32_t,int32_t,int32_t,+,*,0>
    GrB_Semiring_new(&Int32AddMul,Int32Add,GrB_TIMES_INT32);

    // Descriptor for BFS phase mxm
    GrB_Descriptor desc_tsr;
    GrB_Descriptor_new(&desc_tsr);
    GrB_Descriptor_set(desc_tsr,GrB_INP0,GrB_TRAN);      // transpose the adjacency matrix
    GrB_Descriptor_set(desc_tsr,GrB_MASK,GrB_COMP);      // complement the mask
    GrB_Descriptor_set(desc_tsr,GrB_OUTP,GrB_REPLACE);   // clear output before result is stored

    // index and value arrays needed to build numsp
    GrB_Index *i_nsver = (GrB_Index*)malloc(sizeof(GrB_Index)*nsver);
    int32_t   *ones    = (int32_t*)  malloc(sizeof(int32_t)*nsver);
    for(unsigned int i=0; i<nsver; ++i) {
        i_nsver[i] = i;
        ones[i] = 1;
    }

    // numsp: structure holds the number of shortest paths for each node and starting vertex
    // discovered so far.  Initialized to source vertices:  numsp[s[i],i]=1, i=[0,nsver)
    GrB_Matrix numsp;
    GrB_Matrix_new(&numsp, GrB_INT32, n, nsver);
    GrB_Matrix_build_INT32(numsp,s,i_nsver,ones,nsver,GrB_PLUS_INT32);
    free(i_nsver); free(ones);

    // frontier: Holds the current frontier where values are path counts.
    // Initialized to out vertices of each source node in s.
    GrB_Matrix frontier;
    GrB_Matrix_new(&frontier, GrB_INT32, n, nsver);
    GrB_Matrix_extract(frontier,numsp,GrB_NULL,A,GrB_ALL,n,s,nsver,desc_tsr);

    // sigma: stores frontier information for each level of BFS phase.  The memory
    // for an entry in sigmas is only allocated within the do-while loop if needed
    GrB_Matrix *sigmas = (GrB_Matrix*)malloc(sizeof(GrB_Matrix)*n);   // n is an upper bound on diameter

    int32_t   d = 0;                                       // BFS level number
    GrB_Index nvals = 0;                                   // nvals == 0 when BFS phase is complete

    // --------------------- The BFS phase (forward sweep) ---------------------------
    do {
        // sigmas[d](:,s) = d^th level frontier from source vertex s
        GrB_Matrix_new(&(sigmas[d]), GrB_BOOL, n, nsver);

        GrB_Matrix_apply(sigmas[d],GrB_NULL,GrB_NULL,
                    GrB_IDENTITY_BOOL,frontier,GrB_NULL);    // sigmas[d](:,:) = (Boolean) frontier
        GrB_Matrix_eWiseAdd_Monoid(numsp,GrB_NULL,GrB_NULL,
                        Int32Add,numsp,frontier,GrB_NULL);    // numsp += frontier (accum path counts)
        GrB_mxm(frontier,numsp,GrB_NULL,
                Int32AddMul,A,frontier,desc_tsr);          // f<!numsp> = A' +.* f (update frontier)
        GrB_Matrix_nvals(&nvals,frontier);                 // number of nodes in frontier at this level
        d++;
    } while (nvals);

    GrB_Monoid FP32Add;                                  // Monoid <float,+,0.0>
    GrB_Monoid_new_FP32(&FP32Add,GrB_PLUS_FP32,0.0f);
    GrB_Monoid FP32Mul;                                  // Monoid <float,*,1.0>
    GrB_Monoid_new_FP32(&FP32Mul,GrB_TIMES_FP32,1.0f);
    GrB_Semiring FP32AddMul;                             // Semiring <float,float,float,+,*,0.0>
    GrB_Semiring_new(&FP32AddMul,FP32Add,GrB_TIMES_FP32);

    // nspinv: the inverse of the number of shortest paths for each node and starting vertex.  |\label{line:nspinv}|
    GrB_Matrix nspinv;
    GrB_Matrix_new(&nspinv,GrB_FP32,n,nsver);
    GrB_Matrix_apply(nspinv,GrB_NULL,GrB_NULL,
            GrB_MINV_FP32,numsp,GrB_NULL);             // nspinv = 1./numsp

    // bcu: BC updates for each vertex for each starting vertex in s
    GrB_Matrix bcu;
    GrB_Matrix_new(&bcu,GrB_FP32,n,nsver);
    GrB_Matrix_assign_FP32(bcu,GrB_NULL,GrB_NULL,
                1.0f,GrB_ALL,n, GrB_ALL,nsver,GrB_NULL);  // filled with 1 to avoid sparsity issues

    // Descriptor used in the tally phase
    GrB_Descriptor desc_r;
    GrB_Descriptor_new(&desc_r);
    GrB_Descriptor_set(desc_r,GrB_OUTP,GrB_REPLACE);     // clear output before result is stored

    GrB_Matrix w;                                        // temporary workspace matrix
    GrB_Matrix_new(&w,GrB_FP32,n,nsver);

    // -------------------- Tally phase (backward sweep) --------------------
    for (int i=d-1; i>0; i--)  {
        GrB_Matrix_eWiseMult_Monoid(w,sigmas[i],GrB_NULL,
                        FP32Mul,bcu,nspinv,desc_r);          // w<sigmas[i]>=(1 ./ nsp).*bcu

        // add contributions by successors and mask with that BFS level's frontier
        GrB_mxm(w,sigmas[i-1],GrB_NULL,
                FP32AddMul,A,w,desc_r);                    // w<sigmas[i-1]> = (A +.* w)
        GrB_Matrix_eWiseMult_Monoid(bcu,GrB_NULL,GrB_PLUS_FP32,
                        FP32Mul,w,numsp,GrB_NULL);           // bcu += w .* numsp
    }

    // subtract "nsver" from every entry in delta (account for 1 extra value per bcu element)
    GrB_Vector_assign_FP32(*delta,GrB_NULL,GrB_NULL,
                -(float)nsver,GrB_ALL,n,GrB_NULL);        // fill with -nsver
    GrB_Matrix_reduce_BinaryOp(*delta,GrB_NULL,GrB_PLUS_FP32,
                GrB_PLUS_FP32,bcu,GrB_NULL);              // add all updates to -nsver

    // Release resources
    for(int i=0; i<d; i++) {
        GrB_Matrix_free(&(sigmas[i]));
    }
    free(sigmas);

    GrB_Matrix_free(&frontier);
    GrB_Matrix_free(&numsp);
    GrB_Matrix_free(&nspinv);
    GrB_Matrix_free(&bcu);
    GrB_Matrix_free(&w);
    GrB_Descriptor_free(&desc_tsr);
    GrB_Descriptor_free(&desc_r);
    GrB_Semiring_free(&Int32AddMul);
    GrB_Monoid_free(&Int32Add);
    GrB_Semiring_free(&FP32AddMul);
    GrB_Monoid_free(&FP32Add);
    GrB_Monoid_free(&FP32Mul);

    return GrB_SUCCESS;
}

int bc_graphblas(GrB_Matrix G, CArray<int> sources, GrB_Vector *centrality) {
    GrB_Index sources_converted[sources.size];
    for (unsigned int i = 0; i < sources.size; i++) {
        sources_converted[i] = (GrB_Index) sources.data[i];
    }
    return BC_GB(centrality, G, sources_converted, sources.size);
}
