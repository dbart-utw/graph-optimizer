#!/bin/bash
#SBATCH --job-name=reorder
#SBATCH --output=benchmarks/logs/autobench_%j.log
#SBATCH --time=24:00:00

# List of bgo values
BGOS=(bc bfs cc pr sssp tc)
# List of thread counts
# THREADS=(1 2 4 8 16 32)
THREADS=(8)
# List of reorder methods
# REORDERS=(Random BFS MinDegree MaxDegree)
REORDERS=(BFS)

for BGO in "${BGOS[@]}"; do

    for REORDER in "${REORDERS[@]}"; do
        OUTPUT_DIR="benchmarks/gap_results/reorder_exp/${BGO}/reorder_${REORDER}"
        mkdir -p "$OUTPUT_DIR"
        for N in "${THREADS[@]}"; do
            for i in {1..10}; do
                sbatch --dependency=singleton --job-name="${BGO}_bench" --wrap="srun python3 autobench/run_bench.py bgo/${BGO}/${BGO}_gap/ \
                    --runs 1 \
                    --data G=/var/scratch/dbart/data/konect/converted/small/* \
                    --output ${OUTPUT_DIR}/output_${N}_threads.csv \
                    --threads ${N} \
                    --reorder ${REORDER}"
            done
        done
    done
done