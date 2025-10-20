#include <stdio.h>
#include <omp.h>
#include <time.h>

#include "GraphBLAS.h"
#include "suitesparse/LAGraph.h"

#include "../../bgo/find_max/find_max_gb/find_max_gb.hpp"
#include "../../bgo/find_path/find_path.hpp"
#include "../../include/utils.hpp"

#include <cppJoules.h>

char msg[LAGRAPH_MSG_LEN] ;
clock_t start_time;
double time_elapsed;
int result = GrB_SUCCESS;

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Error\nUsage: %s <matrix_market_file.mtx> <num_runs>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));
    int num_runs = atoi(argv[2]);

    EnergyTracker et;
    LAGraph_Init(msg);

    GrB_Matrix A, B;

    read_graph_GB(&A, argv[1]);

    printf("Graph loaded from %s\n", argv[1]);

    GrB_Index num_nodes, num_edges;
    GrB_Matrix_nrows(&num_nodes, A);
    GrB_Matrix_nvals(&num_edges, A);
    printf("Number of nodes: %" PRIu64 "\n", num_nodes);
    printf("Number of edges: %" PRIu64 "\n", num_edges);

    GrB_Matrix_dup(&B, A);

    /* Convert graph to LAGraph. */
    LAGraph_Graph G;
    LAGraph_New(&G, &B, LAGraph_ADJACENCY_UNDIRECTED, msg);
    LAGraph_Cached_AT(G, msg);
    LAGraph_Cached_OutDegree(G, msg);

    for (int i = 0; i < num_runs; i++) {
        /* Select a source node at random. */
        int source = rand() % num_nodes;
        printf("Selected source node: %d\n", source);

        /***************************************************************************
         * BGO 1: PageRank
         **************************************************************************/
        GrB_Vector centrality;
        int iters = 0;
        float damping = 0.85f;
        float tol = 1e-4f;
        int itermax = 100;
        et.start();
        result = LAGr_PageRank(&centrality, &iters, G, damping, tol, itermax, msg);
        et.stop();
        if (result != GrB_SUCCESS)
        {
            fprintf(stderr, "Error in PageRank: %s\n", msg);
            LAGraph_Finalize(msg);
            return 1;
        }
        printf("PageRank completed in %d iterations.\n", iters);
        et.calculate_energy();
        et.print_energy();
        et.save_csv("temp/pr-" + std::to_string(source) + ".csv");

        /***************************************************************************
         * BGO 2: Find maximum centrality node
         **************************************************************************/
        GrB_Index max_centrality_index;
        et.start();
        find_max_index_GB(centrality, &max_centrality_index);
        et.stop();
        printf("Max centrality index: %" PRIu64 "\n", max_centrality_index);
        et.calculate_energy();
        et.print_energy();
        et.save_csv("temp/find_max-" + std::to_string(source) + ".csv");

        /***************************************************************************
         * BGO 3: BFS with max randomly selected source node as source
         **************************************************************************/
        GrB_Vector level_bfs_gb;
        GrB_Vector parent_bfs_gb;
        et.start();
        result = LAGr_BreadthFirstSearch(&level_bfs_gb, &parent_bfs_gb, G, source, msg);
        et.stop();
        if (result != GrB_SUCCESS)
        {
            fprintf(stderr, "Error in BFS: %s\n", msg);
            LAGraph_Finalize(msg);
            return 1;
        }
        printf("BFS completed.\n");
        et.calculate_energy();
        et.print_energy();
        et.save_csv("temp/bfs-" + std::to_string(source) + ".csv");

        /***************************************************************************
         * BGO 4: Find path from root to max BC node
         **************************************************************************/
        std::vector<int> path;
        et.start();
        path = find_path(parent_bfs_gb, source, max_centrality_index);
        et.stop();

        /* Print path from root node to the most popular node.*/
        if (path.empty() || path.back() == -1) {
            std::cout << "No path found from root to max centrality index." << std::endl;
        } else {
            std::cout << "Path from 0 to max BC index: ";
            for (uint i = 0; i < path.size() - 1; i++) {
                std::cout << path[i] << " -> ";
            }
            std::cout << path[path.size() - 1] << std::endl;
        }

        et.calculate_energy();
        et.print_energy();
        et.save_csv("temp/find_path-" + std::to_string(source) + ".csv");
    }

    // Cleanup
    GrB_Matrix_free(&A);
    LAGraph_Finalize(msg);

    return GrB_SUCCESS;
}