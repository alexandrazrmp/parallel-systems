#!/bin/bash
# Run experiments for Exercise 1.1 (Pthreads / sequential).
# Uses the same parameters as the MPI version so results are comparable.

set -euo pipefail

# Paths: assume this script lives in a scripts/ folder and the
# compiled binaries are in the parent/bin directory.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Paths to the binary, output dir and CSV file
BIN="$BASEDIR/bin/poly_mult"
OUTDIR="$BASEDIR/results"
CSV_FILE="$OUTDIR/results_1.1.csv"

# Experiment parameters (kept in sync with the MPI tests)
DEGREES=(10000 50000 100000 200000)
THREADS=(1 2 4 8)  # 1 = sequential baseline
REPEATS=4

# Check binary
if [ ! -f "$BIN" ]; then
  echo "Error: binary $BIN not found."
  exit 1
fi

mkdir -p "$OUTDIR"
# CSV header
echo "degree,threads,run,init_time,serial_time,parallel_time,verification" > "$CSV_FILE"

echo "=========================================="
echo "    Running Exercise 1.1 Experiments      "
echo "=========================================="

for n in "${DEGREES[@]}"; do
  for th in "${THREADS[@]}"; do
    echo "Running: N=$n | Threads=$th..."
    for ((i=1;i<=REPEATS;i++)); do

      # Run the program and capture its output
      out="$("$BIN" "$n" "$th" 2>&1)"

      # Extract timings and verification from the program output
      init=$(echo "$out" | grep -i "Initialization time" | awk '{print $(NF-1)}')
      serial=$(echo "$out" | grep -i "Serial multiplication time" | awk '{print $(NF-1)}')
      parallel=$(echo "$out" | grep -i "Parallel multiplication time" | awk '{print $(NF-1)}')
      ver=$(echo "$out" | grep -i "Verification" | awk '{print $2}')

      # Append one CSV row
      printf "%s,%s,%d,%s,%s,%s,%s\n" "$n" "$th" "$i" "$init" "$serial" "$parallel" "$ver" >> "$CSV_FILE"
    done
  done
done

echo "Done! Results saved in: $CSV_FILE"