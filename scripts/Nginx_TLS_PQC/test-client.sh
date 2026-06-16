#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"
HOST="${NGINX_TLS_HOST:-127.0.0.1}"
PORT="${NGINX_TLS_PORT:-8443}"
CA="${CERTS}/loose/ca_sm2.crt"

printf 'GET / HTTP/1.0\r\nHost: localhost\r\n\r\n' | "${OPENSSL}" s_client \
  -connect "${HOST}:${PORT}" \
  -tls1_3 \
  -groups "SM2:KYBER768" \
  -ciphersuites "TLS_SM4_GCM_SM3" \
  -CAfile "${CA}" \
  -verify_return_error \
  -ign_eof
