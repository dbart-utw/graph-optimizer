#!/bin/python3
import subprocess
import random
import os

def microbenchmark_ops():
    script_dir = os.path.dirname(os.path.realpath(__file__))
    cpp_file = os.path.join(script_dir, 'microbenchmarks.cpp')
    binary_file = os.path.join(script_dir, 'microbenchmarks')

    subprocess.run(['g++', '-o', binary_file, cpp_file])
    microbenchmark_output = subprocess.check_output([binary_file], shell=True).decode('utf-8').split()
    microbenchmarks = {x.split(':')[0]: float(x.split(':')[1]) for x in microbenchmark_output}

    return microbenchmarks

def get_linesizes():
    cache_output = subprocess.check_output('getconf -a | grep CACHE', shell=True).decode('utf-8').split('\n')
    output = {line.split()[0]: int(line.split()[1]) for line in cache_output if len(line.split()) == 2}
    linesizes = {}
    if ('LEVEL1_DCACHE_LINESIZE' in output):
        linesizes['L1_linesize'] = output['LEVEL1_DCACHE_LINESIZE']
    if ('LEVEL2_CACHE_LINESIZE' in output):
        linesizes['L2_linesize'] = output['LEVEL2_CACHE_LINESIZE']
    if ('LEVEL3_CACHE_LINESIZE' in output):
        linesizes['L3_linesize'] = output['LEVEL3_CACHE_LINESIZE']

    return linesizes

def mem_latencies():
    # TODO: use actual latencies
    # Measured values for Anton
    L1_time = 1.26
    L2_time = 4.24
    L3_time = 20.9
    DRAM_time = 62.5

    # Add noise between 0 and 100% to the latencies
    latencies = {'T_L1_read': L1_time + random.uniform(0, 1) * L1_time,
                 'T_L2_read': L2_time + random.uniform(0, 1) * L2_time,
                 'T_L3_read': L3_time + random.uniform(0, 1) * L3_time,
                 'T_DRAM_read': DRAM_time + random.uniform(0, 1) * DRAM_time}

    return latencies

def all_benchmarks():
    print("Running microbenchmarks...")
    microbenchmarks = microbenchmark_ops()
    linesizes = get_linesizes()
    latencies = mem_latencies()

    # Concatinate all dictionaries
    benchmarks = {**microbenchmarks, **linesizes, **latencies}
    return benchmarks

if __name__ == '__main__':
    print(all_benchmarks())