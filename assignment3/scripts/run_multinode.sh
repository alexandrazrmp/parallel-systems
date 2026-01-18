#!/bin/bash

# MPICH paths
MPICC="/usr/local/mpich3/bin/mpicc"
MPIEXEC="/usr/local/mpich3/bin/mpiexec"

# compile program
echo "Compiling..."
$MPICC src/poly_mult.c -o mpi_poly
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# csv output file (put in results/)
mkdir -p results
CSV_FILE="results/multinode_results_final.csv"
echo "N,Nodes,Iteration,Send_Time,Comp_Time,Recv_Time,Total_Time,Status" > "$CSV_FILE"

unset DISPLAY
# use absolute path for the binary
CURRENT_DIR=$(pwd)
EXEC_PATH="$CURRENT_DIR/mpi_poly"

# parameters
SIZES="10000 50000 100000 200000"
NODES="2 4 8 16 32 64"
ITERS=4

# run experiments
for n in $SIZES; do
    for p in $NODES; do
        for ((i=1; i<=ITERS; i++)); do
            echo "Running: N=$n | Processes=$p | Iter=$i..."


            # run mpiexec and capture output (capture stderr)
            OUTPUT=$($MPIEXEC -f machines -n $p "$EXEC_PATH" $n 2>&1)

            # get Total Execution Time (number before 'sec')
            TOTAL=$(echo "$OUTPUT" | grep -i "Total Execution Time" | awk '{print $(NF-1)}')

            if [ -n "$TOTAL" ]; then
                # extract other metrics
                SEND=$(echo "$OUTPUT" | grep -i "Send Time" | awk '{print $(NF-1)}')
                COMP=$(echo "$OUTPUT" | grep -i "Computation Time" | awk '{print $(NF-1)}')
                RECV=$(echo "$OUTPUT" | grep -i "Recv Time" | awk '{print $(NF-1)}')

                # extract verification status (PASS/FAIL)
                STATUS=$(echo "$OUTPUT" | grep -i "Verification Status" | awk '{print $NF}')

                echo "  -> $STATUS: Time=$TOTAL sec"
                echo "$n,$p,$i,$SEND,$COMP,$RECV,$TOTAL,$STATUS" >> "$CSV_FILE"
            else
                # total missing -> print error output
                echo "  -> FAILED."
                echo "---------------- ERROR OUTPUT ----------------"
                echo "$OUTPUT"
                echo "----------------------------------------------"
            fi
        done
    done
    echo "---------------------------------------------"
done

echo "Done! Results saved in $CSV_FILE"