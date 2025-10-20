
#include <vector>
#include <stack>
#include <queue>

#include "bc_brandes.hpp"

int bc_brandes(CMatrix<int> G, CArray<int> sources, CArray<int> *centrality) {
    const int num_verts = G.size_m;
    const int num_sources = sources.size;

    centrality->init(num_verts);

    /* Initialize the centrality vector with all zeros. */
    for (int i = 0; i < num_verts; i++) {
        centrality->data[i] = 0;
    }

    std::stack<int> S;
    std::queue<int> Q;

    for (int i = 0; i < num_sources; i++) {
        int s = sources.data[i];
        
        std::vector<std::vector<int>> P(num_verts);
        std::vector<int> sigma(num_verts, 0);
        std::vector<int> d(num_verts, -1);

        sigma[s] = 1;
        d[s] = 0;
        Q.push(s);

        while (!Q.empty()) {
            int v = Q.front();
            Q.pop();
            S.push(v);

            for (int w = 0; w < num_verts; w++) {
                if (G.data[v * num_verts + w] > 0) {
                    if (d[w] < 0) {
                        Q.push(w);
                        d[w] = d[v] + 1;
                    }

                    if (d[w] == d[v] + 1) {
                        sigma[w] = sigma[w] + sigma[v];
                        P[w].push_back(v);
                    }
                }
            }
        }

        std::vector<double> delta(num_verts, 0.0);
        while (!S.empty()) {
            int w = S.top();
            S.pop();
            for (int v : P[w]) {
                delta[v] += (sigma[v] / (double)sigma[w]) * (1 + delta[w]);
            }
            if (w != s) {
                centrality->data[w] += delta[w];
            }
        }
    }

    return 0;
}
