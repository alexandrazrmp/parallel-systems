#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/shared_var_update"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/raw_results.csv"

ITERATIONS_LIST=(100000 300000 500000 1000000 10000000)
THREADS=(1 2 4 8 10 12 14 16)
REPEATS=4

mkdir -p "$OUTDIR"
echo "iterations,threads,run,mutex_time,rwlock_time,atomic_time" > "$RAW_CSV"

for iters in "${ITERATIONS_LIST[@]}"; do
  echo ""
  echo "=== Testing iteration count: $iters ==="

  for th in "${THREADS[@]}"; do
    echo "  Threads=$th (repeats=$REPEATS)"

    for ((i=1; i<=REPEATS; i++)); do

      # ---- RUN PROGRAM AND STORE OUTPUT ----
      out="$("$BIN" "$iters" "$th" 2>&1)"

      # ---- CHECK VERIFICATION ----
      if ! echo "$out" | grep -q "Verification: PASS"; then
        echo ""
        echo "ERROR: Verification failed!"
        echo "Iterations=$iters Threads=$th Run=$i"
        echo ""
        echo "Program output:"
        echo "$out"
        echo ""
        echo "Aborting experiment to prevent logging invalid results."
        exit 1
      fi

      # ---- EXTRACT TIMES ----
      mutex=$(  echo "$out" | awk '/Mutex approach time/ {print $4}' )
      rwlock=$( echo "$out" | awk '/Read-Write Lock approach time/ {print $5}' )
      atomic=$( echo "$out" | awk '/Atomic approach time/ {print $4}' )

      # ---- APPEND TO CSV ----
      printf "%s,%s,%d,%s,%s,%s\n" \
        "$iters" "$th" "$i" "$mutex" "$rwlock" "$atomic" >> "$RAW_CSV"
    done
  done
done

echo ""
echo "Done. Raw results saved to: $RAW_CSV"
echo ""
echo "To plot results, run:"
echo "python3 \"$BASEDIR/src/shared_var_plot.py\" \"$RAW_CSV\""
