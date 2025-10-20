#include "pr_sequential.hpp"
#include "pr_gb.hpp"
#include "datastructures.hpp"
#include "utils.hpp"
#include "time.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <graph-file>" << std::endl;
        return -1;
    }
    char msg[LAGRAPH_MSG_LEN];
    LAGraph_Init(msg); // DON'T FORGET!

    CMatrix<int> G;
    read_graph_CMatrix<int>(&G, argv[1]);

    CArray<float> PR(G.size_n);

    // GAP implementation
    clock_t start = clock();
    pagerank(G, &PR);
    clock_t end = clock();
    double elapsed_msecs = double(end - start) / CLOCKS_PER_SEC * 1000;
    printf("Elapsed time GAP: %f\n", elapsed_msecs);

    // LAGraph implementation
    GrB_Vector LAGraph_PR;
    GrB_Vector_new(&LAGraph_PR, GrB_FP32, G.size_n);
    LAGraph_Graph M;
    int iters;
    read_graph_LA(&M, argv[1]);

    start = clock();
    pr_gb(M, &LAGraph_PR);
    end = clock();
    elapsed_msecs = double(end - start) / CLOCKS_PER_SEC * 1000;
    printf("Elapsed time GraphBLAS: %f\n", elapsed_msecs);

    GrB_Vector_free(&LAGraph_PR);
    LAGraph_Finalize(msg);
    return 0;
}