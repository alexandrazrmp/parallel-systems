#!/bin/bash
set -euo pipefail

# Setup
MPICC="/usr/local/mpich3/bin/mpicc"
MPIEXEC="/usr/local/mpich3/bin/mpiexec"
BIN="./mpi_poly"
CSV="multinode_results_final.csv"

# Config
SIZES="10000 50000 100000 200000"
NODES="2 4 8 16 32 64"
ITERS=4

# Compile
echo "Compiling..."
$MPICC src/poly_mult.c -o "$BIN"

# Init CSV
echo "N,Nodes,Iteration,Send_Time,Comp_Time,Recv_Time,Total_Time,Status" > "$CSV"

unset DISPLAY
cwd=$(pwd)

echo "Starting MPI benchmarks..."

for n in $SIZES; do
    for p in $NODES; do
        for ((i=1; i<=ITERS; i++)); do
            
            echo "N=$n P=$p Iter=$i"

            # Run with hostfile
            # Note: Using full path for binary is safer in mpiexec
            out=$($MPIEXEC -f machines -n $p "$cwd/mpi_poly" $n)

            # Parse metrics
            total=$(echo "$out" | grep "Total" | awk '{print $2}')

            if [ -n "$total" ]; then
                t_send=$(echo "$out" | grep "Send time" | awk '{print $3}')
                t_comp=$(echo "$out" | grep "Comp time" | awk '{print $3}')
                t_recv=$(echo "$out" | grep "Recv time" | awk '{print $3}')
                status=$(echo "$out" | grep "Result" | awk '{print $2}')

                echo "$n,$p,$i,$t_send,$t_comp,$t_recv,$total,$status" >> "$CSV"
            else
                echo "FAIL on N=$n P=$p"
                echo "$out"
            fi
        done
    done
done

echo "Done. Results in $CSV"