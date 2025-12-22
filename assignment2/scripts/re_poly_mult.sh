#!/usr/bin/env bash
# Simple experiment driver for OpenMP assignment
# Usage: ./scripts/run_experiments.sh

set -e

# Resolve paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/poly_mult_omp"
OUTDIR="$BASEDIR/results"
CSV_FILE="$OUTDIR/results.csv"

# Experiment parameters
DEGREES=(10000 50000 100000 500000)  # Polynomial degrees
THREADS=(1 2 4 8)             # Thread counts to test
REPEATS=3                     # Repeats for average

# Check if binary exists
if [ ! -f "$BIN" ]; then
    echo "Error: Binary $BIN not found. Run 'make' first."
    exit 1
fi

# Create results directory and write CSV header
mkdir -p "$OUTDIR"
echo "degree,threads,run,serial_time,parallel_time,verification" > "$CSV_FILE"

echo "Starting experiments..."

# Loop through all parameters
for n in "${DEGREES[@]}"; do
    for th in "${THREADS[@]}"; do
        for ((i=1; i<=REPEATS; i++)); do
            echo "Running: N=$n Threads=$th (Run $i/$REPEATS)"
            
            # Run the program
            output=$("$BIN" "$n" "$th")
            
            # Extract results using grep and awk
            serial=$(echo "$output" | grep "Serial multiplication time" | awk '{print $(NF-1)}')
            parallel=$(echo "$output" | grep "Parallel multiplication time" | awk '{print $(NF-1)}')
            ver=$(echo "$output" | grep "Verification" | awk '{print $2}')
            
            # Save to CSV
            echo "$n,$th,$i,$serial,$parallel,$ver" >> "$CSV_FILE"
        done
    done
done

echo "Done. Results saved to: $CSV_FILE"
echo "To plot, run: python3 src/plot_results.py $CSV_FILE"