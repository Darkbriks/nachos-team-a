#!/bin/bash
# Test simultaneous launch of two NachOS machines
# Both machines are launched at the same time

cd /home/alioune/M1/s2/team-a/code/build || exit 1

echo "=========================================="
echo "NachOS Simultaneous Network Test"
echo "=========================================="
echo ""
echo "Launching both machines simultaneously..."
echo ""

# Launch both machines at the same time
timeout 15 ./nachos-step6 -m 0 -l 1.0 -R 1 2>&1 & M0_PID=$!
timeout 15 ./nachos-step6 -m 1 -l 1.0 -R 0 2>&1 & M1_PID=$!

echo "Machine 0 PID: $M0_PID"
echo "Machine 1 PID: $M1_PID"
echo ""

# Wait 10 seconds to monitor progress
sleep 10

echo ""
echo "=== Status après 10 secondes ==="
echo ""

ps -p $M0_PID,$M1_PID 2>/dev/null || echo "Processus terminés"

echo ""
echo "Waiting for processes to complete..."
echo ""

# Wait for both processes to finish
wait

echo ""
echo "=========================================="
echo "Test completed!"
echo "=========================================="
