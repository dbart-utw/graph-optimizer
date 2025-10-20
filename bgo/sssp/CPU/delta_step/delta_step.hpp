#ifndef DELTA_STEP_HPP
#define DELTA_STEP_HPP

#include <vector>

int sssp_delta_step(CArray<float> distances, std::vector<float> *G, int num_nodes, int source, float delta);

#endif