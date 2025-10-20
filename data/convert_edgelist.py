import numpy as np
import sys

edgelist_dict = {}

def convert_edge(line):
    global edgelist_dict
    u, v = line.split()[:2]
    if u not in edgelist_dict:
        edgelist_dict[u] = len(edgelist_dict) + 1
    if v not in edgelist_dict:
        edgelist_dict[v] = len(edgelist_dict) + 1
    return [edgelist_dict[u], edgelist_dict[v]]

def get_nm(edges):
    vertex_count = np.max(edges)
    edge_count = edges.shape[0]
    return f'{vertex_count} {vertex_count} {edge_count}\n'

def convert_edgelist(input_filename, output_filename):
    print(f'converting {input_filename} to {output_filename}')
    with open(input_filename, 'r') as f:
        lines = f.readlines()

    new_edgelist = np.array([convert_edge(line) for line in lines if line.strip() and not line.startswith('#')])
    print(new_edgelist.shape)

    with open(output_filename, 'w') as f:
        f.write('%%MatrixMarket coordinate integer general\n')
        f.write(get_nm(new_edgelist))
        for edge in new_edgelist:
            f.write(f'{edge[0]} {edge[1]} 1\n')


if __name__ == '__main__':
    convert_edgelist(sys.argv[1], sys.argv[2])