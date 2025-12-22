#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/mergesort_omp"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/results3.csv"

# Experiment parameters
ARRAY_SIZES=(10000000 100000000)   # 10^7 and 10^8
ALGO_LIST=(0 1)                     # 0 = Serial, 1 = Parallel
THREADS_LIST=(1 2 4 8 16)
REPEATS=3

mkdir -p "$OUTDIR"
echo "N,algorithm,threads,run,execution_time,verification" > "$RAW_CSV"

# Run experiments
for N in "${ARRAY_SIZES[@]}"; do
  for algo in "${ALGO_LIST[@]}"; do
    for threads in "${THREADS_LIST[@]}"; do
      for ((run=1; run<=REPEATS; run++)); do

        echo "Running N=$N algo=$algo threads=$threads run=$run..."

        out="$("$BIN" "$N" "$algo" "$threads")"

        # Extract execution time
        exec_time=$(echo "$out" | grep "Execution time" | awk '{print $3}')
        # Extract verification result
        verification=$(echo "$out" | grep "Verification:" | awk '{print $2}')

        printf "%d,%d,%d,%d,%s,%s\n" \
          "$N" "$algo" "$threads" "$run" "$exec_time" "$verification" \
          >> "$RAW_CSV"

        # Check correctness
        if [[ "$verification" != "PASS" ]]; then
          echo "❌ Verification FAILED for N=$N algo=$algo threads=$threads run=$run"
          echo "$out"
          exit 1
        fi

      done
    done
  done
done

echo "Experiments completed. Results saved to $RAW_CSV"
echo ""
echo "To plot results, run:"
echo "python3 \"$BASEDIR/src/plot_mergesort.py\" \"$RAW_CSV\""