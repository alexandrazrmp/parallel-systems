#!/usr/bin/env bash
# Experiment driver for shared_var_update

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/shared_var_update"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/raw_results.csv"

# ---------- DEFAULT PARAMETERS ----------
# Not too many, not too heavy
ITERATIONS_LIST=(100000 300000 500000 1000000 10000000)
THREADS=(1 2 4 8 10 12 14 16)
REPEATS=4

# ---------- COMMAND-LINE ARGUMENTS ----------
while getopts "i:t:r:" opt; do
  case $opt in
    i) IFS=' ' read -r -a ITERATIONS_LIST <<< "$OPTARG" ;;
    t) IFS=' ' read -r -a THREADS <<< "$OPTARG" ;;
    r) REPEATS="$OPTARG" ;;
    *)
      echo "Usage: $0 -i \"100000 300000 500000 1000000\" -t \"1 2 4 8\" -r 4"
      exit 1
      ;;
  esac
done

mkdir -p "$OUTDIR"
echo "iterations,threads,run,mutex_time,rwlock_time,atomic_time" > "$RAW_CSV"

# ---------- RUN EXPERIMENTS ----------
for iters in "${ITERATIONS_LIST[@]}"; do
  echo ""
  echo "=== Testing iteration count: $iters ==="

  # Safety: skip insane values
  if (( iters > 20000000 )); then
    echo "Skipping $iters (too large)"
    continue
  fi

  for th in "${THREADS[@]}"; do
    echo "  Threads=$th (repeats=$REPEATS)"

    for ((i=1; i<=REPEATS; i++)); do

      out="$("$BIN" "$iters" "$th" 2>&1)"

      mutex=$(  echo "$out" | awk '/Mutex approach time/ {print $4}' )
      rwlock=$( echo "$out" | awk '/Read-Write Lock approach time/ {print $5}' )
      atomic=$( echo "$out" | awk '/Atomic approach time/ {print $4}' )

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
