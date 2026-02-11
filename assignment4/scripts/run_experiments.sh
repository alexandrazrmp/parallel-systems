#!/bin/bash
# Run experiments for Exercise 4.1 (Serial + SIMD)
# Logs timings and verification in a CSV file

set -euo pipefail

# Paths: assume script in scripts/, binaries in ../bin
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/poly_mult_simd"
OUTDIR="$BASEDIR/results"
CSV_FILE="$OUTDIR/results_4.1.csv"

# Polynomial degrees to test
DEGREES=(100000 250000 500000 750000 1000000)
REPEATS=4

# Check binary exists
if [ ! -f "$BIN" ]; then
    echo "Error: binary $BIN not found."
    exit 1
fi

mkdir -p "$OUTDIR"

# CSV header
echo "degree,run,init_time,serial_time,simd_time,verification" > "$CSV_FILE"

echo "=========================================="
echo "    Running Exercise 4.1 Experiments      "
echo "=========================================="

for n in "${DEGREES[@]}"; do
    echo "Running experiments for polynomial degree: $n"
    for ((i=1;i<=REPEATS;i++)); do
        echo "  Run $i of $REPEATS..."
        
        # Run program and capture output
        out="$("$BIN" "$n" 2>&1)"
        
        # Extract initialization time
        init=$(echo "$out" | grep -i "Initialization time" | awk '{print $(NF-1)}')
        # Extract serial multiplication time
        serial=$(echo "$out" | grep -i "Serial multiplication time" | awk '{print $(NF-1)}')
        # Extract SIMD multiplication time (or fallback scalar)
        simd=$(echo "$out" | grep -i "SIMD" | awk '{print $(NF-1)}')
        # Extract verification result
        ver=$(echo "$out" | grep -i "Verification" | awk '{print $2}')
        
        # Append to CSV
        printf "%s,%d,%s,%s,%s,%s\n" "$n" "$i" "$init" "$serial" "$simd" "$ver" >> "$CSV_FILE"
    done
done

echo "All experiments completed. Results saved in: $CSV_FILE"
