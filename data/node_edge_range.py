# Print the range of nodes and edges in the graphs in a specific directory

import os
import sys

directory = sys.argv[1]

min_nodes = min_edges = float('inf')
max_nodes = max_edges = 0

for file in os.listdir(directory):
    if file.endswith('.mtx'):
        with open(os.path.join(directory, file), 'r') as f:
            next(f)
            line = next(f)
            print(file)
            print(line)
            nodes, _, edges = map(int, line.split())
            min_nodes = min(min_nodes, nodes)
            max_nodes = max(max_nodes, nodes)
            min_edges = min(min_edges, edges)
            max_edges = max(max_edges, edges)

print(f"Nodes: {min_nodes} - {max_nodes}")
print(f"Edges: {min_edges} - {max_edges}")