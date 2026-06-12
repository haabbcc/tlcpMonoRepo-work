#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PORT="${TLS13_PQ_PORT:-4453}"
LOG_DIR="${TLS13_PQ_LOG_DIR:-${ROOT}/.tmp/tls13-sm2-kyber768}"
SERVER_LOG="${LOG_DIR}/server.log"
CLIENT_LOG="${LOG_DIR}/client.log"

mkdir -p "${LOG_DIR}"
rm -f "${SERVER_LOG}" "${CLIENT_LOG}"

source "${ROOT}/env.sh"

if ! "${OPENSSL}" s_client -help 2>&1 | grep -q -- "-enable_sm2_kyber768_tls13"; then
  echo "SKIP: this Tongsuo build does not expose -enable_sm2_kyber768_tls13." >&2
  exit 77
fi

TLS13_PQ_PORT="${PORT}" TS_HYBRID_KEX_DEBUG=1 \
  bash "${ROOT}/scripts/TLS13_SM2_KYBER768/server.sh" >"${SERVER_LOG}" 2>&1 &
SERVER_PID=$!

cleanup() {
  kill "${SERVER_PID}" >/dev/null 2>&1 || true
}
trap cleanup EXIT INT TERM

sleep 1
TLS13_PQ_PORT="${PORT}" TS_HYBRID_KEX_DEBUG=1 \
  bash "${ROOT}/scripts/TLS13_SM2_KYBER768/client.sh" >"${CLIENT_LOG}" 2>&1

sleep 1
cleanup
trap - EXIT INT TERM

cat "${SERVER_LOG}"
cat "${CLIENT_LOG}"

grep -Eq "TLSv1\.3|Protocol *: TLSv1\.3|Protocol version: TLSv1\.3" "${CLIENT_LOG}"
grep -q "TLS_SM4_GCM_SM3" "${CLIENT_LOG}"
grep -q "SM2KYBER-DBG" "${SERVER_LOG}"
grep -q "SM2KYBER-DBG" "${CLIENT_LOG}"
grep -Eq "kyber|Kyber|KYBER" "${SERVER_LOG}"
grep -Eq "kyber|Kyber|KYBER" "${CLIENT_LOG}"

echo "TLS 1.3 SM2+Kyber768 hybrid demo test ok"
