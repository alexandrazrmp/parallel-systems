#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/sparse_matrix_vector_mult"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/raw_results.csv"

# Experiment parameters
MATRIX_SIZES=(1000 2000 5000 10000)
SPARSITY_LIST=(0 10 30 50 70 90 100)
ITERATIONS_LIST=(1 5 10 20)
THREADS_LIST=(1 2 4 8)
REPEATS=3

mkdir -p "$OUTDIR"
echo "N,sparsity,iterations,threads,run,serial_CSR_formation,parallel_CSR_formation,serial_CSR_mult,parallel_CSR_mult,serial_dense_mult,parallel_dense_mult" > "$RAW_CSV"

# Function to extract times from output
extract_time() {
    local output="$1"
    local pattern="$2"
    echo "$output" | grep "$pattern" | awk -F':' '{print $2}' | awk '{print $1}'
}

# Run experiments
for N in "${MATRIX_SIZES[@]}"; do
  for sparsity in "${SPARSITY_LIST[@]}"; do
    for iter in "${ITERATIONS_LIST[@]}"; do
      for threads in "${THREADS_LIST[@]}"; do
        for ((run=1; run<=REPEATS; run++)); do
          echo "Running N=$N, sparsity=$sparsity%, iterations=$iter, threads=$threads, run=$run..."
          
          out="$("$BIN" "$N" "$sparsity" "$iter" "$threads" 2>&1)"

          # Check correctness - look for PASS messages in different sections
          if ! echo "$out" | grep -q "CSR Formation Verification: PASS" || \
             ! echo "$out" | grep -q "CSR Multiplication Verification: PASS" || \
             ! echo "$out" | grep -q "Dense Multiplication Verification: PASS" || \
             ! echo "$out" | grep -q "Final Verification (CSR vs Dense): PASS"; then
            echo "❌ Verification FAILED for N=$N sparsity=$sparsity iterations=$iter threads=$threads run=$run"
            echo "$out"
            exit 1
          fi

          # Extract times from the output
          serial_CSR_formation=$(extract_time "$out" "Serial CSR formation time")
          parallel_CSR_formation=$(extract_time "$out" "Parallel CSR formation time")
          serial_CSR_mult=$(extract_time "$out" "Serial CSR multiplication time")
          parallel_CSR_mult=$(extract_time "$out" "Parallel CSR multiplication time")
          serial_dense_mult=$(extract_time "$out" "Serial dense multiplication time")
          parallel_dense_mult=$(extract_time "$out" "Parallel dense multiplication time")

          # Write to CSV
          printf "%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s\n" \
            "$N" "$sparsity" "$iter" "$threads" "$run" \
            "$serial_CSR_formation" "$parallel_CSR_formation" \
            "$serial_CSR_mult" "$parallel_CSR_mult" \
            "$serial_dense_mult" "$parallel_dense_mult" \
            >> "$RAW_CSV"


        done
      done
    done
  done
done

echo ""
echo "Experiments completed. Results saved to $RAW_CSV"
echo ""
echo "Summary of experiments run:"
echo "- Matrix sizes: ${MATRIX_SIZES[@]}"
echo "- Sparsity levels: ${SPARSITY_LIST[@]}%"
echo "- Iterations: ${ITERATIONS_LIST[@]}"
echo "- Thread counts: ${THREADS_LIST[@]}"
echo "- Repeats per configuration: $REPEATS"
echo ""
echo "To plot results, run:"
echo "python3 \"src/sparse_mat_plot.py\" \"$RAW_CSV\""
echo "python3 \"src/sparse_mat_plot2.py\" \"$RAW_CSV\""