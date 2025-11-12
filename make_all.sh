#!/bin/bash
set -e

# -------------------------------
# Configuration
# -------------------------------

# Default list of BGOs
DEFAULT_BGOS=("bc" "cc" "tc" "pr" "bfs" "find_max" "find_path")

# Default options
INCLUDE_GPU=false
CLEAN=false
BGOS=()  # Start empty — may be set by args or default later

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
        --*)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [--include-gpu] [--clean] [BGO ...]"
            exit 1
            ;;
        *)
            BGOS+=("$arg")
            ;;
    esac
done

# If no BGOs were specified, use the default list
if [ ${#BGOS[@]} -eq 0 ]; then
    BGOS=("${DEFAULT_BGOS[@]}")
fi

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

	# --- CPU ---
	if [ -d "$bgo_dir/CPU" ]; then
	    echo ">>> $ACTION $bgo (CPU)..."
	    for impl_dir in "$bgo_dir"/CPU/*/ ; do
	        [[ -d "$impl_dir" ]] || continue
	        echo "    -> $(basename "$impl_dir")"
	        (cd "$impl_dir" && $MAKE_CMD)
	    done
	fi
	
	# --- GPU ---
	if [ "$INCLUDE_GPU" = true ] && [ -d "$bgo_dir/GPU" ]; then
	    echo ">>> $ACTION $bgo (GPU)..."
	    for impl_dir in "$bgo_dir"/GPU/*/ ; do
	        [[ -d "$impl_dir" ]] || continue
	        echo "    -> $(basename "$impl_dir")"
	        (cd "$impl_dir" && $MAKE_CMD)
	    done
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
