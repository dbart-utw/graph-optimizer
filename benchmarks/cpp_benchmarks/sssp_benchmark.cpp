#include "GraphBLAS.h"
#include "LAGraph.h"
#include "../../include/utils.hpp"
#include "../../bgo/betweenness_centrality.hpp"
#include "../../bgo/find_max.hpp"
#include "../../bgo/delta_step.hpp"
#include "../../bgo/bfs.hpp"
#include "../../bgo/dijkstra.hpp"
#include <stdio.h>
#include <ctime>

char msg[LAGRAPH_MSG_LEN];
clock_t start_time;
double time_elapsed;

int main(int argc, char **argv) {
    srand(time(0));
    if (argc < 2)
    {
        fprintf(stderr, "Error\nUsage: %s <matrix_market_file.mtx>\n", argv[0]);
        return 1;
    }

    LAGraph_Init(msg);

    GrB_Matrix A;

    read_graph_GB(&A, argv[1]);

    GrB_Index num_nodes;
    GrB_Matrix_nrows(&num_nodes, A);

    // Get a single connected component using LAGraph
    // LAGraph_Graph G;
    // LAGraph_New(&G, &A, LAGraph_ADJACENCY_UNDIRECTED, msg);
    // GrB_Vector cc;
    // LAGr_ConnectedComponents(&cc, G, msg);
    // pretty_print_vector<int>(cc, "Connected Components");

    std::vector<float> G_naive[num_nodes];
    GB_matrix_to_vector_array(A, G_naive);



    /***************************************************************************
     * METHOD 1: LAGraph Delta-stepping (using GraphBLAS)
     **************************************************************************/
    // GrB_Vector distances;
    // GrB_Scalar delta;
    // GrB_Scalar_new(&delta, GrB_FP32);
    // GrB_Scalar_setElement_INT32(delta, 1);
    // start_time = clock();
    // LAGr_SingleSourceShortestPath(&distances, G, 0, delta, msg);
    // time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    // // pretty_print_vector<int>(distances, "distances LAGraph");
    // printf("Time elapsed LAGraph: %f\n", time_elapsed);

    /***************************************************************************
     * METHOD 2: Dijkstra's algorithm
     **************************************************************************/
    // float *distances_dijkstra = new float[num_nodes];
    // int *parent_dijkstra = new int[num_nodes];
    // start_time = clock();
    // dijkstra(distances_dijkstra, parent_dijkstra, G_naive, 0, num_nodes);
    // time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    // pretty_print_array(distances_dijkstra, num_nodes, "distances Dijkstra");
    // pretty_print_array(parent_dijkstra, num_nodes, "parent Dijkstra");
    // printf("Time elapsed Dijkstra: %f\n", time_elapsed);

    /***************************************************************************
     * METHOD 3: Traditional Delta-stepping
     **************************************************************************/
    // float *distances_naive = new float[num_nodes];
    // start_time = clock();
    // sssp_delta_step(distances_naive, G_naive, num_nodes, 0, 1.0);
    // time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    // // pretty_print_array(distances_naive, num_nodes, "distances naive delta-stepping");
    // printf("Time elapsed naive delta-stepping: %f\n", time_elapsed);

    /***************************************************************************
     * METHOD 4: Traditional BFS
     **************************************************************************/
    int *level_bfs_naive = new int[num_nodes];
    int *parent_bfs_naive = new int[num_nodes];
    start_time = clock();
    bfs(level_bfs_naive, parent_bfs_naive, G_naive, 0, num_nodes);
    time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    // pretty_print_array(level_bfs_naive, num_nodes, "level naive BFS");
    // pretty_print_array(parent_bfs_naive, num_nodes, "parent naive BFS");
    printf("Time elapsed naive BFS: %f\n", time_elapsed);

    /***************************************************************************
     * METHOD 5: LAGraph BFS
     **************************************************************************/
    // GrB_Vector level_bfs_gb;
    // GrB_Vector parent_bfs_gb;
    // start_time = clock();
    // LAGr_BreadthFirstSearch(&level_bfs_gb, &parent_bfs_gb, G, 0, msg);
    // time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    // pretty_print_vector<int>(level_bfs_gb, "level LAGraph");
    // pretty_print_vector<int>(parent_bfs_gb, "parent LAGraph");
    // printf("Time elapsed LAGraph BFS: %f\n", time_elapsed);

    // Cleanup
    GrB_Matrix_free(&A);
    LAGraph_Finalize(msg);

    return 0;
}