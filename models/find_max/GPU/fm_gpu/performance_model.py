import models.utils as utils

symbolical_model_parameters = ["T_float_gt", "T_int_add", "cache_linesizes", "mem_access_times", "int_size"]

def symbolic_model(T_float_gt, T_int_add, cache_linesizes, mem_access_times, int_size):
    T_find_max = "n"

    return T_find_max


def predict(hardware):
    microbenchmarks = hardware['cpus']['benchmarks']
    return symbolic_model(microbenchmarks['T_float_gt'], microbenchmarks['T_int_add'],
                          [microbenchmarks['L1_linesize'], microbenchmarks['L2_linesize'], microbenchmarks['L3_linesize']],
                          [microbenchmarks['T_L1_read'], microbenchmarks['T_L2_read'], microbenchmarks['T_L3_read'], microbenchmarks['T_DRAM_read']],
                          4)