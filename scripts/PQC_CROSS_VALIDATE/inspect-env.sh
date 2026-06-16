#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"
LOG_DIR="${ROOT}/logs/cross-pqc"
mkdir -p "${LOG_DIR}"

{
  echo "OPENSSL=${OPENSSL}"
  echo "OPENSSL_CONF=${OPENSSL_CONF:-}"
  echo "OPENSSL_MODULES=${OPENSSL_MODULES:-}"
  "${OPENSSL}" version -a
  "${OPENSSL}" list -providers || true
  "${OPENSSL}" ciphers -s -v -tls1_3 | grep -Ei 'SM4|AES' || true
  "${OPENSSL}" ciphers -V ALL | grep -Ei 'MLKEM|KYBER|SM2|SM4|AES_128_GCM' || true
  "${ROOT}/nginx-install/sbin/nginx" -V 2>&1 || true
  ldd "${ROOT}/nginx-install/sbin/nginx" | grep -E 'ssl|crypto' || true
} | tee "${LOG_DIR}/environment.log"
