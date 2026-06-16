#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"
NGINX_PREFIX="${NGINX_PREFIX:-${ROOT}/nginx-install}"
NGINX_BIN="${NGINX_BIN:-${NGINX_PREFIX}/sbin/nginx}"
CONF="${ROOT}/nginx-work/nginx.conf"

if [ -x "${NGINX_BIN}" ] && [ -f "${CONF}" ]; then
  "${NGINX_BIN}" -p "${ROOT}/nginx-work" -c "${CONF}" -s stop || true
fi
