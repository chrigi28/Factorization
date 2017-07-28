#!/bin/bash
#mpirun -n $1 ./pi $2
#$1 hostfile $2 No prozess $3 iteration
mpiexec -f $1 -n $2 ./Factorization $3
