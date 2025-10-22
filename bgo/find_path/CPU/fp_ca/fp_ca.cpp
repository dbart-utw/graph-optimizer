#include "fp_ca.hpp"

int find_path(CArray<int> parent, int start, int end) {
    std::vector<int> path;

    int current = start;
    while (current != end) {
        // printf("start %d, end %d, current %d", start, end, current);
        path.push_back(current);
        current = parent.data[current];
    }
    path.push_back(end);

    return 0;
}
