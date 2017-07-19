clear
#mpiexec -n 5 ./Test 50 56
mpiexec -n 5 ./Test  10^201+1 | tee test.log
