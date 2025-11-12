#include "rev_struct_edgelist.hpp"

int bfs_rev_struct_edgelist(EdgeStructList &esl, CArray<int32_t> *levels) {
    levels->init(esl.num_nodes);
    BFSGPU<Reduction<normal>>(esl, levels->data, REV_STRUCT_EDGELIST);

    return 0;
}