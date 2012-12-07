#!/bin/bash

mpiexec.hydra -f /mnt/data/hostfile -np 2 ./src/ep3 $1

