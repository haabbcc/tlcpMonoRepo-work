#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"

PORT="${TLS13_PQ_PORT:-4453}"
CERT_DIR="${CERTS}/loose"
CA="${CERT_DIR}/ca_sm2.crt"
CERT="${CERT_DIR}/sign_sm2.crt"
KEY="${CERT_DIR}/sign_sm2.key"

if ! "${OPENSSL}" s_server -help 2>&1 | grep -q -- "-enable_sm2_kyber768_tls13"; then
  echo "This Tongsuo build does not expose -enable_sm2_kyber768_tls13." >&2
  exit 1
fi

for f in "${CA}" "${CERT}" "${KEY}"; do
  if [ ! -f "${f}" ]; then
    echo "Missing certificate file: ${f}" >&2
    exit 1
  fi
done

export TS_HYBRID_KEX_DEBUG="${TS_HYBRID_KEX_DEBUG:-1}"

exec "${OPENSSL}" s_server \
  -accept "${PORT}" \
  -tls1_3 \
  -enable_sm_tls13_strict \
  -enable_sm2_kyber768_tls13 \
  -groups "SM2:KYBER768" \
  -ciphersuites "TLS_SM4_GCM_SM3" \
  -cert "${CERT}" \
  -key "${KEY}" \
  -CAfile "${CA}" \
  -verify_return_error \
  -state \
  -tlsextdebug \
  -www
