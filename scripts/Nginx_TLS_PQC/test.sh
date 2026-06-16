#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
bash "${ROOT}/scripts/Nginx_TLS_PQC/render-conf.sh" >/dev/null
bash "${ROOT}/scripts/Nginx_TLS_PQC/stop-nginx.sh" >/dev/null 2>&1 || true
bash "${ROOT}/scripts/Nginx_TLS_PQC/start-nginx.sh"
trap 'bash "${ROOT}/scripts/Nginx_TLS_PQC/stop-nginx.sh" >/dev/null 2>&1 || true' EXIT
sleep 1
bash "${ROOT}/scripts/Nginx_TLS_PQC/test-client.sh" | tee "${ROOT}/nginx-work/test-client.out"
grep -q "nginx_tls_pqc_ok" "${ROOT}/nginx-work/test-client.out"
echo "Nginx TLS test ok"
