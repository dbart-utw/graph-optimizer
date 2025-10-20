#include <stdio.h>
#include <omp.h>
#include <time.h>
#include "GraphBLAS.h"
#include "suitesparse/LAGraph.h"
#include "../bgo/betweenness_centrality.hpp"
#include "../bgo/find_max.hpp"
#include "../include/utils.hpp"
#include "../bgo/find_path.hpp"
#include "../bgo/bfs_gpu.cuh"

bool PARALLEL_EXECUTION = false;
char msg[LAGRAPH_MSG_LEN] ;
clock_t start_time;
double time_elapsed;


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Error\nUsage: %s <matrix_market_file.mtx>\n", argv[0]);
        return 1;
    }

    LAGraph_Init(msg);

    GrB_Matrix A, B;

    read_graph_GB(&A, argv[1]);

    GrB_Index num_nodes;
    GrB_Matrix_nrows(&num_nodes, A);

    GrB_Matrix_dup(&B, A);

    /***************************************************************************
     * BGO 1: filtering, using connected components
     **************************************************************************/
    // GrB_Vector cc;
    // LAGr_ConnectedComponents(A, &cc);
    // pretty_print_vector_FP64(cc, "Connected Components");

    /*
     * Extract the value at index 0 from the connected components vector
     */
    // float first_cc_value;
    // GrB_Vector_extractElement_FP32(&first_cc_value, cc, 0);
    // GxB_Scalar first_cc_scalar;
    // GxB_Scalar_new(&first_cc_scalar, GrB_FP32);
    // GxB_Scalar_setElement_FP32(first_cc_scalar, first_cc_value);

    /*
     * Select only the nodes that are in the first connected component
     */
    // GrB_Vector selection;
    // GrB_Vector_new(&selection, GrB_BOOL, num_nodes);
    // GxB_Vector_select(selection, NULL, NULL, GxB_EQ_THUNK, cc, first_cc_scalar, NULL);
    // pretty_print_vector_BOOL(selection, "Selection");
    // pretty_print_matrix_FP64(A, "Original Matrix");

    /*
     * Make diagonal matrix from selection vector
     */
    // GrB_Matrix D;
    // GrB_Matrix_new(&D, GrB_BOOL, num_nodes, num_nodes);
    // GrB_Matrix_diag(D, selection, 0, GrB_NULL);


    /***************************************************************************
     * BGO 2: Betweenness centrality
     **************************************************************************/
    // Decide source nodes
    int test = 1000;
    GrB_Index source[test];
    for (int i = 0; i < test; i++) {
        source[i] = rand() % num_nodes;
    }

    /*
     * Calculate betweenness centrality with LAGraph.
     */
    LAGraph_Graph G;
    LAGraph_New(&G, &A, LAGraph_ADJACENCY_UNDIRECTED, msg);

    GrB_Vector bc;
    LAGr_Betweenness(&bc, G, source, test, msg);

    /***************************************************************************
     * BGO 3: Find maximum betweenness centrality
     **************************************************************************/
    int max_bc_index = find_max(bc);
    printf("Max BC index: %d\n", max_bc_index);

    /***************************************************************************
     * BGO 4: BFS with max BC node as source
     **************************************************************************/
    GrB_Vector level_bfs_gb;
    GrB_Vector parent_bfs_gb;
    // LAGr_BreadthFirstSearch(&level_bfs_gb, &parent_bfs_gb, G, max_bc_index, msg);
    breadthFirstSearchGPU(&level_bfs_gb, &parent_bfs_gb, B, num_nodes, max_bc_index);
    

    /***************************************************************************
     * BGO 5: Find path from root to max BC node
     **************************************************************************/
    std::vector<int> path = find_path(parent_bfs_gb, 0, max_bc_index);

    /* Print path from root node to the most popular node.*/
    std::cout << "Path from 0 to max BC index: ";
    for (uint i = 0; i < path.size() - 1; i++) {
        std::cout << path[i] << " -> ";
    }
    std::cout << path[path.size() - 1] << std::endl;

    // Cleanup
    GrB_Matrix_free(&A);

    LAGraph_Finalize(msg);
    return GrB_SUCCESS;
}