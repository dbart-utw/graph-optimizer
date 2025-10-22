import models.utils as utils

EULER_CONSTANT = 0.5772156649015329

# Worst case: we need to traverse the entire diameter of the graph.
def worstcase(T_push_back, cache_linesizes, mem_access_times, int_size):
    int_mem_read = utils.avg_mem_access_time([1/(linesize/int_size) for linesize in cache_linesizes], mem_access_times)
    return f'({T_push_back} + diameter * {T_push_back + int_mem_read}) / 1000000'


# In a random graph, the average shortest path length is "(ln(n) - γ) / ln(<k>) + 1/2",
# where γ is the euler constant, n is the number of nodes, and <k> is the average degree of the graph.
# <k> is equal to 2*m/n, where m is the number of edges in the graph.
def avgcase(T_push_back, cache_linesizes, mem_access_times, int_size):
    int_mem_read = utils.avg_mem_access_time([1/(linesize/int_size) for linesize in cache_linesizes], mem_access_times)
    return f'({T_push_back} + ((math.log(n) - {EULER_CONSTANT}) / math.log(2*m/n) + 1/2) * {T_push_back + int_mem_read}) / 1000000'


# Best case: the node we are looking for is the root node. While loop is never executed, and execution time is constant.
def bestcase(T_int_neq, T_push_back):
    return f'({T_int_neq + T_push_back}) / 1000000'


# For now just predict the worst case scenario. In the future we might
def predict(hardware):
    microbenchmarks = hardware['cpus']['benchmarks']
    return worstcase(microbenchmarks['T_push_back'],
                     [microbenchmarks['L1_linesize'], microbenchmarks['L2_linesize'], microbenchmarks['L3_linesize']],
                     [microbenchmarks['T_L1_read'], microbenchmarks['T_L2_read'], microbenchmarks['T_L3_read'], microbenchmarks['T_DRAM_read']],
                     4)