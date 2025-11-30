#!/bin/bash

# Resolve important directories relative to this script
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)
RESULTS_DIR="$ROOT_DIR/results"
OUT_FILE="$RESULTS_DIR/raw_results.csv"

# Ensure results directory exists and work from repository root
mkdir -p "$RESULTS_DIR"
cd "$ROOT_DIR" || exit 1

# Rebuild binaries to ensure they are up to date
echo "Rebuilding binaries..."
make clean
make barrier_pthread barrier_condvar barrier_spin

# Stop if compilation fails
if [ $? -ne 0 ]; then echo "Build failed."; exit 1; fi

# Write CSV header
echo "Implementation,Threads,Iterations,Total_Time_Sec,Avg_Time_Per_Barrier_Sec" > $OUT_FILE

# Run counts/parameters (override via env, e.g. RUNS=1 ./scripts/barrier_script.sh)
RUNS=${RUNS:-3}
ITERATIONS=${ITERATIONS:-"100000 1000000"}
THREADS=${THREADS:-"1 2 4 8"}
PROGS=${PROGS:-"barrier_pthread barrier_condvar barrier_spin"}

echo "Starting benchmarks ($RUNS runs per config)..."

for prog in $PROGS; do
    for n in $ITERATIONS; do
        for t in $THREADS; do
            
            echo "Testing $prog: Threads=$t, N=$n"
            
            # Reset sum variable
            sum_time=0.0
            
            # Execute runs
            for ((i=1; i<=RUNS; i++)); do
                # Run binary
                output=$(./bin/$prog $t $n)
                
                # Extract total execution time (seconds)
                val=$(echo "$output" | grep "Measured barrier loop time" | awk '{print $(NF-1)}')
                
                # Add to sum (using awk for floating point math)
                sum_time=$(awk "BEGIN {print $sum_time + $val}")
            done

            # Calculate average total time
            avg_total=$(awk "BEGIN {printf \"%.6f\", $sum_time / $RUNS}")
            
            # Calculate average time per single barrier operation
            avg_per_barrier=$(awk "BEGIN {printf \"%.9f\", $avg_total / $n}")

            # Save to CSV
            echo "$prog,$t,$n,$avg_total,$avg_per_barrier" >> $OUT_FILE
            
        done
    done
done

echo "Done. Results saved to $OUT_FILE"