#include "GraphBLAS.h"
#include "LAGraph.h"
#include "../../include/utils.hpp"
#include "../../bgo/betweenness_centrality.hpp"
#include "../../bgo/find_max.hpp"
#include <stdio.h>
#include <ctime>
#include <fstream>

#define REPEAT_10(x) x x x x x x x x x x

char msg[LAGRAPH_MSG_LEN];
clock_t start_time;
double time_elapsed;

int main(int argc, char **argv) {
    srand(time(0));

    if (argc < 3)
    {
        fprintf(stderr, "Error\nUsage: %s <matrix_market_file.mtx>  <output_csv_folder> \n", argv[0]);
        return 1;
    }

    LAGraph_Init(msg);

    GrB_Matrix A;

    read_graph_GB(&A, argv[1]);

    GrB_Index num_nodes;
    GrB_Matrix_nrows(&num_nodes, A);
    std::vector<int> G_naive[num_nodes];
    GB_matrix_to_vector_array(A, G_naive);

    /*
     * Initialization of source nodes
     */
    int n_sources = 1000;
    GrB_Index sources_GrB[n_sources];
    int sources[n_sources];
    for (int i = 0; i < n_sources; i++) {
        int random = rand();
        int temp = random % num_nodes;
        sources_GrB[i] = temp;
        sources[i] = temp;
    }

    int n_iterations = 5;

    /*
     * Initialize output csv files for all methods
     */
    std::ofstream output_csv_graphBLAS;
    std::ofstream output_csv_LAGraph;
    std::ofstream output_csv_naive;
    std::ofstream output_csv_brandes;
    output_csv_graphBLAS.open(argv[2] + std::string("_graphblas.csv"));
    output_csv_LAGraph.open(argv[2] + std::string("_LAGraph.csv"));
    output_csv_naive.open(argv[2] + std::string("_naive.csv"));
    output_csv_brandes.open(argv[2] + std::string("_brandes.csv"));
    output_csv_graphBLAS << "iteration,n_sources,time" << std::endl;
    output_csv_LAGraph << "iteration,n_sources,time" << std::endl;
    output_csv_naive << "iteration,n_sources,time" << std::endl;
    output_csv_brandes << "iteration,n_sources,time" << std::endl;


    /***************************************************************************
     * METHOD 1: GraphBLAS
     **************************************************************************/
    GrB_Vector bc;
    for (int i = 0; i < n_iterations; i++) {
        start_time = clock();
        BC_GB(&bc, A, sources_GrB, n_sources);
        time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        output_csv_graphBLAS << i << "," << n_sources << "," << time_elapsed << std::endl;
        std::cout << "Time elapsed for iteration " << i << ": " << time_elapsed << std::endl;
    }


    /***************************************************************************
     * METHOD 2: LAGraph (also using GraphBLAS)
     **************************************************************************/
    LAGraph_Graph G;
    LAGraph_New(&G, &A, LAGraph_ADJACENCY_UNDIRECTED, msg);

    for(int i = 0; i < n_iterations; i++) {
        start_time = clock();
        LAGr_Betweenness(&bc, G, sources_GrB, n_sources, msg);
        time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        output_csv_LAGraph << i << "," << n_sources << "," << time_elapsed << std::endl;
        std::cout << "Time elapsed for iteration " << i << ": " << time_elapsed << std::endl;
    }


    /***************************************************************************
     * METHOD 3: Naive sequential
     **************************************************************************/
    float bc_naive[num_nodes];

    for (int i = 0; i < n_iterations; i++) {
        start_time = clock();
        BC_naive(bc_naive, G_naive, sources, n_sources, num_nodes);
        time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        output_csv_naive << i << "," << n_sources << "," << time_elapsed << std::endl;
        std::cout << "Time elapsed for iteration " << i << ": " << time_elapsed << std::endl;
    }


    /***************************************************************************
     * METHOD 4: Brandes' algorithm
     **************************************************************************/
    float bc_brandes[num_nodes];

    for (int i = 0; i < n_iterations; i++) {
        start_time = clock();
        BC_brandes(bc_brandes, G_naive, sources, n_sources, num_nodes);
        time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        output_csv_brandes << i << "," << n_sources << "," << time_elapsed << std::endl;
        std::cout << "Time elapsed for iteration " << i << ": " << time_elapsed << std::endl;
    }

    // Cleanup
    GrB_Matrix_free(&A);
    LAGraph_Finalize(msg);

    return 0;
}
