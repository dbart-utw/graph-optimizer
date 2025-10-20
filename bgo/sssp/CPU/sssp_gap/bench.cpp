#include <chrono>
#include <cppJoules.h>

#include "GraphBLAS.h"
// #include "LAGraph.h"

#include "utils.hpp"
#include "gap/builder.h"
#include "gap/command_line.h"

#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

#include "sssp_gap.hpp"

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

    if (argc < 5 || argc > 7) {
        std::cerr << "Invalid number of arguments!" << std::endl;
        return -1;
    }

    int runs = atoi(argv[5]);

    std::cout << "run,status,runtime_ns,energy_joules" << std::endl;

    
    Reader<int32_t> r = Reader<int32_t>(argv[1]); bool needs_weights = false; pvector<SGEdge> el = r.ReadFile(needs_weights); BuilderBase<int32_t> b = BuilderBase<int32_t>(); PRGraph g;
    
    int arg_2 = (int) atoi(argv[2]);
    
    int arg_3 = (int) atoi(argv[3]);
    
    CArray<int> arg_4;
    

    
    g = b.MakeGraphFromEL(el);
    

    // char* reorder = argv[6];
    // 
    // 
    // if (strcmp(reorder, "none") == 0) {
    // 
    // 
    // 
    // 
    // } else if (strcmp(reorder, "Random") == 0) {
    // 
    // g.ReorderRandom();
    // 
    // 
    // } else if (strcmp(reorder, "Reverse") == 0) {
    // 
    // g.ReorderReverse();
    // 
    // 
    // } else if (strcmp(reorder, "MaxDegree") == 0) {
    // 
    // g.ReorderMaxDegree();
    // 
    // 
    // } else if (strcmp(reorder, "MinDegree") == 0) {
    // 
    // g.ReorderMinDegree();
    // 
    // 
    // } else if (strcmp(reorder, "BFS") == 0) {
    // 
    // g.ReorderBFS(0);
    // 
    // } else {
    //     std::cerr << "Invalid reorder option!" << std::endl;
    //     return -1;
    // }

    for (int run = 0; run < runs; run++) {
        auto start = std::chrono::steady_clock::now();
        tracker.start();

        int status = SSSP_gap (
            
            g , 
            
            arg_2 , 
            
            arg_3 , 
            
            &arg_4 
            
        );

        tracker.stop();
        auto end = std::chrono::steady_clock::now();
         
        tracker.calculate_energy();
        auto runtime_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        auto total_energy = capture_total_energy(tracker);
        std::cout << run << "," << status << "," << runtime_ns << "," << total_energy << std::endl;


        if (run == 0) {
            
            write_vector_CArray(arg_4, argv[4]);
            
        }

        
        arg_4.free();
        

        if (status != 0) { return status; }
    }

    return 0;
}