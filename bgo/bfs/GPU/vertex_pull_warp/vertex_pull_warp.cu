#include "vertex_pull_warp.hpp"

int bfs_vertex_pull(CSR &g, CArray<int32_t> *levels) {
    levels->init(g.num_nodes());
    BFSGPU<Reduction<normal>>(g, levels->data, VERTEX_PULL_WARP);
    
    return 0;
}
