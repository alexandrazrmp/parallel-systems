#!/bin/bash
set -euo pipefail

# Setup
BIN="./mpi_poly"
CSV="experiment_results.csv"

# Config
SIZES="10000 50000 100000 200000"
PROCS="1 2 4 8"
ITERS=4

# Compile
echo "Compiling..."
mpicc src/poly_mult.c -o "$BIN"

# Init CSV
echo "N,Processes,Iteration,Send_Time,Comp_Time,Recv_Time,Total_Time,Status" > "$CSV"

unset DISPLAY
echo "Starting Single Node experiments..."

for n in $SIZES; do
    for p in $PROCS; do
        for ((i=1; i<=ITERS; i++)); do
            
            echo "N=$n P=$p Iter=$i"

            # Run with oversubscribe for single node
            out=$(mpirun --oversubscribe -np $p "$BIN" $n 2>/dev/null)

            # Parse
            t_send=$(echo "$out" | grep "Send time" | awk '{print $3}')
            t_comp=$(echo "$out" | grep "Comp time" | awk '{print $3}')
            t_recv=$(echo "$out" | grep "Recv time" | awk '{print $3}')
            total=$(echo "$out" | grep "Total" | awk '{print $2}')
            status=$(echo "$out" | grep "Result" | awk '{print $2}')

            if [ -n "$total" ]; then
                echo "$n,$p,$i,$t_send,$t_comp,$t_recv,$total,$status" >> "$CSV"
            else
                echo "Error on N=$n P=$p"
            fi
        done
    done
done

echo "Done. Results in $CSV"