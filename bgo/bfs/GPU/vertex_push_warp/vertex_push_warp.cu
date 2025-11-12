#include "vertex_push_warp.hpp"

int bfs_vertex_push_warp(CSR &g, CArray<int32_t> *levels) {
    levels->init(g.num_nodes());
    BFSGPU<Reduction<normal>>(g, levels->data, VERTEX_PUSH_WARP);
    
    return 0;
}
