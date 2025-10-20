# Graph Optimizer Autobench

Autobench can be used to automatically generate benchmarking code and gather performance results from BGO implementations.

The autobench infrastructure expects that BGO's (in the `bgo` directory) are implemented in a two layer structure. At the highest level, an abstract description of the BGO's is formulated in the `config.json` files. Each subdirectory then contains an actual implementation of the BGO.

The benchmarking code can ben generated based on a bgo header file with a single function definition. The `config.json` file, in the parent directory, is used to generate cases to benchmark.

## Usage Instructions

### Add a new abstract BGO
First, create a new directory in the `bgo` directory. Preferably named after the BGO. Then, create a new file named `config.json` in the new directory. The contents of this file must describe the arguments of the BGO on an abstract level. See the example below:
```JSON
{
    "in_args": [
        {
            "id": "G",
            "kind": "GRAPH"
        },
        {
            "id": "sources",
            "kind": "VECTOR",
            "value": "G.RAND_VERT_VECTOR"
        }
    ],
    "out_args": [
        {
            "id": "centrality",
            "kind": "VECTOR"
        }
    ]
}
```
The arguments must be supplied in-order. Output arguments always come after the input arguments. For a list of accepted argument kinds and values, see the appendix below.

### Add a new implemented BGO
Based on the abstract description of a BGO, you can add multiple implementations. Each implementation is contained a seperate subdirectory. This subdirectory must contain a Makefile with the option `bench` and a `.hpp` file with the same name as the subdirectory. The `.hpp` file must contain a single function definition that alligns with the abstract BGO description. See the example below:
```C++
int bc_lagr(LAGraph_Graph G, CArray<GrB_Index> sources, GrB_Vector *centrality);
```
Check the appendix below for the accepted argument types.

A generic makefile can be found at `autobench/misc/Makefile_generic`. You can create a symbolic link to this file using the following command:
```sh
ln -s ../../../autobench/misc/Makefile_generic Makefile
```
There is also a generic makefile for CUDA code at `autobench/misc/Makefile_cuda`.

You are free to create your own custom Makefile and template for the benchmark code.

### Configure & run the benchmark
You can run a benchmark using `python autobench/run_bench.py <bgo paths...>`. You may also use one of the options like:
- `--num N` : to specify the number of randomly generated inputs per graph/BGO pair.
- `--runs N` : to specify the number of runs to perform with the same input.
- `--data key1=value11 key2=value21,value22` : to pin some of the input data and optionally set output files. Always include atleast one value for `G`.

Example:
```sh
python autobench/run_bench.py bgo/bc/bc_lagr --num 1 --runs 4 --data G=data/RO_edges.mtx
```

### Appendix: Example slurm job
In case you are running the benchmark on a cluster with SLURM installed you may want to create a SLURM job.

Example `autobench.job` file:
```sh
#!/bin/sh
#SBATCH --time=00:15:00
#SBATCH -N 1

. env/bin/activate

python autobench/run_bench.py bgo/bc/bc_lagr --num 1 --runs 4 --data G=data/RO_edges.mtx
```
You can run the job using `sbatch autobench.job`.


### Appendix: Known argument types

- `GRAPH`
    - Implementations:
        - `CMatrix<int>`
        - `GPU_CMatrix<int>`
        - `GrB_Matrix`
        - `LAGraph_Graph`
        - `BCGraph`
    - Statistics:
        - `GRAPH.SIZE_VERTS`
        - `GRAPH.SIZE_EDGES`
- `VECTOR`
    - Implementations:
        - `CArray<int>`
        - `CArray<GrB_Index>`
        - `GPU_CArray<int>`
        - `GrB_Vector`
    - Values
        - `GRAPH.RAND_VERT_VECTOR`
    - Statistics:
        - `VECTOR.SIZE`
- `VERTEX`
    - Implementations:
        - `int`
        - `GrB_Index`
    - values
        - `GRAPH.RAND_VERT`
