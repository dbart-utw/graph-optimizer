#include <chrono>
#include <cppJoules.h>

#include "GraphBLAS.h"
// #include "LAGraph.h"

#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

#include "gap/builder.h"
#include "gap/command_line.h"

#include "utils.hpp"
#include "gpu_utils.hpp"

#include "vertex_pull.hpp"

// Capture and parse tracker.print_energy()
long long capture_total_energy(EnergyTracker tracker) {
    // Save original stdout
    fflush(stdout);
    int stdout_fd = dup(STDOUT_FILENO);

    // Create pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    // Redirect stdout to pipe
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    // Call print_energy() (output goes into pipe)
    tracker.print_energy();

    // Restore stdout
    fflush(stdout);
    dup2(stdout_fd, STDOUT_FILENO);
    close(stdout_fd);

    // Read from pipe
    char buffer[4096];
    ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    if (n <= 0) return -1;
    buffer[n] = '\0';

    // --- Parse ---
    std::istringstream iss(buffer);
    std::string label;
    double time_val = 0.0;
    long long energy_sum = 0;

    while (iss >> label) {
        if (label == "Time") {
            iss >> time_val; // skip runtime
        } else {
            long long val;
            if (iss >> val) {
                energy_sum += val;
            }
        }
    }
    return energy_sum;
}

int main(int argc, char **argv) {
    char msg[LAGRAPH_MSG_LEN];
    LAGraph_Init(msg);
    EnergyTracker tracker;

    if (argc < 3) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        return -1;
    }

    int runs = 1;
    if (argc == 4) {
        runs = atoi(argv[3]);
    }

    std::cout << "run,status,runtime_ns,energy_joules" << std::endl;

    for (int run = 0; run < runs; run++) {
        
        Reader<int32_t> r = Reader<int32_t>(argv[1]); bool needs_weights = false; pvector<SGEdge> el = r.ReadFile(needs_weights); BuilderBase<int32_t> b = BuilderBase<int32_t>(); CSRGraph<int32_t> g;
        
        CArray<int32_t> arg_2;
        

        
        g = b.MakeGraphFromEL(el);
        
        write_vector_CArray<int32_t>(arg_2, argv[2]);
        

        auto start = std::chrono::steady_clock::now();
        tracker.start();

        int status = bfs_vertex_pull (
            
            g , 
            
            &arg_2 
            
        );

        tracker.stop();
        auto end = std::chrono::steady_clock::now();
         
        tracker.calculate_energy();
        auto runtime_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        auto total_energy = capture_total_energy(tracker);
        std::cout << run << "," << status << "," << runtime_ns << "," << total_energy << std::endl;

        if (run == 0) {
            
        }

        
        arg_2.free();
        

        if (status != 0) { return status; }
    }

    return 0;
}