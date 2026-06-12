#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
source "${ROOT}/env.sh"

ANGIE_PREFIX="${ANGIE_PREFIX:-${ROOT}/angie-install}"
ANGIE_BIN="${ANGIE_BIN:-${ANGIE_PREFIX}/sbin/angie}"
CONF="${ROOT}/angie-work/angie-tlcp-pqc.conf"

if [[ ! -x "${ANGIE_BIN}" ]]; then
  echo "Missing ${ANGIE_BIN}; run scripts/Angie_TLCP_PQC/build-angie.sh first." >&2
  exit 1
fi

if [[ ! -f "${CONF}" ]]; then
  bash "${ROOT}/scripts/Angie_TLCP_PQC/render-conf.sh" >/dev/null
fi

export LD_LIBRARY_PATH="${TONGSUO_ROOT}:${PQMAGIC_LIB}:${LD_LIBRARY_PATH:-}"
export OPENSSL_CONF="${ROOT}/config/openssl-providers.cnf"
export OPENSSL_MODULES="${PROVIDER_PQMAGIC_DIR}"

"${ANGIE_BIN}" -p "${ROOT}/angie-work" -c "${CONF}" -t
"${ANGIE_BIN}" -p "${ROOT}/angie-work" -c "${CONF}"

echo "Angie started on https://127.0.0.1:8443"
