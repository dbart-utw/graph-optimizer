#include "GraphBLAS.h"
#include <vector>
#include <set>
#include <algorithm>
#include <limits>
#include <iostream>
#include "delta_step.hpp"

/*
 * Check if all sets in B are empty
 */
bool isEmpty(std::vector<std::set<int>> &B) {
    for (std::set<int> s : B) {
        /* If at least one set is not empty, we return false. */
        if (!s.empty()) {
            return false;
        }
    }

    return true;
}

std::set<int> B_init(int i, float delta, float *tent, int num_nodes) {
    std::set<int> Bi;
    for (int v = 0; v < num_nodes; v++) {
        /* B[i] = {v ∈ V | iΔ ≤ tent(v) < (i+1) Δ}*/
        if ((float)i * delta <= tent[v] && tent[v] < (float)(i + 1) * delta) {
            Bi.insert(v);
        }
    }

    return Bi;
}

std::set<std::pair<int, float>> Req_init(std::set<int> set, float *tent, std::vector<float> *G, std::set<std::pair<int, int>> edges) {
    std::set<std::pair<int, float>> Req;

    for (int v : set) {
        for (std::pair<int, int> edge : edges) {
            if (edge.first == v) {
                std::pair<int, float> pair(edge.second, tent[v] + G[v][edge.second]);
                Req.insert(pair);
            }
        }
    }

    return Req;
}

void relax(int v, float new_dist, float *tent, std::vector<std::set<int>> &B, float delta) {
    if (new_dist < tent[v]) {
        if (tent[v] != std::numeric_limits<float>::infinity()) {
            B[(int) std::floor(tent[v] / delta)].erase(v);
        }
        B[(int) std::floor(new_dist / delta)].insert(v);
        tent[v] = new_dist;
    }
}

int sssp_delta_step(float *distances, std::vector<float> *G, int num_nodes, int source, float delta) {
    std::set<std::pair<int, int>> heavy;
    std::set<std::pair<int, int>> light;

    /*
     * Populate heavy and light sets with correpsonding vertices.
     */
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
            std::pair<int, int> pair(i, j);
            if (G[i][j] > delta) {
                heavy.insert(pair);
            } else if (G[i][j] != 0) {
                light.insert(pair);
            }
        }
    }

    float tent[num_nodes];
    std::fill_n(tent, num_nodes, std::numeric_limits<float>::infinity());


    std::vector<std::set<int>> B;
    B.push_back(std::set<int>());

    relax(source, 0, tent, B, delta);


    std::set<std::pair<int, float>> Req;
    for (int i = 0; !isEmpty(B); i++) {
        B.push_back(std::set<int>());
        std::set<int> S;

        std::set<int> Bi = B_init(i, delta, tent, num_nodes);
        B[i] = Bi;
        while (!B[i].empty()) {
            Req = Req_init(B[i], tent, G, light);
            S.insert(B[i].begin(), B[i].end());
            B[i].clear();
            for (std::pair<int, float> pair : Req) {
                relax(pair.first, pair.second, tent, B, delta);
            }
        }

        Req = Req_init(B[i], tent, G, heavy);
        for (std::pair<int, int> pair : Req) {
            S.insert(pair.first);
        }
    }

    for (int i = 0; i < num_nodes; i++) {
        distances[i] = tent[i];
    }

    return 0;
}