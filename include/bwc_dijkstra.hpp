
#include <iostream>
#include <limits.h>
#include <vector>
#include <string.h>
#include <queue>
#include "../include/BinaryHeap.hpp"
using namespace std;

struct vprop {
	int dist;
	int num_paths;
};

void bwc_dijkstra(vector<int> *G, int source, int N, vprop *props) {
	bool visited[N];
	vprop *temp_props = (vprop*)(malloc(sizeof(vprop) * N));
	BinaryHeap heap(N);
	for (int i = 0; i < N; i++) {
		temp_props[i] = {INT_MAX, 0};
		visited[i] = false;
	}

	temp_props[source] = {0, 1};
	visited[source] = true;
	heap.decrease_key(source, 0);

	while (!heap.is_empty()) {
		int i = heap.extract_min();
		visited[i] = true;
		for (int j = 0; j < N; j++) {
			if (!visited[j] && G[i][j] != 0) {
				if (temp_props[j].dist > temp_props[i].dist + G[i][j]) {
					temp_props[j] = {temp_props[i].dist + G[i][j], temp_props[i].num_paths};
					heap.decrease_key(j, temp_props[j].dist);
				} else if (temp_props[j].dist == temp_props[i].dist + G[i][j]) {
					temp_props[j].num_paths += temp_props[i].num_paths;
				}
			}
		}
	}

	memcpy(props, temp_props, sizeof(vprop) * N);
	free(temp_props);
}