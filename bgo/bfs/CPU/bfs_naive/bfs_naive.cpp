#include <queue>

#include "datastructures.hpp"

#include "bfs_naive.hpp"

int bfs_naive(CMatrix<int> G, int source, CArray<int> *level, CArray<int> *parent) {
    const int num_nodes = G.size_m;

    // Initialize level and parent arrays
    level->init(num_nodes);
    parent->init(num_nodes);
    for (int i = 0; i < num_nodes; ++i) {
        level->data[i] = -1;  // -1 represents an unreachable node
        parent->data[i] = -1; // -1 represents no parent
    }

    // Create a queue for BFS
    std::queue<int> queue;

    // Mark the source node as visited and enqueue it
    level->data[source] = 0;
    queue.push(source);

    while (!queue.empty()) {
        // Dequeue a vertex from queue and print it
        int u = queue.front();
        queue.pop();

        // Get all adjacent vertices of the dequeued vertex u
        // If an adjacent vertex has not been visited, then mark it visited and enqueue it
        for (int v = 0; v < num_nodes; ++v) {
            if (G.data[u * num_nodes + v] > 0 && level->data[v] == -1) {
                parent->data[v] = u;
                level->data[v] = level->data[u] + 1;
                queue.push(v);
            }
        }
    }

    return 0;
}
