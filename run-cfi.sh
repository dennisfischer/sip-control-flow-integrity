#!/usr/bin/env bash

set -euo pipefail

CLANG=clang-3.9
OPT=opt-3.9
LLVM_LINK=llvm-link-3.9

BUILD_DIR="$(pwd)/buildcfi/"
SRC_FILE=$1
LIB_PATH="build/control-flow-integrity/libControlFlowIntegrityPass.so"
LIB_HELPER_FILE="NewStackAnalysis.c"

SRC_FILE_BASE=$(basename "${SRC_FILE}")
OUT_BC_FILE=${BUILD_DIR}${SRC_FILE_BASE}
OUT_BC_PASS_FILE=${OUT_BC_FILE}"_cfi"
OUT_BC_LIB_HELPER_FILE=${BUILD_DIR}$(basename ${LIB_HELPER_FILE})
LINKED_FILE=${OUT_BC_PASS_FILE}"_linked"

function make_working_dir {
    if [ -d "$1" ]; then
        rm -r "$1"
    fi
    mkdir -p "$1"
}

function compile_to_bc {
    ${CLANG} -emit-llvm -c "$1" -o "$2" "${@:3:99}"
}

make_working_dir "${BUILD_DIR}"
compile_to_bc "${SRC_FILE}" "${OUT_BC_FILE}.bc"

${OPT} -load ${LIB_PATH} -control-flow-integrity < "${OUT_BC_FILE}.bc" > "${OUT_BC_PASS_FILE}.bc"

compile_to_bc ${LIB_HELPER_FILE} "${OUT_BC_LIB_HELPER_FILE}.bc"
${LLVM_LINK} "${OUT_BC_LIB_HELPER_FILE}.bc" "${OUT_BC_PASS_FILE}.bc" -o "${LINKED_FILE}.bc"
${CLANG} -g "${LINKED_FILE}.bc" -lm -lssl -lcrypto -o "${SRC_FILE_BASE%.*}"