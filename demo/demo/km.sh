#!/bin/sh
LIB_DIR=./lib
INC_DIR=./include
GM_INC_DIR=./include

gcc -ggdb3 -O0 -o server server.c -I${INC_DIR} -I${GM_INC_DIR} -L${LIB_DIR} -lssl -lcrypto -lgm_cipher -ldl -lpthread -Wl,-rpath=${LIB_DIR}
gcc -ggdb3 -O0 -o client client.c -I${INC_DIR} -I${GM_INC_DIR} -L${LIB_DIR} -lssl -lcrypto -lgm_cipher -ldl -lpthread -Wl,-rpath=${LIB_DIR}
gcc -ggdb3 -O0 -o server_tls server_tls.c -I${INC_DIR} -I${GM_INC_DIR} -L${LIB_DIR} -lssl -lcrypto -lgm_cipher -ldl -lpthread -Wl,-rpath=${LIB_DIR}
#gcc -ggdb3 -O0 -o client client.c -I${INC_DIR} -I${GM_INC_DIR} ${LIB_DIR}/libssl.a -L${LIB_DIR} -lcrypto -lSKF_final -lhik_skfm -lgm_cipher -lSecIAC -ldl -lpthread -Wl,-rpath=${LIB_DIR}
