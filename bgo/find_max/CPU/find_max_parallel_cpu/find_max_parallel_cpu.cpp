#include "omp.h"
#include <thread>
#include <iostream>
#include <ctime>
#include <vector>
#include "find_max_parallel_cpu.hpp"

struct MaxValue {
    float value;
    int index;
};

/*******************************************************************
 * Version 1: OpenMP with custom reduction
 ******************************************************************/
MaxValue& max(MaxValue& a, MaxValue& b) {
    return a.value > b.value ? a : b;
}

#pragma omp declare reduction(customMax:MaxValue:omp_out=max(omp_out, omp_in))

int find_max_omp_custom_reduction(float *v, int n) {
    MaxValue max = {-1., -1};
    #pragma omp parallel for reduction(customMax:max)
    for (int i = 0; i < n; i++) {
        if (v[i] > max.value) {
            max.value = v[i];
            max.index = i;
        }
    }

    return max.index;
}

/*******************************************************************
 * Version 2: OpenMP with local reduction
 ******************************************************************/
int find_max_omp_local_reduction(float *v, int n) {
    float max_value = -1.;
    int max_index = -1;
    #pragma omp parallel
    {
        float max_value_local = -1.;
        int max_index_local = -1;
        #pragma omp for
        for (int i = 0; i < n; i++) {
            if (v[i] > max_value_local) {
                max_value_local = v[i];
                max_index_local = i;
            }
        }
        #pragma omp critical
        {
            if (max_value_local > max_value) {
                max_value = max_value_local;
                max_index = max_index_local;
            }
        }
    }

    return max_index;
}


/*******************************************************************
 * Version 3: Custom thread pool
 ******************************************************************/
void thread_function(int start_i, int end_i, float *v, float &max_value, int &max_index) {
    for (int i = start_i; i < end_i; i++) {
        if (v[i] > max_value) {
            max_value = v[i];
            max_index = i;
        }
    }
}


int find_max_threads(float *v, int n) {
    std::vector<float> max_values(NUM_THREADS, -1);
    std::vector<int> max_indices(NUM_THREADS, -1);

    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; i++) {
        int start_i = i * n / NUM_THREADS;
        int end_i = (i + 1) * n / NUM_THREADS;

        threads.emplace_back(thread_function, start_i, end_i, v, std::ref(max_values[i]), std::ref(max_indices[i]));
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    float max_value = -1.;
    int max_index = -1;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (max_values[i] > max_value) {
            max_value = max_values[i];
            max_index = max_indices[i];
        }
    }

    return max_index;
}


// int main() {
//     srand(42);
//     const int iterations = 1;
//     const int n = 5000000;
//     float *v = new float[n];

//     for (int i = 0; i < n; i++) {
//         v[i] = static_cast <float> (rand());
//     }

//     omp_set_num_threads(NUM_THREADS);

//     int result_custom_reduction, result_local_reduction, result_threads;

//     clock_t start_time;
//     double time_elapsed_custom_reduction = 0, time_elapsed_local_reduction = 0, time_elapsed_threads = 0;

//     for (int i = 0; i < iterations; i++) {
//         start_time = clock();
//         result_custom_reduction = find_max_omp_custom_reduction(v, n);
//         time_elapsed_custom_reduction += (double)(clock() - start_time) / CLOCKS_PER_SEC;

//         start_time = clock();
//         result_local_reduction = find_max_omp_local_reduction(v, n);
//         time_elapsed_local_reduction += (double)(clock() - start_time) / CLOCKS_PER_SEC;

//         start_time = clock();
//         result_threads = find_max_threads(v, n);
//         time_elapsed_threads += (double)(clock() - start_time) / CLOCKS_PER_SEC;
//     }

//     std::cout << "Time elapsed custom reduction: " << time_elapsed_custom_reduction << ". With result: " << result_custom_reduction << std::endl;
//     std::cout << "Time elapsed local reduction: " << time_elapsed_local_reduction << ". With result: " << result_local_reduction << std::endl;
//     std::cout << "Time elapsed threads: " << time_elapsed_threads << ". With result: " << result_threads << std::endl;

//     std::cout << "Result value: " << v[result_custom_reduction] << std::endl;

//     delete[] v;
// }