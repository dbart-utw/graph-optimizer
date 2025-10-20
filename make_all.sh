#!/bin/bash
set -e

# -------------------------------
# Configuration
# -------------------------------

# List of BGOs to build/clean
BGOS=("bc" "cc" "tc" "pr" "bfs" "find_max" "find_path")

# Default options
INCLUDE_GPU=false
CLEAN=false

# -------------------------------
# Parse arguments
# -------------------------------
for arg in "$@"; do
    case "$arg" in
        --include-gpu)
            INCLUDE_GPU=true
            ;;
        --clean)
            CLEAN=true
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [--include-gpu] [--clean]"
            exit 1
            ;;
    esac
done

# -------------------------------
# Action
# -------------------------------
ACTION="build"
MAKE_CMD="make -j$(nproc)"
if [ "$CLEAN" = true ]; then
    ACTION="clean"
    MAKE_CMD="make clean"
fi

echo ">>> Performing $ACTION on selected BGO implementations..."
echo "    Included BGOs: ${BGOS[*]}"
echo "    Include GPU:   ${INCLUDE_GPU}"

for bgo in "${BGOS[@]}"; do
    bgo_dir="bgo/${bgo}"
    if [ ! -d "$bgo_dir" ]; then
        echo "Skipping $bgo — directory not found: $bgo_dir"
        continue
    fi

    if [ -d "$bgo_dir/CPU" ]; then
        echo ">>> $ACTION $bgo (CPU)..."
        find "$bgo_dir/CPU" -type f -name Makefile -execdir $MAKE_CMD \;
    fi

    if [ "$INCLUDE_GPU" = true ] && [ -d "$bgo_dir/GPU" ]; then
        echo ">>> $ACTION $bgo (GPU)..."
        find "$bgo_dir/GPU" -type f -name Makefile -execdir $MAKE_CMD \;
    fi
done

# -------------------------------
# Sampling module
# -------------------------------
if [ -d "sampling" ]; then
    echo ">>> $ACTION sampling module..."
    (cd sampling && $MAKE_CMD || true)
fi

echo "All $ACTION operations complete."
