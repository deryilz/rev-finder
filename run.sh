#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e
# Ensure failures in piped commands cause the script to exit immediately
set -o pipefail

# ==============================================================================
# Configuration
# ==============================================================================
PATCH_FILE="modifications.patch"
CLEAN_SUBMODULE="cubiomes"
PATCHED_DIR="cubiomes-patch"

# ==============================================================================
# Patching Step
# ==============================================================================
if [ ! -d "$PATCHED_DIR" ]; then
    echo "||PATCHING CUBIOMES||" >&2

    # 1. Check if the clean source exists
    if [ ! -d "$CLEAN_SUBMODULE" ]; then
        echo "Error: Source directory '$CLEAN_SUBMODULE' does not exist." >&2
        exit 1
    fi

    # 2. Check if patch file exists
    if [ ! -f "$PATCH_FILE" ]; then
        echo "Error: Patch file '$PATCH_FILE' not found!" >&2
        exit 1
    fi

    # 3. Create build target
    cp -r "$CLEAN_SUBMODULE" "$PATCHED_DIR"

    # 4. Apply patch with explicit failure handling
    # --dry-run tests the patch first without modifying files
    if ! patch --dry-run -s -p1 -d "$PATCHED_DIR" < "$PATCH_FILE"; then
        echo "||ERROR: Patch dry-run failed! Aborting build.||" >&2
        # Clean up the partial directory so it retries cleanly next run
        rm -rf "$PATCHED_DIR"
        exit 1
    fi

    # Apply the patch for real
    patch -p1 -d "$PATCHED_DIR" < "$PATCH_FILE" > /dev/null

    echo "||PATCH APPLIED SUCCESSFULLY||" >&2
fi

# ==============================================================================
# Compilation Step
# ==============================================================================
echo "||COMPILING||" >&2

# Build C library
make -C "$PATCHED_DIR" > /dev/null
echo "||DONE WITH CUBIOMES||" >&2

# Build main application
g++ main.cpp -std=c++20 -O3 -march=native -flto=auto \
    -I./"$PATCHED_DIR" \
    -L./"$PATCHED_DIR" -lcubiomes -lm \
    -o program

echo "||DONE||" >&2

# Execute binary
./program "$@"
