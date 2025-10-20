#
# HOW TO USE THIS SCRIPT:
#
# Run the script with three arguments.
# The first argument is the path to the benchmark executable.
# This executable should take two argument, the filename of the input graph and an output csv file name.
# The second argument is the path to the directory containing the input graphs.
# The final argument is the path to the output directory to which the results will be written.
#

import os
import sys
import subprocess

for filename in os.listdir(sys.argv[2]):
    if filename.endswith(".mtx"):
        print("=====Running benchmark with input file: " + filename + "=====")
        output = subprocess.check_output([sys.argv[1], sys.argv[2] + filename, sys.argv[3] + filename[:-4] + ".csv"])
