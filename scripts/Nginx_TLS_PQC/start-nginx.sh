#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"
NGINX_PREFIX="${NGINX_PREFIX:-${ROOT}/nginx-install}"
NGINX_BIN="${NGINX_BIN:-${NGINX_PREFIX}/sbin/nginx}"
CONF="${ROOT}/nginx-work/nginx.conf"

if [ ! -x "${NGINX_BIN}" ]; then
  echo "Missing ${NGINX_BIN}; run scripts/Nginx_TLS_PQC/build-nginx.sh first." >&2
  exit 1
fi

if [ ! -f "${CONF}" ]; then
  bash "${ROOT}/scripts/Nginx_TLS_PQC/render-conf.sh" >/dev/null
fi

"${NGINX_BIN}" -p "${ROOT}/nginx-work" -c "${CONF}" -t
"${NGINX_BIN}" -p "${ROOT}/nginx-work" -c "${CONF}"
echo "Nginx started on https://127.0.0.1:8443"
