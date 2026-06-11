#!/bin/sh
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "${SCRIPT_DIR}/../.." && pwd)

LIB_DIR=${TONGSUO_LIB_DIR:-"${REPO_ROOT}/tongsuo"}
INC_DIR=${TONGSUO_INC_DIR:-"${REPO_ROOT}/tongsuo/include"}
CERT_OPENSSL_BIN=${CERT_OPENSSL_BIN:-openssl}

#LIB_DIR=../gmtls/lib
#INC_DIR=../gmtls/include

cd "${SCRIPT_DIR}"

mkdir -p "${REPO_ROOT}/demo/certs"
if [ ! -f "${REPO_ROOT}/demo/certs/tls_ca.crt" ]; then
	echo "Generating TLS test certificates under ${REPO_ROOT}/demo/certs"
	{
	"${CERT_OPENSSL_BIN}" req -x509 -newkey rsa:2048 -nodes \
		-subj "/CN=tlcp-demo-tls-ca" \
		-keyout "${REPO_ROOT}/demo/certs/tls_ca.key" \
		-out "${REPO_ROOT}/demo/certs/tls_ca.crt" \
		-days 3650
	"${CERT_OPENSSL_BIN}" req -newkey rsa:2048 -nodes \
		-subj "/CN=tlcp-demo-tls-server" \
		-keyout "${REPO_ROOT}/demo/certs/tls_server.key" \
		-out "${REPO_ROOT}/demo/certs/tls_server.csr"
	"${CERT_OPENSSL_BIN}" x509 -req \
		-in "${REPO_ROOT}/demo/certs/tls_server.csr" \
		-CA "${REPO_ROOT}/demo/certs/tls_ca.crt" \
		-CAkey "${REPO_ROOT}/demo/certs/tls_ca.key" \
		-CAcreateserial \
		-out "${REPO_ROOT}/demo/certs/tls_server.crt" \
		-days 3650
	"${CERT_OPENSSL_BIN}" req -newkey rsa:2048 -nodes \
		-subj "/CN=tlcp-demo-tls-client" \
		-keyout "${REPO_ROOT}/demo/certs/tls_client.key" \
		-out "${REPO_ROOT}/demo/certs/tls_client.csr"
	"${CERT_OPENSSL_BIN}" x509 -req \
		-in "${REPO_ROOT}/demo/certs/tls_client.csr" \
		-CA "${REPO_ROOT}/demo/certs/tls_ca.crt" \
		-CAkey "${REPO_ROOT}/demo/certs/tls_ca.key" \
		-CAcreateserial \
		-out "${REPO_ROOT}/demo/certs/tls_client.crt" \
		-days 3650
	} >/dev/null 2>&1
fi

gcc -ggdb3 -O0 -o server server.c -I${INC_DIR}  -L${LIB_DIR} -lssl -lcrypto -ldl -lpthread -Wl,-rpath=${LIB_DIR}
gcc -ggdb3 -O0 -o client client.c -I${INC_DIR}  -L${LIB_DIR} -lssl -lcrypto -ldl -lpthread -Wl,-rpath=${LIB_DIR}
gcc -ggdb3 -O0 -o server_tls server_tls.c -I${INC_DIR}  -L${LIB_DIR} -lssl -lcrypto -ldl -lpthread -Wl,-rpath=${LIB_DIR}
gcc -ggdb3 -O0 -o client_tls client_tls.c -I${INC_DIR}  -L${LIB_DIR} -lssl -lcrypto -ldl -lpthread -Wl,-rpath=${LIB_DIR}
#gcc -ggdb3 -O0 -o server_P server_P.c -I${INC_DIR} -I${GM_INC_DIR} -L${LIB_DIR} -lssl -lcrypto -ldl -lpthread -Wl,-rpath=${LIB_DIR}
