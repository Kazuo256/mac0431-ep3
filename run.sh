#!/bin/bash

mpiexec.hydra -f /mnt/data/hostfile -np 1 ./src/ep3 $1

