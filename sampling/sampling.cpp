#include "sampling.hpp"
#include "gap/timer.h"
#include <algorithm>
#include <random>
#include <unordered_set>
#include <queue>
#include "gap/reader.h"
#include "gap/builder.h"
#include "omp.h"

// Define a hash function for SGEdge
struct EdgeHash {
    size_t operator()(const SGEdge &e) const {
        // Combine the hash of the two nodes (u and v) to create a unique hash for the edge
        return std::hash<int>()(e.u) ^ (std::hash<int>()(e.v) << 1);
    }
};

// Define equality for SGEdge
struct EdgeEqual {
    bool operator()(const SGEdge &e1, const SGEdge &e2) const {
        // Check if two edges are the same (ignoring direction)
        return (e1.u == e2.u && e1.v == e2.v) || (e1.u == e2.v && e1.v == e2.u);
    }
};

template<typename NodeID_>
void RandomVertexSampling(const CSRGraph<NodeID_> &G, float sample_rate, CSRGraph<NodeID_> &sampled_graph) {
    int32_t new_num_nodes = G.num_nodes() * sample_rate;

    /* Create a list of all vertices and shuffle it */
    std::vector<NodeID_> all_vertices(G.num_nodes());
    std::iota(all_vertices.begin(), all_vertices.end(), 0); // Fill with 0, 1, ..., num_nodes - 1
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle(all_vertices.begin(), all_vertices.end(), rng);

    /* Select the first new_num_nodes vertices */
    std::vector<NodeID_> vertices(all_vertices.begin(), all_vertices.begin() + new_num_nodes);

    /* Create a bitmap for fast lookup */
    std::vector<bool> is_in_sample(G.num_nodes(), false);
    for (NodeID_ v : vertices) {
        is_in_sample[v] = true;
    }

    /* Initialize data structures for the sampled graph */
    NodeID_ out_indices[new_num_nodes + 1];
    NodeID_ in_indices[new_num_nodes + 1];
    std::vector<NodeID_> out_neighs;
    std::vector<NodeID_> in_neighs;

    int32_t index = 0;
    int out_neighs_length = 0;
    int in_neighs_length = 0;

    for (NodeID_ v : vertices) {
        out_neighs_length = out_neighs.size();
        in_neighs_length = in_neighs.size();

        for (NodeID_ u : G.out_neigh(v)) {
            if (is_in_sample[u]) { // Use bitmap for fast lookup
                out_neighs.push_back(u);
            }
        }
        for (NodeID_ u : G.in_neigh(v)) {
            if (is_in_sample[u]) { // Use bitmap for fast lookup
                in_neighs.push_back(u);
            }
        }

        out_indices[index] = out_neighs_length;
        in_indices[index++] = in_neighs_length;
    }
    out_indices[index] = out_neighs_length;
    in_indices[index] = in_neighs_length;

    /* Allocate memory for the sampled graph */
    NodeID_** out_pointers = new NodeID_*[new_num_nodes + 1];
    NodeID_** in_pointers = new NodeID_*[new_num_nodes + 1];
    NodeID_* out_neighs_array = new NodeID_[out_neighs.size()];
    NodeID_* in_neighs_array = new NodeID_[in_neighs.size()];

    /* Copy the neighbors to the new arrays */
    #pragma omp parallel for
    for (int i = 0; i < out_neighs.size(); i++) {
        out_neighs_array[i] = out_neighs[i];
    }
    #pragma omp parallel for
    for (int i = 0; i < in_neighs.size(); i++) {
        in_neighs_array[i] = in_neighs[i];
    }

    /* Create the pointers to the new arrays */
    #pragma omp parallel for
    for (int i = 0; i < new_num_nodes + 1; i++) {
        out_pointers[i] = &out_neighs_array[out_indices[i]];
        in_pointers[i] = &in_neighs_array[in_indices[i]];
    }

    /* Build the sampled graph */
    sampled_graph = CSRGraph<NodeID_>(new_num_nodes, out_pointers, out_neighs_array, in_pointers, in_neighs_array);

    return;
}

template<typename NodeID_>
pvector<SGEdge> relabelEdges(pvector<SGEdge> &old_edges) {
    pvector<SGEdge> new_edges = pvector<SGEdge>();
    std::map<NodeID_, NodeID_> vertex_map;
    NodeID_ vertex_index = 0;

    for (const SGEdge &e : old_edges) {
        if (vertex_map.find(e.u) == vertex_map.end()) {
            vertex_map[e.u] = vertex_index++;
        }
        if (vertex_map.find(e.v) == vertex_map.end()) {
            vertex_map[e.v] = vertex_index++;
        }
        new_edges.push_back(SGEdge(vertex_map[e.u], vertex_map[e.v]));
    }

    return new_edges;
}

template<typename NodeID_>
void RandomEdgeSampling(const pvector<SGEdge> &el, float sample_rate, CSRGraph<NodeID_> &sampled_graph) {
    int64_t new_num_edges = el.size() * sample_rate;
    pvector<SGEdge> sampled_edges = pvector<SGEdge>(new_num_edges);
    NodeID_ vertex_index = 0;

    /* Shuffle the edge list */
    std::vector<SGEdge> shuffled_edges(el.begin(), el.end());
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle(shuffled_edges.begin(), shuffled_edges.end(), rng);

    /* Select the first new_num_edges edges */
    for (int i = 0; i < new_num_edges; i++) {
        sampled_edges[i] = shuffled_edges[i];
    }

    pvector<SGEdge> new_edges = relabelEdges<int32_t>(sampled_edges);

    /* Build the new graph */
    BuilderBase<NodeID_> b = BuilderBase<NodeID_>();
    sampled_graph = b.MakeGraphFromEL(new_edges);

    return;
}

template<typename NodeID_>
void RandomEdgeFromDisk(const std::string &filename, float sample_rate, CSRGraph<NodeID_> &sampled_graph) {
    Reader<NodeID_> r = Reader<NodeID_>(filename);
    BuilderBase<NodeID_> b = BuilderBase<NodeID_>();
    bool needs_weights = false;
    std::ifstream file(filename);
    pvector<SGEdge> el = r.ReadInMTX(file, needs_weights, sample_rate);
    pvector<SGEdge> relabeled_edges = relabelEdges<int32_t>(el);
    sampled_graph = b.MakeGraphFromEL(relabeled_edges);
    file.close();
    return;
}


// Walker percentage experiment with 0.001%, 0.01%, 0.1%, 1%., but also percentage of sample rate.
// Alternative: Only consider graphs with a single connected component.
template<typename NodeID_>
void FrontierSampling(const CSRGraph<NodeID_> &G, float max_sample_rate, float walker_percentage, CSRGraph<NodeID_> &sampled_graph, bool edge_sampling=false, int max_walkers=100, int min_walkers=1) {
    int32_t threshold = edge_sampling ? G.num_edges() * max_sample_rate : G.num_nodes() * max_sample_rate;
    int32_t num_walkers = std::max(std::min(static_cast<int32_t>(walker_percentage * G.num_edges()), max_walkers), min_walkers);
    const int32_t cost = 1;
    std::random_device rd;
    std::mt19937 gen(rd());
    int32_t *degrees = new int32_t[num_walkers];
    Timer t;

    /* Select <num_walkers> walkers uniformly from the graph. */
    NodeID_ *walkers = new NodeID_[num_walkers];
    for (int32_t i = 0; i < num_walkers; i++) {
        walkers[i] = rand() % G.num_nodes();
    }

    // Use a hash set to track unique edges
    std::unordered_set<SGEdge, EdgeHash, EdgeEqual> edge_set;
    pvector<SGEdge> new_edges = pvector<SGEdge>();
    std::unordered_set<NodeID_> new_verts = std::unordered_set<NodeID_>(walkers, walkers + num_walkers);

    int consecutive_no_additions = 0; // Counter for consecutive iterations with no new edges

    while ((edge_sampling && (new_edges.size() < threshold)) || (!edge_sampling && (new_verts.size() < threshold))) {
        /* Check if all walkers have 0 neighbors */
        for (int32_t j = 0; j < num_walkers; j++) {
            degrees[j] = G.out_degree(walkers[j]);
        }

        /* Select a random walker with probability proportional to its degree. */
        std::discrete_distribution<> dist(degrees, degrees + num_walkers);
        int32_t selected_idx = dist(gen);
        NodeID_ selected_walker = walkers[selected_idx];

        if (degrees[selected_idx] == 0) {
            std::cout << "All walkers have 0 neighbors. Exiting." << std::endl;
            break;
        }

        /* Uniformly select outgoing edges from the selected walker. */
        int32_t selected_neighbor_idx = rand() % degrees[selected_idx];
        NodeID_ selected_neighbor = *(G.out_index()[selected_walker] + selected_neighbor_idx);

        /* Create the edge */
        SGEdge edge(selected_walker, selected_neighbor);

        /* Add edge to new_edges only if it is not already in the set */
        if (edge_set.find(edge) == edge_set.end()) {
            edge_set.insert(edge);
            new_edges.push_back(edge);
            consecutive_no_additions = 0;
        } else {
            consecutive_no_additions++;
        }

        /* Check if we are stuck in a loop */
        if (consecutive_no_additions > 100) {
            std::cout << "Stuck in a loop where all edges have already been added. Exiting." << std::endl;
            break;
        }

        /* Replace selected walker with selected neighbor, and add neighbor to the new_verts */
        walkers[selected_idx] = selected_neighbor;
        new_verts.insert(selected_neighbor);
    }

    pvector<SGEdge> relabeled_edges = relabelEdges<int32_t>(new_edges);

    /* Build the new graph */
    BuilderBase<NodeID_> b = BuilderBase<NodeID_>();
    sampled_graph = b.MakeGraphFromEL(relabeled_edges);

    delete[] degrees;
    delete[] walkers;

    return;
}

template<typename NodeID_>
void FrontierSamplingVertex(const CSRGraph<NodeID_> &G, float sample_rate, float walker_percentage, CSRGraph<NodeID_> &sampled_graph) {
    FrontierSampling(G, sample_rate, walker_percentage, sampled_graph, false);
}

template<typename NodeID_>
void FrontierSamplingEdge(const CSRGraph<NodeID_> &G, float sample_rate, float walker_percentage, CSRGraph<NodeID_> &sampled_graph) {
    FrontierSampling(G, sample_rate, walker_percentage, sampled_graph, true);
}

template<typename NodeID_>
void ForestFireSampling(const CSRGraph<NodeID_> &G, float sample_rate, CSRGraph<NodeID_> &sampled_graph, float p_forward, bool edge_sampling=false) {
    int32_t threshold = edge_sampling ? G.num_edges() * sample_rate : G.num_nodes() * sample_rate;

    std::unordered_set<NodeID_> sampled_nodes;
    std::unordered_set<SGEdge, EdgeHash, EdgeEqual> sampled_edges;

    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::queue<NodeID_> node_queue;
    NodeID_ current_node;

    while ((edge_sampling && (sampled_edges.size() < threshold)) || (!edge_sampling && (sampled_nodes.size() < threshold))) {
        /* If queue is empty, randomly select a start node. */
        while (node_queue.empty()) {
            current_node = rand() % G.num_nodes();
            /* Check if the node has been visited before. */
            auto result = sampled_nodes.insert(current_node);

            /* If the node has already been visited, continue to the next iteration. */
            if (result.second) {
                node_queue.push(current_node);
            }
        }

        /* Select the next node from the queue. */
        current_node = node_queue.front();
        node_queue.pop();

        for (NodeID_ neighbor : G.out_neigh(current_node)) {
            /* If the neighbor is already sampled, skip it. */
            if (sampled_nodes.find(neighbor) == sampled_nodes.end()) {
                node_queue.push(neighbor);
            }

            /* Randomly select the edge with probability p_forward. */
            if (dist(rng) < p_forward) {
                /* Add node and edge to the sets of sampled nodes and edges. */
                sampled_nodes.insert(neighbor);
                sampled_edges.insert(SGEdge(current_node, neighbor));
            }
        }
    }

    /* Convert edges to pvector */
    pvector<SGEdge> new_edges = pvector<SGEdge>(sampled_edges.size());
    int index = 0;
    for (const SGEdge &e : sampled_edges) {
        new_edges[index++] = e;
    }

    /* Relabel edges */
    pvector<SGEdge> relabeled_edges = relabelEdges<int32_t>(new_edges);

    /* Build the new graph */
    BuilderBase<NodeID_> b = BuilderBase<NodeID_>();
    sampled_graph = b.MakeGraphFromEL(relabeled_edges);

    return;
}

template<typename NodeID_>
void ForestFireSamplingVertex(const CSRGraph<NodeID_> &G, float sample_rate, float p_forward, CSRGraph<NodeID_> &sampled_graph) {
    ForestFireSampling(G, sample_rate, sampled_graph, p_forward, false);
}

template<typename NodeID_>
void ForestFireSamplingEdge(const CSRGraph<NodeID_> &G, float sample_rate, float p_forward, CSRGraph<NodeID_> &sampled_graph) {
    ForestFireSampling(G, sample_rate, sampled_graph, p_forward, true);
}

template void RandomVertexSampling<int32_t>(const CSRGraph<int32_t>&, float, CSRGraph<int32_t>&);
template void RandomEdgeSampling<int32_t>(const pvector<SGEdge>&, float, CSRGraph<int32_t>&);
template void RandomEdgeFromDisk<int32_t>(const std::string&, float, CSRGraph<int32_t>&);
template void FrontierSamplingVertex<int32_t>(const CSRGraph<int32_t>&, float, float, CSRGraph<int32_t>&);
template void FrontierSamplingEdge<int32_t>(const CSRGraph<int32_t>&, float, float, CSRGraph<int32_t>&);
template void ForestFireSamplingVertex<int32_t>(const CSRGraph<int32_t>&, float, float, CSRGraph<int32_t>&);
template void ForestFireSamplingEdge<int32_t>(const CSRGraph<int32_t>&, float, float, CSRGraph<int32_t>&);