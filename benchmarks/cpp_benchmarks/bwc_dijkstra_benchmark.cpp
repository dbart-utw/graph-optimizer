#include <iostream>
#include <fstream>
#include "GraphBLAS.h"
#include "LAGraph.h"
#include "../../include/bwc_dijkstra.hpp"
#include "../../include/utils.hpp"

#define REPEAT_10(x) x x x x x x x x x x

char msg[LAGRAPH_MSG_LEN];
clock_t start_time;
double time_elapsed;

int main(int argc, char **argv) {
    srand(time(0));

    if (argc < 3)
    {
        fprintf(stderr, "Error\nUsage: %s <matrix_market_file.mtx> <output_csv>\n", argv[0]);
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
    int n_sources = 100;
    int sources[n_sources];
    for (int i = 0; i < n_sources; i++) {
        int random = rand();
        int temp = random % num_nodes;
        sources[i] = temp;
    }

    int n_iterations = 5;

    /*
     * Initialize output csv
     */
    std::ofstream output_csv;
    output_csv.open(argv[2]);
    output_csv << "iteration,source,time" << std::endl;

    /*
     * Benchmarking bwc_dijkstra
     */
    vprop *props = (vprop*)(malloc(sizeof(vprop) * num_nodes));
    for (int i = 0; i < n_iterations; i++) {
        for (int j = 0; j < n_sources; j++) {
            start_time = clock();
            REPEAT_10(bwc_dijkstra(G_naive, sources[j], num_nodes, props);)
            time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            output_csv << i << "," << sources[j] << "," << time_elapsed << std::endl;
            std::cout << "Time elapsed for iteration " << i << " source " << sources[j] << ": " << time_elapsed << std::endl;
        }
    }

    /* "Using" props */
    std::cout << "Number of paths for node 0: " << props[0].num_paths << std::endl;

    return 0;
}