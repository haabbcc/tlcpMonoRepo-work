#!/bin/sh
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "${SCRIPT_DIR}/../.." && pwd)

LIB_DIR=${TONGSUO_LIB_DIR:-"${REPO_ROOT}/tongsuo"}
INC_DIR=${TONGSUO_INC_DIR:-"${REPO_ROOT}/tongsuo/include"}

#LIB_DIR=../gmtls/lib
#INC_DIR=../gmtls/include

cd "${SCRIPT_DIR}"

gcc -ggdb3 -O0 -o server server.c -I${INC_DIR}  -L${LIB_DIR} -lssl -lcrypto -ldl -lpthread -Wl,-rpath=${LIB_DIR}
gcc -ggdb3 -O0 -o client client.c -I${INC_DIR}  -L${LIB_DIR} -lssl -lcrypto -ldl -lpthread -Wl,-rpath=${LIB_DIR}
#gcc -ggdb3 -O0 -o server_P server_P.c -I${INC_DIR} -I${GM_INC_DIR} -L${LIB_DIR} -lssl -lcrypto -ldl -lpthread -Wl,-rpath=${LIB_DIR}
