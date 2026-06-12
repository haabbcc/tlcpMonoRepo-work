#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ANGIE_PREFIX="${ANGIE_PREFIX:-${ROOT}/angie-install}"
ANGIE_BIN="${ANGIE_BIN:-${ANGIE_PREFIX}/sbin/angie}"
CONF="${ROOT}/angie-work/angie-tlcp-pqc.conf"

if [[ -x "${ANGIE_BIN}" && -f "${CONF}" ]]; then
  "${ANGIE_BIN}" -p "${ROOT}/angie-work" -c "${CONF}" -s stop || true
fi
