import os
import glob

from scipy.io import mminfo, mmread, mmwrite
import numpy as np

from util import random_sources

R = 10
P = 10
IN_GRAPHS = glob.glob('../data/random/random_*.mtx')
WORKDIR = 'data/simple'


print(f"Running experiment with {len(IN_GRAPHS)} input graphs, {P} instances and {R} runs per instance.")

os.makedirs(f'{WORKDIR}/runtime', exist_ok=True)
os.makedirs(f'{WORKDIR}/results', exist_ok=True)

graph = [fn for fn in IN_GRAPHS for _ in range(P)]

# Prepare the sources vector.
for i, filename in enumerate(graph):
    rows, cols, entries, _, _, _ = mminfo(filename)
    assert rows == cols
    mmwrite(f'{WORKDIR}/runtime/{i:03d}_sources.mtx', [random_sources(rows)])

# BETWEENNESS CENTRALITY (LAGRAPH)
sources = [f'{WORKDIR}/runtime/{i:03d}_sources.mtx' for i in range(len(graph))]
centrality = [f'{WORKDIR}/runtime/{i:03d}_centrality.mtx' for i in range(len(graph))]
os.system(f"python ../autobench/run_bench.py ../bgo/bc/bc_lagr --runs {R} --data G={','.join(graph)} sources={','.join(sources)} centrality={','.join(centrality)} --output {WORKDIR}/results/bc.csv")

# FIND MAXIMUM (GRAPHBLAS)
os.system(f"python ../autobench/run_bench.py ../bgo/find_max/find_max_gb --runs {R} --data values={','.join(centrality)} --output {WORKDIR}/results/find_max.csv")

# BREADTH FIRST SEARCH (LAGRAPH)
source = [str(np.argmax(mmread(cmtx))) for cmtx in centrality]
os.system(f"python ../autobench/run_bench.py ../bgo/bfs/bfs_lagr --runs {R} --data G={','.join(graph)} source={','.join(source)} --output {WORKDIR}/results/bfs.csv")

# COMBINED
os.system(f"python ../autobench/run_bench.py code/simple --runs {R} --data G={','.join(graph)} sources={','.join(sources)} --output {WORKDIR}/results/combined.csv")
