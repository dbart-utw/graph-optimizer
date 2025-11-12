#!/usr/bin/env python3
import os
import sys
import models.utils as utils


def symbolic_model(T_float_div, T_float_mult, T_float_sub, T_float_add, T_malloc, T_int_add, T_int_mult, T_int_gt, cache_linesizes, mem_access_times, int_size):
    return utils.ERROR_404


# TODO: find better way to pass linesizes, access times and sizes.
def predict(hardware):
    microbenchmarks = hardware['cpus']['benchmarks']
    return symbolic_model(microbenchmarks['T_float_div'], microbenchmarks['T_float_mult'], microbenchmarks['T_float_sub'], microbenchmarks['T_float_add'],
                          utils.T_malloc, microbenchmarks['T_int_add'], microbenchmarks['T_int_mult'], microbenchmarks['T_int_gt'],
                          [microbenchmarks['L1_linesize'], microbenchmarks['L2_linesize'], microbenchmarks['L3_linesize']],
                          [microbenchmarks['T_L1_read'], microbenchmarks['T_L2_read'], microbenchmarks['T_L3_read'], microbenchmarks['T_DRAM_read']],
                          4)
