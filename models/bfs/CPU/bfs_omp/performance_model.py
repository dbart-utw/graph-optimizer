#!/usr/bin/env python3
import sys
import os
import models.utils as utils

symbolical_model_parameters = ["T_q_front", "T_q_pop", "T_q_push", "T_int_add", "cache_linesizes", "mem_access_times", "int_size", "float_size"]

def symbolic_model(T_q_front, T_q_pop, T_q_push, T_int_add, cache_linesizes, mem_access_times, int_size, float_size):
    miss_rates = [1 / (linesize/int_size) for linesize in cache_linesizes]
    G_miss_rates = [1 / (linesize/float_size) for linesize in cache_linesizes]
    T_mem_read = utils.avg_mem_access_time(miss_rates, mem_access_times)
    T_G_mem_read = utils.avg_mem_access_time(G_miss_rates, mem_access_times)
    T_init = f"2*{T_mem_read}"
    T_while_loop = f"{T_q_front + T_q_pop}+n*{T_int_add + T_G_mem_read + T_mem_read}"
    T_visit_node = 2*mem_access_times[-1] + mem_access_times[0] + T_int_add + T_q_push
    T_bfs = f"(n*{T_init}+n*({T_while_loop})+n*({T_visit_node})) / 1000000"

    return T_bfs


# TODO: find better way to pass linesizes, access times and sizes.
def predict(hardware):
    microbenchmarks = hardware['cpus']['benchmarks']
    return symbolic_model(microbenchmarks['T_q_front'], microbenchmarks['T_q_pop'], microbenchmarks['T_q_push'], microbenchmarks['T_int_add'],
                          [microbenchmarks['L1_linesize'], microbenchmarks['L2_linesize'], microbenchmarks['L3_linesize']],
                          [microbenchmarks['T_L1_read'], microbenchmarks['T_L2_read'], microbenchmarks['T_L3_read'], microbenchmarks['T_DRAM_read']],
                          4, 4)


if __name__=="__main__":
    arguments = sys.argv[1:]

    # Check if the correct number of arguments were given.
    if len(arguments) != len(symbolical_model_parameters):
        utils.exit_with_error(f"Wrong number of arguments. Script needs {symbolical_model_parameters}, but {len(arguments)} were given.")

    # Read the argumends from commandline, and check if they are of the right type.
    T_q_front = utils.try_cast_float(arguments[0], "Expected a number for parameter 'T_q_front' on position 1")
    T_q_pop = utils.try_cast_float(arguments[1], "Expected a number for parameter 'T_q_pop' on position 2")
    T_q_push = utils.try_cast_float(arguments[2], "Expected a number for parameter 'T_q_push' on position 3")
    T_int_add = utils.try_cast_float(arguments[3], "Expected a number for parameter 'T_add' on position 4")
    cache_linesizes = utils.try_cast_float_list(arguments[4], "Expected a list of numbers for parameter 'cache_linesizes' on position 5")
    mem_access_times = utils.try_cast_float_list(arguments[5], "Expected a list of numbers for parameter 'mem_access_times' on position 6")
    int_size = utils.try_cast_float(arguments[6], "Expected a number for parameter 'int_size' on position 7")
    float_size = utils.try_cast_float(arguments[7], "Expected a number for parameter 'float_size' on position 8")

    # Check if length of mem_access_times is 1 more than cache_linesizes
    if len(mem_access_times) != len(cache_linesizes) + 1:
        utils.exit_with_error("Expected the length of 'mem_access_times' to be 1 more than the length of 'cache_linesizes'")

    # Calculate the symbolic model
    T_bfs = symbolic_model(T_q_front, T_q_pop, T_q_push, T_int_add, cache_linesizes, mem_access_times, int_size, float_size)
    print(T_bfs)
