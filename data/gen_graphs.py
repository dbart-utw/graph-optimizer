# Generate star graphs and line graphs of a given number of nodes

import networkx as nx
from argparse import ArgumentParser
from scipy.io import mmwrite

def tree_node_count(r, h):
    return (r**(h+1)-1)//(r-1)

def generate_star_graph(n):
    G = nx.star_graph(n)
    return G

def generate_line_graph(n):
    G = nx.path_graph(n)
    return G

def generate_balanced_tree(n):
    r = 2

    # Find the closest power of r to n
    h = 1
    while tree_node_count(r, h) < n:
        h += 1

    G = nx.balanced_tree(r, h)
    # Remove nodes to get exactly n nodes
    generated_nodes = tree_node_count(r, h)
    to_remove = generated_nodes - n
    G.remove_nodes_from(range(generated_nodes - to_remove, generated_nodes))

    return G

def generate_circular_ladder(n):
    G = nx.circular_ladder_graph(n)
    return G

def generate_cycle_graph(n):
    G = nx.cycle_graph(n)
    return G

def generate_ladder_graph(n):
    G = nx.ladder_graph(n)
    return G

def generate_wheel_graph(n):
    G = nx.wheel_graph(n)
    return G

def write_graph_to_file(G, filename):
    M = nx.convert_matrix.to_scipy_sparse_matrix(G)
    with open(filename, 'wb') as file:
        mmwrite(file, M)

def gen_star_and_line(n: int) -> None:
    G = generate_star_graph(n)
    write_graph_to_file(G, f'star/{n}.mtx')
    G = generate_line_graph(n)
    write_graph_to_file(G, f'line/{n}.mtx')

if __name__=='__main__':
    parser = ArgumentParser(description='Generate star and line graphs')
    # Add argument n. If n is an int, use this. If n is a list, it should contain 3 items
    # The first item is the start, the second item is the end, and the third item is the step
    # In this case, generate star graphs of n nodes from start to end with step
    parser.add_argument('n', type=int, nargs="+", help='Number of nodes')
    args = parser.parse_args()
    n = args.n
    if len(n) == 1:
        gen_star_and_line(n[0])
    else:
        start, end, step = n
        for i in range(start, end, step):
            gen_star_and_line(i)


# Trivial:
# Balanced tree
# Circular ladder
# Cycle graph
# Ladder graph
# Wheel graph

