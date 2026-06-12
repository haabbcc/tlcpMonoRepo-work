#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"

HOST="${TLS13_PQ_HOST:-127.0.0.1}"
PORT="${TLS13_PQ_PORT:-4453}"
CERT_DIR="${CERTS}/loose"
CA="${CERT_DIR}/ca_sm2.crt"

if ! "${OPENSSL}" s_client -help 2>&1 | grep -q -- "-enable_sm2_kyber768_tls13"; then
  echo "This Tongsuo build does not expose -enable_sm2_kyber768_tls13." >&2
  exit 1
fi

if [ ! -f "${CA}" ]; then
  echo "Missing CA file: ${CA}" >&2
  exit 1
fi

export TS_HYBRID_KEX_DEBUG="${TS_HYBRID_KEX_DEBUG:-1}"

printf 'GET / HTTP/1.0\r\n\r\n' | exec "${OPENSSL}" s_client \
  -connect "${HOST}:${PORT}" \
  -tls1_3 \
  -enable_sm_tls13_strict \
  -enable_sm2_kyber768_tls13 \
  -groups "SM2:KYBER768" \
  -ciphersuites "TLS_SM4_GCM_SM3" \
  -CAfile "${CA}" \
  -verify_return_error \
  -state \
  -tlsextdebug \
  -ign_eof
