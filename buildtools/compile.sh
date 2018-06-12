#!/usr/bin/env bash

clang -emit-llvm -S ../examples/something.cpp -v
opt -load ../build/control-flow-integrity/libControlFlowIntegrityPass.so -control-flow-integrity < something.ll > something_pass.bc
opt -O3 < something_pass.bc > something_opt.bc

clang -g -c -emit-llvm /cfi/code/NewStackAnalysis.c -o NewStackAnalysis.bc -lssl -lcrypto
llvm-link NewStackAnalysis.bc something_opt.bc -o something_linked.bc
clang -g something_tmp.bc -lm -lssl -lcrypto -o something

# alternatively
# clang -Xclang -load -Xclang ../build/control-flow-integrity/libControlFlowIntegrityPass.so -c something.c
# cc -c /cfi/code/NewStackAnalysis.c
# lld something.o NewStackAnalysis.o