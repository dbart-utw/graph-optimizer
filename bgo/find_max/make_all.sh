#!/bin/bash
# Run make in each subdirectory two levels deep from the current directory

for dir in */*/; do
    if [ -d "$dir" ]; then
        echo "Entering $dir"
        (cd "$dir" && make)
    fi
done