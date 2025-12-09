#!/usr/bin/env bash
# Usage: ./scripts/run_mergesort.sh

set -e
BIN="./bin/mergesort_omp"

# Make sure it builds
make

echo "--- Running Mergesort Experiments ---"

# Array sizes: 10 million and 100 million (as per exercise)
# Note: 100 million integers take ~400MB RAM.
SIZES=(10000000 100000000)
THREADS=(1 2 4 8)

for n in "${SIZES[@]}"; do
    echo "========================================"
    echo "Testing Size: $n"
    
    # 1. Run Serial (Algo 0)
    echo "-> Running Serial..."
    "$BIN" "$n" 0 1 | grep "Execution time"
    
    # 2. Run Parallel (Algo 1) with different threads
    for th in "${THREADS[@]}"; do
        echo "-> Running Parallel with $th threads..."
        "$BIN" "$n" 1 "$th" | grep "Execution time"
    done
done