#ifndef SAMPLING_HPP
#define SAMPLING_HPP

#include "gap/reader.h"
#include "gap/graph.h"
#include "gap/builder.h"
#include "gap/pvector.h"
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <map>

template<typename NodeID_>
void RandomVertexSampling(const CSRGraph<NodeID_> &G, float sample_rate, CSRGraph<NodeID_> &sampled_graph);
template<typename NodeID_>
void RandomEdgeSampling(const pvector<SGEdge> &el, float sample_rate, CSRGraph<NodeID_> &sampled_graph);
template<typename NodeID_>
void RandomEdgeFromDisk(const std::string &filename,  float sample_rate, CSRGraph<NodeID_> &sampled_graph);

template<typename NodeID_>
void FrontierSamplingVertex(const CSRGraph<NodeID_> &G, float max_sample_rate, float walker_percentage, CSRGraph<NodeID_> &sampled_graph);
template<typename NodeID_>
void FrontierSamplingEdge(const CSRGraph<NodeID_> &G, float max_sample_rate, float walker_percentage, CSRGraph<NodeID_> &sampled_graph);

template<typename NodeID_>
void ForestFireSamplingVertex(const CSRGraph<NodeID_> &G, float sample_rate, float p_forward, CSRGraph<NodeID_> &sampled_graph);
template<typename NodeID_>
void ForestFireSamplingEdge(const CSRGraph<NodeID_> &G, float sample_rate, float p_forward, CSRGraph<NodeID_> &sampled_graph);

#endif