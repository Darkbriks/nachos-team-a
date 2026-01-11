#!/bin/bash
# Test script for RingTest
# Usage: ./testRing.sh <num_machines>

NUM_MACHINES=${1:-3}
BUILD_DIR="../../build"

cd "$BUILD_DIR" || exit 1

echo "Testing ring with $NUM_MACHINES machines..."
echo ""

# First, launch all non-zero machines (they will wait to receive the token)
for i in $(seq 1 $((NUM_MACHINES - 1))); do
    NEXT=$((i + 1))
    if [ $i -eq $((NUM_MACHINES - 1)) ]; then
        NEXT=0
    fi

    echo "Launching machine $i (will forward token to machine $NEXT)..."
    # -m $i: machine ID
    # -g $i $NUM_MACHINES: run RingTest with machine ID and total number of machines
    ./nachos-step6 -m $i -g $i $NUM_MACHINES &
    sleep 0.2
done

# Give machines time to initialize their sockets
echo "Waiting for machines to initialize..."
sleep 1

# Now launch machine 0 (which will initiate the token)
echo "Launching machine 0 (will send initial token to machine 1)..."
./nachos-step6 -m 0 -g 0 $NUM_MACHINES &

echo ""
echo "Waiting for ring test to complete..."
# Wait for all background processes
wait

echo "Ring test completed!"
