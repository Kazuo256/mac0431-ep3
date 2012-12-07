#!/bin/bash

rm -f log-*
rm -f answer
mpiexec.hydra -f /mnt/data/hostfile -np $1 ./src/ep3 $2
cat log-*
echo "##############"
echo "The answer is:"
cat answer
echo "##############"

