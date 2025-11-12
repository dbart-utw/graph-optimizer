#include "vertex_push.hpp"

int bfs_vertex_push(CSR &g, CArray<int32_t> *levels) {
    levels->init(g.num_nodes());
    BFSGPU<Reduction<normal>>(g, levels->data, VERTEX_PUSH);
    
    return 0;
}
