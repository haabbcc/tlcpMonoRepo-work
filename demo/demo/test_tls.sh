#!/bin/sh
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "${SCRIPT_DIR}"

sh ./mk.sh

./server_tls > tls_server.log 2>&1 &
SERVER_PID=$!

cleanup() {
	kill "${SERVER_PID}" >/dev/null 2>&1 || true
}
trap cleanup EXIT INT TERM

sleep 1
./client_tls > tls_client.log 2>&1
wait "${SERVER_PID}" || true
trap - EXIT INT TERM

cat tls_server.log
cat tls_client.log

grep -q "TLS server handshake ok" tls_server.log
grep -q "TLS client handshake ok" tls_client.log
grep -q "TLS recv: hello from tls server" tls_client.log

echo "TLS interface test ok"
