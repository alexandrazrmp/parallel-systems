#!/usr/bin/env bash
# Simple experiment driver for assignment1/bin/poly_mult
# Usage: ./run_experiments.sh -d "10000 100000" -t "1 2 4 8" -r 4

set -euo pipefail

# Resolve base directory (project assignment1 directory) relative to this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/poly_mult"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/raw_results.csv"

# default parameters
DEGREES=(10000 100000)
THREADS=(1 2 4 8)
REPEATS=4

while getopts "d:t:r:" opt; do
  case $opt in
    d) IFS=' ' read -r -a DEGREES <<< "$OPTARG" ;;
    t) IFS=' ' read -r -a THREADS <<< "$OPTARG" ;;
    r) REPEATS="$OPTARG" ;;
    *) echo "Usage: $0 -d \"100000 1000000\" -t \"1 2 4 8\" -r 4"; exit 1 ;;
  esac
done

mkdir -p "$OUTDIR"
echo "degree,threads,run,init_time,serial_time,parallel_time,verification" > "$RAW_CSV"

for n in "${DEGREES[@]}"; do
  for th in "${THREADS[@]}"; do
    echo "Running degree=$n threads=$th (repeats=$REPEATS)"
    for ((i=1;i<=REPEATS;i++)); do
      out="$("$BIN" "$n" "$th" 2>&1)"
      init=$(echo "$out" | grep -i "Initialization time" | awk '{print $(NF-1)}')
      serial=$(echo "$out" | grep -i "Serial multiplication time" | awk '{print $(NF-1)}')
      parallel=$(echo "$out" | grep -i "Parallel multiplication time" | awk '{print $(NF-1)}')
      ver=$(echo "$out" | grep -i "Verification" | awk '{print $2}')
      printf "%s,%s,%d,%s,%s,%s,%s\n" "$n" "$th" "$i" "$init" "$serial" "$parallel" "$ver" >> "$RAW_CSV"
    done
  done
done

echo "Done. Raw results: $RAW_CSV"
echo "To plot results use: python3 $BASEDIR/src/plot_results.py $RAW_CSV"