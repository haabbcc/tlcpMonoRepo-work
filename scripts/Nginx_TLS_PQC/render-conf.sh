#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
mkdir -p "${ROOT}/nginx-work/logs" "${ROOT}/nginx-work/client_body_temp" "${ROOT}/nginx-work/proxy_temp"
sed "s|@TLCP_ROOT@|${ROOT}|g" \
  "${ROOT}/config/nginx-tls-pqc.conf.in" \
  > "${ROOT}/nginx-work/nginx.conf"
echo "${ROOT}/nginx-work/nginx.conf"
