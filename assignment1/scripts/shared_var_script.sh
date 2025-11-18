#!/usr/bin/env bash
# Experiment driver for shared_var_update
# Usage example:
#   ./scripts/shared_var_script.sh -i "10000000 20000000" -t "1 2 4 8" -r 4

set -euo pipefail

# Resolve directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/shared_var_update"
OUTDIR="$BASEDIR/results2"
RAW_CSV="$OUTDIR/raw_results2.csv"

# ---------- DEFAULT PARAMETERS ----------
ITERATIONS_LIST=(10000000)
THREADS=(1 2 4 8)
REPEATS=4

# ---------- COMMAND-LINE ARGUMENTS ----------
while getopts "i:t:r:" opt; do
  case $opt in
    i) IFS=' ' read -r -a ITERATIONS_LIST <<< "$OPTARG" ;;  # iterations list
    t) IFS=' ' read -r -a THREADS <<< "$OPTARG" ;;          # thread list
    r) REPEATS="$OPTARG" ;;                                 # repeats
    *)
      echo "Usage: $0 -i \"10000000 20000000\" -t \"1 2 4 8\" -r 4"
      exit 1
      ;;
  esac
done

# ---------- CREATE RESULTS DIRECTORY ----------
mkdir -p "$OUTDIR"

echo "iterations,threads,run,mutex_time,rwlock_time,atomic_time" > "$RAW_CSV"

# ---------- RUN EXPERIMENTS ----------
for iters in "${ITERATIONS_LIST[@]}"; do
  for th in "${THREADS[@]}"; do
    echo "Running iterations=$iters threads=$th (repeats=$REPEATS)"

    for ((i=1; i<=REPEATS; i++)); do

      out="$("$BIN" "$iters" "$th" 2>&1)"

      mutex=$(echo "$out"  | awk '/Mutex approach time/ {print $4}')
      rwlock=$(echo "$out" | awk '/Read-Write Lock approach time/ {print $5}')
      atomic=$(echo "$out" | awk '/Atomic approach time/ {print $4}')

      printf "%s,%s,%d,%s,%s,%s\n" \
        "$iters" "$th" "$i" "$mutex" "$rwlock" "$atomic" >> "$RAW_CSV"
    done

  done
done

echo "Done. Raw results saved to: $RAW_CSV"
echo ""
echo "To plot results, run the following command:"
echo "python3 \"$BASEDIR/src/shared_var_plot.py\" \"$RAW_CSV\""
