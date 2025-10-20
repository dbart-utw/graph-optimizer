import models.utils as utils

symbolical_model_parameters = ["T_float_gt", "T_int_add", "cache_linesizes", "mem_access_times", "int_size"]

def symbolic_model(T_float_gt, T_int_add, cache_linesizes, mem_access_times, int_size):
    miss_rates = [1 / (linesize/int_size) for linesize in cache_linesizes]
    T_mem_read = utils.avg_mem_access_time(miss_rates, mem_access_times)
    T_find_max = f"n*({T_mem_read + T_float_gt + mem_access_times[0] + T_int_add}) / 1000000"

    return T_find_max


def predict(hardware):
    microbenchmarks = hardware['cpus']['benchmarks']
    return symbolic_model(microbenchmarks['T_float_gt'], microbenchmarks['T_int_add'],
                          [microbenchmarks['L1_linesize'], microbenchmarks['L2_linesize'], microbenchmarks['L3_linesize']],
                          [microbenchmarks['T_L1_read'], microbenchmarks['T_L2_read'], microbenchmarks['T_L3_read'], microbenchmarks['T_DRAM_read']],
                          4)