#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
source "${ROOT}/env.sh"

CERT="${CERTS}/loose"

export LD_LIBRARY_PATH="${TONGSUO_ROOT}:${PQMAGIC_LIB}:${LD_LIBRARY_PATH:-}"
export OPENSSL_CONF="${ROOT}/config/openssl-providers.cnf"
export OPENSSL_MODULES="${PROVIDER_PQMAGIC_DIR}"

exec "${OPENSSL}" s_client -connect 127.0.0.1:8443 \
  -ntls -enable_ntls \
  -sign_cert "${CERT}/sign_sm2.crt" \
  -sign_key "${CERT}/sign_sm2.key" \
  -enc_cert "${CERT}/enc_sm2.crt" \
  -enc_key "${CERT}/enc_sm2.key" \
  -CAfile "${CERT}/ca_sm2.crt" \
  -cipher 'ECC-KYBER-SM4-GCM-SM3'
