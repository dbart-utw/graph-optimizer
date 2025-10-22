import models.utils as utils

symbolical_model_parameters = ["T_heap_insert_max", "T_int_add", "T_malloc_n", "T_heap_extract_min", "T_heap_decrease_key", "cache_linesizes", "mem_access_times", "bool_size", "int_size", "float_size"]

def symbolic_model(T_heap_insert_max, T_int_add, T_heap_extract_min, T_heap_decrease_key, cache_linesizes, mem_access_times, bool_size, int_size, float_size):
    T_float_mem_read = utils.avg_mem_access_time([1/(linesize/float_size) for linesize in cache_linesizes], mem_access_times)
    T_vprop_mem_read = utils.avg_mem_access_time([1/(linesize/(2*int_size)) for linesize in cache_linesizes], mem_access_times)
    T_bool_mem_read =utils.avg_mem_access_time([1/(linesize/bool_size) for linesize in cache_linesizes], mem_access_times)
    T_heap_init = f'n*({T_heap_insert_max})'
    T_dijkstra = f'{T_heap_init} + n*({T_vprop_mem_read + T_bool_mem_read}) + n*({T_heap_extract_min + mem_access_times[-1]} + n*({T_int_add + T_bool_mem_read + T_float_mem_read + T_vprop_mem_read + mem_access_times[-1] + T_float_mem_read + 4*mem_access_times[0] + T_heap_decrease_key}))'
    T_inner_loop = f'{2*T_vprop_mem_read} + n*({T_int_add + 2*mem_access_times[0] + T_vprop_mem_read + mem_access_times[-1]})'
    T_bc = f'(n*{T_float_mem_read} + s*({T_dijkstra} + {T_vprop_mem_read}) + s*(s-1)*({T_inner_loop})) / 1000000'

    return T_bc


def predict(hardware):
    microbenchmarks = hardware['cpus']['benchmarks']
    return symbolic_model(microbenchmarks['T_heap_insert_max'], microbenchmarks['T_int_add'], microbenchmarks['T_heap_extract_min'], microbenchmarks['T_heap_decrease_key'],
                          [microbenchmarks['L1_linesize'], microbenchmarks['L2_linesize'], microbenchmarks['L3_linesize']],
                          [microbenchmarks['T_L1_read'], microbenchmarks['T_L2_read'], microbenchmarks['T_L3_read'], microbenchmarks['T_DRAM_read']],
                          1, 4, 4)