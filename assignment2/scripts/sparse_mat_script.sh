#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/sparse_matrix_vector_mult"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/raw_results.csv"

# Experiment parameters
MATRIX_SIZES=(1000)
# 2000 5000 10000)
SPARSITY_LIST=(0)
# 10 30 50 70 90 100)
ITERATIONS_LIST=(1 )
#5 10 20)
THREADS_LIST=(1)
# 2 4 8)
REPEATS=3

mkdir -p "$OUTDIR"
echo "N,sparsity,iterations,threads,run,CSR_total_time,CSR_formation_time,CSR_multiplication_time,Dense_time" > "$RAW_CSV"

# Run experiments
for N in "${MATRIX_SIZES[@]}"; do
  for sparsity in "${SPARSITY_LIST[@]}"; do
    for iter in "${ITERATIONS_LIST[@]}"; do
      for threads in "${THREADS_LIST[@]}"; do
        for ((run=1; run<=REPEATS; run++)); do

          out="$("$BIN" "$N" "$sparsity" "$iter" "$threads")"

          # Check correctness
          if ! echo "$out" | grep -q "Verification: PASS"; then
            echo "❌ Verification FAILED for N=$N sparsity=$sparsity iterations=$iter threads=$threads run=$run"
            echo "$out"
            exit 1
          fi

          # Extract times
          CSR_formation=$(echo "$out" | grep "CRS formation time" | awk '{print $4}')
          CSR_mult=$(echo "$out" | grep "CSR multiplication time" | awk '{print $4}')
          CSR_total=$(echo "$out" | grep "TOTAL CSR time" | awk '{print $4}')
          Dense_time=$(echo "$out" | grep "Dense multiplication time" | awk '{print $4}')

          printf "%d,%d,%d,%d,%d,%s,%s,%s,%s\n" \
            "$N" "$sparsity" "$iter" "$threads" "$run" "$CSR_total" "$CSR_formation" "$CSR_mult" "$Dense_time" \
            >> "$RAW_CSV"

        done
      done
    done
  done
done

echo "Experiments completed. Results saved to $RAW_CSV"
