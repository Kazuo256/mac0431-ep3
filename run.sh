#!/bin/bash

rm out-*
mpiexec.hydra -f /mnt/data/hostfile -np $1 ./src/ep3 $2
cat out-*

