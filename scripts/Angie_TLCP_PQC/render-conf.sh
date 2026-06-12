#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
source "${ROOT}/env.sh"

mkdir -p "${ROOT}/angie-work/logs" "${ROOT}/angie-work/client-body" "${ROOT}/angie-work/proxy"

sed "s|@TLCP_ROOT@|${ROOT}|g" \
  "${ROOT}/config/angie-tlcp-pqc.conf.in" \
  > "${ROOT}/angie-work/angie-tlcp-pqc.conf"

echo "${ROOT}/angie-work/angie-tlcp-pqc.conf"
