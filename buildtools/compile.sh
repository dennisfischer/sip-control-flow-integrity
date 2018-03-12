#!/usr/bin/env bash

clang-5.0 -emit-llvm -S ../examples/something.c -v
opt-5.0 -load ../build/control-flow-integrity/libControlFlowIntegrityPass.so -control_flow_integrity < something.ll > something_pass.bc
opt-5.0 -O3 < something_pass.bc > something_opt.bc

clang-5.0 -g -c -emit-llvm /cfi/code/NewStackAnalysis.c -o NewStackAnalysis.bc -lssl -lcrypto
llvm-link-5.0 NewStackAnalysis.bc something_opt.bc -o something_linked.bc
clang-5.0 -g something_tmp.bc -lm -lssl -lcrypto -o something
