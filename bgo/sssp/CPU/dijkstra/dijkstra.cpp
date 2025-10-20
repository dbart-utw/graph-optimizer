#include <vector>
#include <queue>
#include <limits>
#include <stdio.h>
#include "dijkstra.hpp"

/*
 * Dijkstra's algorithm for single source shortest path
 */
void dijkstra(float *dist, int *parent, std::vector<float> *G, int src, int num_nodes) {
    /* Initialize dist and parent arrays. */
    for (int i = 0; i < num_nodes; ++i) {
        dist[i] = std::numeric_limits<float>::infinity();  // Infinity
        parent[i] = -1; // -1 represents no parent
    }

    /* Create a priority queue to store vertices that are being preprocessed. */
    std::priority_queue<std::pair<float, int>, std::vector <std::pair<float, int>> , std::greater<std::pair<float, int>>> pq;

    /* Insert source itself in priority queue and initialize its distance as 0. */
    pq.push(std::make_pair(0., src));
    dist[src] = 0.;

    /* Looping till priority queue becomes empty (or all distances are not finalized) */
    while (!pq.empty())
    {
        int u = pq.top().second;
        pq.pop();

        for (int v = 0; v < num_nodes; ++v)
        {
            /*
             * If there is a direct edge from u to v,
             * and if v is not finalized yet, and the new distance
             * through u is smaller than current value of dist[v].
             */
            if (G[u][v] > 0 && dist[u] != std::numeric_limits<int>::max() && dist[u]+G[u][v] < dist[v])
            {
                parent[v] = u;
                dist[v] = dist[u] + G[u][v];
                pq.push(std::make_pair(dist[v], v));
            }
        }
    }
}