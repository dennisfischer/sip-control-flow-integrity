#!/usr/bin/env bash

opt-5.0 -load code/libFunctionPass.so -functionpass < ../something.bc > something_pass.bc
opt-5.0 -O3 < something_pass.bc > something_opt.bc
llc-5.0 something_opt.bc
gcc -c something_opt.s -o something_opt.o
gcc -c ../code/NewStackAnalysis.c -o StackAnalysis.o
gcc something_opt.o StackAnalysis.o -o something
./something
