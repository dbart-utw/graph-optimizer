#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <vector>

/*
 * Dijkstra's algorithm for single source shortest path
 */
void dijkstra(float *dist, int *parent, std::vector<float> *G, int src, int num_nodes);

#endif