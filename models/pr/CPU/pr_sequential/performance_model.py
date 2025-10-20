#!/usr/bin/env python3
import os
import sys
import models.utils as utils


def symbolic_model(T_float_div, T_float_mult, T_float_sub, T_float_add, T_malloc, T_int_add, T_int_mult, T_int_gt, cache_linesizes, mem_access_times, int_size):
    miss_rates = [1 / (linesize/int_size) for linesize in cache_linesizes]
    T_mem_read = utils.avg_mem_access_time(miss_rates, mem_access_times)
    T_mem_write = T_mem_read
    T_neighbour_scan = f'n*({2*T_int_add + T_int_mult + T_int_gt})'
    T_init = f'{2*T_float_div + T_float_sub} + {T_malloc("n")*3}'
    T_init_arrays = f'n*({T_neighbour_scan} + 2*{T_mem_write} + {T_mem_read + T_float_div + T_int_add}) + m*({T_int_add} + {T_mem_write})'
    T_main_loop = f'100*n*({T_neighbour_scan} + {T_float_add + T_float_mult + 2*T_mem_read + T_float_div} + 2*{T_mem_write}) + m*({(T_float_add + T_mem_read)})'
    T_pagerank = f'{T_init} + {T_init_arrays} + {T_main_loop}'
    return T_pagerank


# TODO: find better way to pass linesizes, access times and sizes.
def predict(hardware):
    microbenchmarks = hardware['cpus']['benchmarks']
    return symbolic_model(microbenchmarks['T_float_div'], microbenchmarks['T_float_mult'], microbenchmarks['T_float_sub'], microbenchmarks['T_float_add'],
                          utils.T_malloc, microbenchmarks['T_int_add'], microbenchmarks['T_int_mult'], microbenchmarks['T_int_gt'],
                          [microbenchmarks['L1_linesize'], microbenchmarks['L2_linesize'], microbenchmarks['L3_linesize']],
                          [microbenchmarks['T_L1_read'], microbenchmarks['T_L2_read'], microbenchmarks['T_L3_read'], microbenchmarks['T_DRAM_read']],
                          4)
