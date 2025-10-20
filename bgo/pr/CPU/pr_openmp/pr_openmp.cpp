#include <vector>
#include <stack>
#include <queue>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <omp.h>
#include <math.h>
#include "pr_openmp.hpp"

/*
 * PageRank algorithm.
 * G: adjacency matrix of the graph.
 * PR: array to store the PageRank of each node.
 * float epsilon: convergence threshold.
 * Returns 0 if the algorithm executed successfully, -1 otherwise.
 */
int pagerank(CMatrix<int> G, CArray<float> *PR) {
    const float init_score = 1.0f / G.size_n;
    const float base_score = (1.0f - d) / G.size_n;
    float incoming_total;
    float old_score;

    PR->init(G.size_n);

    std::vector<float> outgoing_contrib(G.size_n, 0.0f);
    std::vector<float> out_degree(G.size_n, 0.0f);

    #pragma omp parallel for
    for (int i = 0; i < (int) G.size_n; i++) {
        /* Calculate the out-degree of each node. */
        for (int j = 0; j < (int) G.size_m; j++) {
            if (G.data[i * G.size_m + j] > 0) {
                out_degree[i]++;
            }
        }

        PR->data[i] = init_score;
        outgoing_contrib[i] = init_score / out_degree[i];
    }

    for (int iter = 0; iter < max_iter; iter++) {
        double error = 0;
        #pragma omp parallel for reduction(+ : error) schedule(dynamic)
        for (int u = 0; u < (int) G.size_n; u++) {
            incoming_total = 0;

            for (int v = 0; v < (int) G.size_m; v++) {
                if (G.data[u * G.size_m + v] > 0) {
                    incoming_total += outgoing_contrib[v];
                }
            }

            old_score = PR->data[u];
            PR->data[u] = base_score + d * incoming_total;
            error += std::fabs(PR->data[u] - old_score);
            outgoing_contrib[u] = PR->data[u] / out_degree[u];
        }
    }

    return 0;
}

