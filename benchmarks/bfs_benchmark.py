import subprocess
import os
import sys

def predicted_time(n):
    return (171.429*n + 3.36897*n**2) / 1000000000

with open(sys.argv[1], "w") as csv_file:
    csv_file.write("dataset,nodes,edges,measured_time,predicted_time\n")
    for filename in os.listdir("../data/bfs_test/star_graphs/"):
        if filename.endswith(".mtx"):
            print("Running sssp_benchmark with input file: " + filename)
            with open("../data/bfs_test/star_graphs/" + filename) as f:
                lines = f.readlines()
                num_nodes = int(lines[1].split()[0])
                num_edges = int(lines[1].split()[2])

            execution_times = []
            for i in range(10):
                output = subprocess.check_output(["./sssp_benchmark", "../data/bfs_test/star_graphs/" + filename])
                executon_time = float(output.decode("utf-8").split(':')[1].strip())
                print(f"Execution time run {i}:", executon_time, "seconds")
                execution_times.append(float(output.decode("utf-8").split(':')[1].strip()))
                csv_file.write(filename[:-4] + "," + str(num_nodes) + "," + str(num_edges) + "," + str(executon_time) + "," + str(predicted_time(num_nodes)) + "\n")

            print("Avg execution time:", sum(execution_times) / len(execution_times), "seconds")
            # extract number of nodes from file (second line, first integer)
            print("Predicted time was:", predicted_time(num_nodes))
