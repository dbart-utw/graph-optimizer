#include "struct_edgelist.hpp"

int bfs_struct_edgelist(EdgeStructList &esl, CArray<int32_t> *levels) {
    levels->init(esl.num_nodes);
    BFSGPU<Reduction<normal>>(esl, levels->data, STRUCT_EDGELIST);

    return 0;
}