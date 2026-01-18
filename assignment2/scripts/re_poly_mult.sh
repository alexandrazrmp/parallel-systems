#!/bin/bash
set -euo pipefail

# Setup paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN="$BASEDIR/bin/poly_mult_omp"
OUTDIR="$BASEDIR/results"
CSV="$OUTDIR/results_2.1_omp.csv"

# Config
DEGREES=(10000 50000 100000 200000)
THREADS=(1 2 4 8)
REPEATS=4

if [ ! -f "$BIN" ]; then
    echo "Error: Binary missing at $BIN"
    exit 1
fi

mkdir -p "$OUTDIR"

# Init CSV
echo "degree,threads,run,serial_time,parallel_time,verification" > "$CSV"

echo "Starting OpenMP benchmarks..."

for n in "${DEGREES[@]}"; do
    for th in "${THREADS[@]}"; do
        for ((i=1; i<=REPEATS; i++)); do
            
            # Run
            echo "N=$n T=$th Run=$i"
            out=$("$BIN" "$n" "$th")

            # Parse
            t_ser=$(echo "$out" | grep "Serial multiplication time" | awk '{print $(NF-1)}')
            t_par=$(echo "$out" | grep "Parallel multiplication time" | awk '{print $(NF-1)}')
            check=$(echo "$out" | grep "Verification" | awk '{print $2}')

            # Save
            echo "$n,$th,$i,$t_ser,$t_par,$check" >> "$CSV"
        done
    done
done

echo "Done. Results in $CSV"