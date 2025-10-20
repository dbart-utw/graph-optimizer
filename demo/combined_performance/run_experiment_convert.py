import os
import glob
import random

from scipy.io import mminfo, mmread, mmwrite
import numpy as np

from util import random_sources

R = 10
P = 10
IN_GRAPHS = glob.glob('../data/random/random_*.mtx')
WORKDIR = 'data/convert'


print(f"Running experiment with {len(IN_GRAPHS)} input graphs, {P} instances and {R} runs per instance.")

os.makedirs(f'{WORKDIR}/runtime', exist_ok=True)
os.makedirs(f'{WORKDIR}/results', exist_ok=True)

graph = [fn for fn in IN_GRAPHS for _ in range(P)]

# Prepare the sources vector.
for i, filename in enumerate(graph):
    rows, cols, entries, _, _, _ = mminfo(filename)
    assert rows == cols
    mmwrite(f'{WORKDIR}/runtime/{i:03d}_sources.mtx', [random_sources(rows)])
sources = [f'{WORKDIR}/runtime/{i:03d}_sources.mtx' for i in range(len(graph))]

# CONVERSION GB -> CM
os.system(f"python ../autobench/run_bench.py ../bgo/convert/convert --runs {R} --data G={','.join(graph)} --output {WORKDIR}/results/convert.csv")

# BREADTH FIRST SEARCH (NAIVE)
centrality = [f'data/simple/runtime/{i:03d}_centrality.mtx' for i in range(len(graph))]
source = [str(np.argmax(mmread(cmtx))) for cmtx in centrality]
os.system(f"python ../autobench/run_bench.py ../bgo/bfs/bfs_naive --runs {R} --data G={','.join(graph)} source={','.join(source)} --output {WORKDIR}/results/bfs.csv")

# COMBINED
os.system(f"python ../autobench/run_bench.py code/convert --runs {R} --data G={','.join(graph)} sources={','.join(sources)} --output {WORKDIR}/results/combined.csv")
