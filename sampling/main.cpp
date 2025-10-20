#include "gap/reader.h"
#include "gap/builder.h"
#include "gap/graph.h"
#include "gap/timer.h"
#include "gap/writer.h"
#include <iostream>
#include <fstream>
#include "sampling.hpp"
#include <omp.h>
// #include "pagerank/pagerank.hpp"

void ReadGraph(const std::string &filename, CSRGraph<int32_t> &g, pvector<SGEdge> &el) {
    Reader<int32_t> r = Reader<int32_t>(filename);
    BuilderBase<int32_t> b = BuilderBase<int32_t>();
    bool needs_weights = false;
    el = r.ReadFile(needs_weights);
    g = b.MakeGraphFromEL(el);
}

/* Based on an input string (sampling_method), sample the graph accordingly, and return sampled graphs.
 * Options are: none (return original graph), random_vertex, random_edge. If not, print error message and return error code */
void SampleGraph(CSRGraph<int32_t> &g, pvector<SGEdge> &el, CSRGraph<int32_t> &sampled_graph, const std::string &sampling_method, float sampling_rate, float arg1) {
    if (sampling_rate < 0.0 || sampling_rate > 1.0) {
        std::cerr << "Error: Sampling rate must be between 0 and 1." << std::endl;
        exit(1);
    }

    if (sampling_method == "none" || sampling_rate == 1.0) {
        sampled_graph = std::move(g);
    } else if (sampling_method == "random_vertex") {
        RandomVertexSampling(g, sampling_rate, sampled_graph);
    } else if (sampling_method == "random_edge") {
        RandomEdgeSampling(el, sampling_rate, sampled_graph);
    } else if (sampling_method == "frontier_vertex") {
        FrontierSamplingVertex(g, sampling_rate, arg1, sampled_graph);
    } else if (sampling_method == "frontier_edge") {
        FrontierSamplingEdge(g, sampling_rate, arg1, sampled_graph);
    } else if (sampling_method == "forest_fire_vertex") {
        ForestFireSamplingVertex(g, sampling_rate, arg1, sampled_graph);
    } else if (sampling_method == "forest_fire_edge") {
        ForestFireSamplingEdge(g, sampling_rate, arg1, sampled_graph);
    } else {
        std::cerr << "Error: Unknown sampling method: " << sampling_method << std::endl;
        std::cerr << "Available kernels: none, random_vertex, random_edge" << std::endl;
        exit(1);
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <graph_filename> <sampling_rate> <output_filename>" << std::endl;
        return 1;
    }

    omp_set_num_threads(omp_get_max_threads());

    std::string filename = argv[1];
    float sampling_rate = std::stof(argv[2]);
    std::string output_filename = argv[3];

    CSRGraph<int32_t> sampled_graph;
    RandomEdgeFromDisk(filename, sampling_rate, sampled_graph);
    
    // write graph to temporary file: "sampled.mtx"
    WriterBase w = WriterBase(sampled_graph);
    std::fstream file;
    file.open(output_filename, std::fstream::out);
    w.WriteMTX(file);
    file.close();
    

    return 0;
}
