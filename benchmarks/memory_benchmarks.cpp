#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <iostream>

#define KB 1024
#define MB (1024 * KB)

// Define cache sizes in bytes
#define L1_SIZE 32768
#define L2_SIZE 524288
#define L3_SIZE 16777216
#define DRAM_SIZE (L3_SIZE * 2)

// Macro to prevent compiler optimizations
#define DO_NOT_OPTIMIZE(var) asm volatile("" : "+m"(var))

void measure_latency(char *array, size_t size) {
    struct timespec before, after;
    double read_time = 0, write_time = 0;

    // Measure write latency
    for (size_t i = 0; i < size; i += 64) {
        clock_gettime(CLOCK_MONOTONIC, &before);
        array[i] = i;
        clock_gettime(CLOCK_MONOTONIC, &after);
        write_time += (double)(after.tv_sec - before.tv_sec) * 1e9 +
              (double)(after.tv_nsec - before.tv_nsec);
    }

    std::cout << write_time << std::endl;

    // Measure read latency
    for (size_t i = 0; i < size; i += 64) {
        clock_gettime(CLOCK_MONOTONIC, &before);
        DO_NOT_OPTIMIZE(array[i]);
        clock_gettime(CLOCK_MONOTONIC, &after);
        read_time += (double)(after.tv_sec - before.tv_sec) * 1e9 +
              (double)(after.tv_nsec - before.tv_nsec);
    }

    printf("Size: %zu KB - Read Latency: %f cycles, Write Latency: %f cycles\n",
           size / KB, read_time / (size / 64), write_time / (size / 64));
}

int main() {
    char *array;

    // Allocate memory for different levels
    array = (char *)malloc(DRAM_SIZE);
    if (!array) {
        perror("Failed to allocate memory");
        return EXIT_FAILURE;
    }

    printf("Measuring latencies...\n");

    // Measure latencies for different cache levels and DRAM
    measure_latency(array, L1_SIZE);
    measure_latency(array, L2_SIZE);
    measure_latency(array, L3_SIZE);
    measure_latency(array, DRAM_SIZE);

    free(array);

    return 0;
}
