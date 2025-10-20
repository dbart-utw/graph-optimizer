#include <vector>
#include <string.h>
#include <queue>

#include "BinaryHeap.hpp"

#include "bc_naive.hpp"

struct vprop {
    int dist;
    int num_paths;
};

void bwc_dijkstra(CMatrix<int> G, int source, vprop *props) {
    const int N = G.size_m;

    bool visited[N];
    vprop *temp_props = (vprop*)(malloc(sizeof(vprop) * N));
    BinaryHeap heap(N);
    for (int i = 0; i < N; i++) {
        temp_props[i].dist = INT_MAX;
        temp_props[i].num_paths = 0;
        visited[i] = false;
    }
    temp_props[source].dist = 0;
    temp_props[source].num_paths = 1;
    visited[source] = true;
    heap.decrease_key(source, 0);

    while (!heap.is_empty()) {
        int i = heap.extract_min();
        visited[i] = true;
        for (int j = 0; j < N; j++) {
            if (!visited[j] && G.data[i * N + j] != 0) {
                if (temp_props[j].dist > temp_props[i].dist + G.data[i * N + j]) {
                    temp_props[j].dist = temp_props[i].dist + G.data[i * N + j];
                    temp_props[j].num_paths = temp_props[i].num_paths;
                    heap.decrease_key(j, temp_props[j].dist);
                } else if (temp_props[j].dist == temp_props[i].dist + G.data[i * N + j]) {
                    temp_props[j].num_paths += temp_props[i].num_paths;
                }
            }
        }
    }

    memcpy(props, temp_props, sizeof(vprop) * N);
    free(temp_props);
}

int bc_naive(CMatrix<int> G, CArray<int> sources, CArray<int> *centrality) {
    const int num_verts = G.size_m;
    const int num_sources = sources.size;

    centrality->init(num_verts);
    
    /* Initialize the BC vector with all zeros. */
    for (size_t i = 0; i < centrality->size; i++) {
        centrality->data[i] = 0;
    }

    vprop *assp[num_sources];
    for (size_t i = 0; i < sources.size; i++) {
        assp[i] = (vprop*)malloc(sizeof(vprop) * G.size_m);
    }

    for (size_t i = 0; i < sources.size; i++) {
        bwc_dijkstra(G, sources.data[i], assp[i]);
    }

    vprop *ds;
    vprop *dt;
    for (int s = 0; s < num_sources; s++) {
        for (int t = 0; t < num_sources; t++) {
            if (sources.data[t] != sources.data[s]) {
                ds = assp[s];
                dt = assp[t];
                for (int v = 0; v < num_verts; v++) {
                    if (v != sources.data[s] && v != sources.data[t] && ds[v].dist + dt[v].dist == ds[sources.data[t]].dist && ds[sources.data[t]].num_paths > 0) {
                        centrality->data[v] += (ds[v].num_paths * dt[v].num_paths) / (float)ds[sources.data[t]].num_paths;
                    }
                }
            }
        }
    }

    return 0;
}
