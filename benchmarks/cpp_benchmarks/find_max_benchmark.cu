#include "GraphBLAS.h"
#include "LAGraph.h"
#include "../../bgo/find_max.hpp"
#include "../../bgo/find_max_gpu.cuh"
#include "../../bgo/find_max_parallel_cpu.hpp"
#include "../../include/utils.hpp"
#include <stdio.h>
#include <fstream>
#include <ctime>
#include <cassert>

int main(int argc, char **argv) {
    srand(time(NULL));

    /*
     * Initialize output csv files for all methods
     */
    std::ofstream output_csv_array;
    std::ofstream output_csv_graphblas;
    std::ofstream output_csv_gpu;
    std::ofstream output_csv_custom_red;
    std::ofstream output_csv_local_red;
    std::ofstream output_csv_threads;
    output_csv_array.open(argv[1] + std::string("array.csv"));
    output_csv_graphblas.open(argv[1] + std::string("graphblas.csv"));
    output_csv_gpu.open(argv[1] + std::string("gpu.csv"));
    output_csv_custom_red.open(argv[1] + std::string("parallel_omp_custom_reduction.csv"));
    output_csv_local_red.open(argv[1] + std::string("parallel_omp_local_reduction.csv"));
    output_csv_threads.open(argv[1] + std::string("parallel_threads.csv"));
    output_csv_array << "iteration,array_size,time" << std::endl;
    output_csv_graphblas << "iteration,array_size,time" << std::endl;
    output_csv_gpu << "iteration,array_size,time" << std::endl;
    output_csv_custom_red << "iteration,array_size,time" << std::endl;
    output_csv_local_red << "iteration,array_size,time" << std::endl;
    output_csv_threads << "iteration,array_size,time" << std::endl;

    const uint64_t iterations = 10;
    int total = 0;
    int max_index = 0;
    clock_t start_time;
    double time_elapsed;
    uint64_t ns[] = {1000, 5000, 10000, 50000, 100000, 500000, 1000000, 5000000, 10000000, 50000000, 100000000};
    for (uint64_t n : ns) {
        std::cout << "Array size: " << n << std::endl;

        float *v = new float[n];
        GrB_Vector v_grb;
        GrB_Info info = GrB_Vector_new(&v_grb, GrB_FP32, n);

        for (int i = 0; i < iterations; i++) {
            for (uint64_t j = 0; j < n; j++) {
                v[j] = static_cast <float> (rand());
                GrB_Index grb_j = j;
                GrB_Vector_setElement_FP32(v_grb, v[j], grb_j);
            }

            start_time = clock();
            max_index = find_max(v, n);
            time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            output_csv_array << i << "," << n << "," << time_elapsed << std::endl;
            printf("Array: Max index: %d, time: %f\n", max_index, time_elapsed);
            total += max_index;

            start_time = clock();
            max_index = find_max(v_grb, n);
            time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            output_csv_graphblas << i << "," << n << "," << time_elapsed << std::endl;
            printf("GrB Vector: Max index: %d, time: %f\n", max_index, time_elapsed);
            total += max_index;

            start_time = clock();
            max_index = find_max_gpu(v, n);
            time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            output_csv_gpu << i << "," << n << "," << time_elapsed << std::endl;
            printf("GPU: Max index: %d, time: %f\n", max_index, time_elapsed);
            total += max_index;

            start_time = clock();
            max_index = find_max_omp_custom_reduction(v, n);
            time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            output_csv_custom_red << i << "," << n << "," << time_elapsed << std::endl;
            printf("Custom Red: Max index: %d, time: %f\n", max_index, time_elapsed);
            total += max_index;

            start_time = clock();
            max_index = find_max_omp_local_reduction(v, n);
            time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            output_csv_local_red << i << "," << n << "," << time_elapsed << std::endl;
            printf("Local red: Max index: %d, time: %f\n", max_index, time_elapsed);
            total += max_index;

            start_time = clock();
            max_index = find_max_threads(v, n);
            time_elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            output_csv_threads << i << "," << n << "," << time_elapsed << std::endl;
            printf("Threads: Max index: %d, time: %f\n", max_index, time_elapsed);
            total += max_index;
        }
        // assert(max_index * 6 * iterations == total);
        delete[] v;
    }

    return total;
}