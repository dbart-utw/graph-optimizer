#ifndef FIND_MAX_PARALLEL_CPU_HPP
#define FIND_MAX_PARALLEL_CPU_HPP

#define NUM_THREADS 16

int find_max_omp_custom_reduction(float *v, int n);
int find_max_omp_local_reduction(float *v, int n);
int find_max_threads(float *v, int n);

#endif