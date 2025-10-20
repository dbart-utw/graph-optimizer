#!/usr/bin/env python3

# Move files in directory (arg1) to another directory (arg2) if they are larger than a certain size (arg3)

import os
import sys
import shutil

def move_large_files(directory, destination, size):
    for file in os.listdir(directory):
        file_path = os.path.join(directory, file)
        if os.path.getsize(file_path) > size:
            shutil.move(file_path, destination)

if __name__ == '__main__':
    move_large_files(sys.argv[1], sys.argv[2], int(sys.argv[3]))