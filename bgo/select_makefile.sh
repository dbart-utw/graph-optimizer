#!/usr/bin/env bash
set -euo pipefail

# --- Configuration and usage ---
if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <device_name>"
    echo "Example: $0 das6"
    exit 1
fi

DEVICE="$1"
MAKEFILES_DIR="makefiles/$DEVICE"

# Check that the device makefiles exist
if [[ ! -d "$MAKEFILES_DIR" ]]; then
    echo "Error: Device directory '$MAKEFILES_DIR' not found."
    exit 1
fi

echo "🔧 Updating Makefile links for device: $DEVICE"
echo

# --- Traverse all top-level directories (BGOs) ---
for bgo_dir in */ ; do
    # Skip the makefiles directory itself
    [[ "$bgo_dir" == "makefiles/" ]] && continue

    echo "📂 Processing BGO: ${bgo_dir%/}"

    for target in CPU GPU; do
        target_dir="$bgo_dir/$target"
        [[ -d "$target_dir" ]] || continue

        echo "  ▶ Found $target implementations"

        # Go one level deeper (implementation dirs)
        for impl_dir in "$target_dir"/*/ ; do
            # Skip non-directories
            [[ -d "$impl_dir" ]] || continue

            impl_makefile="${impl_dir}Makefile"
            link_target="../../$MAKEFILES_DIR/$target"

            # Remove old Makefile if it exists (file or symlink)
            if [[ -e "$impl_makefile" || -L "$impl_makefile" ]]; then
                rm -f "$impl_makefile"
                echo "    - Removed old Makefile from $impl_dir"
            fi

            # Create symlink
            ln -s "$link_target" "$impl_makefile"
            echo "    + Linked $impl_makefile → $link_target"
        done
    done

    echo
done

echo "✅ All Makefiles updated successfully for device '$DEVICE'."
