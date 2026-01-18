#!/usr/bin/env bash
set -euo pipefail

# Setup paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN="$BASEDIR/bin/mergesort_omp"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/results_mergesort.csv"

# Config
SIZES=(10000000 100000000) # 10^7, 10^8
THREADS=(1 2 4 8 16)
REPEATS=3

if [ ! -f "$BIN" ]; then
    echo "Error: Binary not found at $BIN"
    exit 1
fi

mkdir -p "$OUTDIR"

# Init CSV
echo "N,algorithm,threads,run,execution_time,verification" > "$RAW_CSV"

echo "Starting benchmarks..."

for N in "${SIZES[@]}"; do
    
    echo "-------------------------------------"
    echo "Size: N=$N"
    echo "-------------------------------------"

    # Serial runs (Algo 0)
    echo "  > Serial..."
    
    for ((run=1; run<=REPEATS; run++)); do
        # Run: size, algo=0, threads=1
        out="$("$BIN" "$N" "0" "1")"
        
        t_exec=$(echo "$out" | grep "Execution time" | awk '{print $3}')
        check=$(echo "$out" | grep "Verification:" | awk '{print $2}')
        
        printf "%d,0,1,%d,%s,%s\n" "$N" "$run" "$t_exec" "$check" >> "$RAW_CSV"
        
        if [[ "$check" != "PASS" ]]; then
             echo "FAIL: Serial N=$N"
             exit 1
        fi
    done

    # Parallel runs (Algo 1)
    echo "  > Parallel..."
    
    for th in "${THREADS[@]}"; do
        for ((run=1; run<=REPEATS; run++)); do
            
            # Run: size, algo=1, threads
            out="$("$BIN" "$N" "1" "$th")"

            t_exec=$(echo "$out" | grep "Execution time" | awk '{print $3}')
            check=$(echo "$out" | grep "Verification:" | awk '{print $2}')

            printf "%d,1,%d,%d,%s,%s\n" "$N" "$th" "$run" "$t_exec" "$check" >> "$RAW_CSV"

            if [[ "$check" != "PASS" ]]; then
                echo "FAIL: Parallel N=$N Threads=$th"
                exit 1
            fi
        done
    done
done

echo ""
echo "Done. Results in $RAW_CSV"