#!/usr/bin/env bash
# Simple experiment driver for assignment1/bin/array_stats
# Usage: ./re_array_stats.sh -d "100000 1000000" -t "1 2 3 4 8" -r 5

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/array_stats"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/raw_results.csv"

# defaults
SIZES=(100000 1000000)
THREADS=(1 2 3 4 8)
REPEATS=5

while getopts "d:t:r:" opt; do
  case $opt in
    d) IFS=' ' read -r -a SIZES <<< "$OPTARG" ;;
    t) IFS=' ' read -r -a THREADS <<< "$OPTARG" ;;
    r) REPEATS="$OPTARG" ;;
    *) echo "Usage: $0 -d \"100000 1000000\" -t \"1 2 3 4 8\" -r 5"; exit 1 ;;
  esac
done

mkdir -p "$OUTDIR"
# header: size,threads,run,init_time,serial_time,parallel_time,verification
echo "size,threads,run,init_time,serial_time,parallel_time,verification" > "$RAW_CSV"

# build binary if missing
if [ ! -x "$BIN" ]; then
  echo "Binary not found at $BIN — running make..."
  (cd "$BASEDIR" && make array_stats)
fi

for n in "${SIZES[@]}"; do
  for th in "${THREADS[@]}"; do
    echo "Running size=$n threads=$th (repeats=$REPEATS)"
    for ((i=1;i<=REPEATS;i++)); do
      out="$("$BIN" "$n" "$th" 2>&1)"
      init=$(echo "$out" | grep -i "Initialization time" | awk '{print $(NF-1)}')
      serial=$(echo "$out" | grep -i "Serial execution time" | awk '{print $(NF-1)}')
      parallel=$(echo "$out" | grep -i "Parallel execution time" | awk '{print $(NF-1)}')
      ver=$(echo "$out" | grep -i "Verification" | awk '{print $2}')
      printf "%s,%s,%d,%s,%s,%s,%s\n" "$n" "$th" "$i" "$init" "$serial" "$parallel" "$ver" >> "$RAW_CSV"
    done
  done
done

echo "Done. Raw results: $RAW_CSV"
echo "To plot results use: python3 $BASEDIR/src/pr_array_stats.py $RAW_CSV"
