#include <iostream>
#include <fstream>
#include <vector>
#include "GraphBLAS.h"
#include "LAGraph.h"
#include "../../bgo/find_path.hpp"
#include "../../include/utils.hpp"
#include "../../bgo/bfs.hpp"
#include <iomanip>

#define REPEAT_10(x) x x x x x x x x x x
#define REPEAT_100(x) REPEAT_10(REPEAT_10(x))
#define REPEAT_10000(x) REPEAT_100(REPEAT_100(x))

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
    std::vector<float> G_naive[num_nodes];
    GB_matrix_to_vector_array<float>(A, G_naive);

    LAGraph_Graph G;
    LAGraph_New(&G, &A, LAGraph_ADJACENCY_DIRECTED, msg);

    /*
     * Initialization of source nodes
     */
    const int n_iterations = 100;
    int sources[n_iterations * 2];
    for (int i = 0; i < n_iterations * 2; i++) {
        int random = rand();
        int temp = random % num_nodes;
        sources[i] = temp;
    }

    /*
     * Initialize output csv
     */
    std::string filename = argv[2];
    std::string stem = filename.substr(0, filename.size()-4);
    std::ofstream output_csv_GB;
    std::ofstream output_csv_naive;
    output_csv_GB.open(stem + "_GB.csv");
    output_csv_naive.open(stem + "_naive.csv");
    output_csv_GB << "iteration,source,dest,time" << std::endl;
    output_csv_naive << "iteration,source,dest,time" << std::endl;

    /* Disable rounding or truncating of floats for streams */
    output_csv_GB << std::fixed << std::setprecision(12);
    output_csv_naive << std::fixed << std::setprecision(12);

    /*
     * Do BFS to get parent array
     */
    GrB_Vector level_bfs_gb;
    GrB_Vector parent_bfs_gb;
    int *level_bfs = (int*)malloc(sizeof(int) * num_nodes);
    int *parent_bfs = (int*)malloc(sizeof(int) * num_nodes);
    std::vector<int> path;

    /*
     * Benchmarking bwc_dijkstra
     */
    for (int i = 0; i < n_iterations * 2; i+=2) {
        printf("Iteration %d\n", i / 2);

        LAGr_BreadthFirstSearch(&level_bfs_gb, &parent_bfs_gb, G, sources[i], msg);
        bfs(level_bfs, parent_bfs, G_naive, sources[i], num_nodes);

        start_time = clock();
        REPEAT_10000(find_path(parent_bfs_gb, sources[i + 1], sources[i]);)
        time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        output_csv_GB << i / 2 << "," << sources[i] << "," << sources[i + 1] << "," << time_elapsed << std::endl;

        start_time = clock();
        REPEAT_10000(find_path(parent_bfs, sources[i + 1], sources[i]);)
        time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        output_csv_naive << i / 2 << "," << sources[i] << "," << sources[i + 1] << "," << time_elapsed << std::endl;
    }

    return 0;
}