#!/bin/bash
# re_array_stats.sh

# Set output directory and file
RESULTS_DIR="results"
OUT_FILE="$RESULTS_DIR/raw_results_stats.csv"

# Create results directory if missing
mkdir -p "$RESULTS_DIR"

# Rebuild using Make
echo "Rebuilding..."
make clean
make array_stats

# Stop if build fails
if [ $? -ne 0 ]; then echo "Build failed"; exit 1; fi

# Write CSV header
echo "Size,Init_Time,Serial_Time,Parallel_Time,Verification" > "$OUT_FILE"

# Large array sizes to ensure False Sharing is visible
SIZES="10000000 50000000 100000000 200000000"

echo "Starting experiments..."

for size in $SIZES; do
    echo "Running with array size: $size"
    
    # Run binary and append output to the results file
    ./bin/array_stats $size >> "$OUT_FILE"
done

echo "Done. Results saved to $OUT_FILE"