#!/usr/bin/env bash

usage () {
	printf "Usage: $0 <llvm bitcode file> <desired path to new binary> <sensitive function list>\n"
	printf "Example: '$0 ../something.bc ./something'\n"
	exit 1
}

if [ $# -ne 3 ]
then
	printf "Wrong number of parameters\n"
	usage
fi

opt-5.0 -load control-flow-integrity/libControlFlowIntegrityPass.so -i $3 -functionpass < $1 > something_pass.bc
opt-5.0 -O3 < something_pass.bc > something_opt.bc
#llc-3.9 something_opt.bc
#gcc -c something_opt.s -o something_opt.o
#gcc -O1 -c ../control-flow-integrity/NewStackAnalysis.c -o StackAnalysis.o -lssl -lcrypto
#gcc -O1 something_opt.o StackAnalysis.o -o $2 -lssl -lcrypto


clang-5.0 -g -c -emit-llvm /cfi/code/NewStackAnalysis.c -o NewStackAnalysis.bc -lssl -lcrypto
llvm-link-5.0 NewStackAnalysis.bc something_opt.bc -o something_tmp.bc
clang-5.0 -g something_tmp.bc -lm -lncurses -o $2 -lssl -lcrypto
