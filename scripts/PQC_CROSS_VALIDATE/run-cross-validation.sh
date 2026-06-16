#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"

LOG_DIR="${ROOT}/logs/cross-pqc"
mkdir -p "${LOG_DIR}" "${ROOT}/nginx-work/cross-pqc"
SUMMARY="${LOG_DIR}/summary.md"
TSV="${LOG_DIR}/summary.tsv"

printf 'Test ID\tDirection\tEndpoint\tExpected Group\tExpected Cipher\tResult\tEvidence\n' > "${TSV}"

record() {
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$@" >> "${TSV}"
}

stop_nginx() {
  if [ -f "${ROOT}/nginx-work/cross-pqc/nginx.pid" ]; then
    "${ROOT}/nginx-install/sbin/nginx" -p "${ROOT}/nginx-work/cross-pqc" \
      -c "${ROOT}/nginx-work/cross-pqc/nginx.conf" -s stop >/dev/null 2>&1 || true
  fi
}
trap stop_nginx EXIT

make_conf() {
  local group4433="$1"
  local group4434="$2"
  cat > "${ROOT}/nginx-work/cross-pqc/nginx.conf" <<CONF
worker_processes 1;
error_log ${ROOT}/nginx-work/cross-pqc/error.log debug;
pid ${ROOT}/nginx-work/cross-pqc/nginx.pid;
events { worker_connections 1024; }
http {
  server {
    listen 127.0.0.1:4433 ssl;
    server_name localhost;
    ssl_protocols TLSv1.3;
    ssl_certificate ${CERTS}/loose/sign_sm2.crt;
    ssl_certificate_key ${CERTS}/loose/sign_sm2.key;
    ssl_conf_command Ciphersuites TLS_AES_128_GCM_SHA256;
    ssl_conf_command Groups ${group4433};
    location / { return 200 "nginx tls13 x25519mlkem768 ok\n"; }
  }
  server {
    listen 127.0.0.1:4434 ssl;
    server_name localhost;
    ssl_protocols TLSv1.3;
    ssl_certificate ${CERTS}/loose/sign_sm2.crt;
    ssl_certificate_key ${CERTS}/loose/sign_sm2.key;
    ssl_conf_command Ciphersuites TLS_SM4_GCM_SM3;
    ssl_conf_command Groups ${group4434};
    location / { return 200 "nginx tls13 sm2mlkem768 ok\n"; }
  }
}
CONF
}

run_cmd_log() {
  local log="$1"
  shift
  set +e
  "$@" >"${log}" 2>&1
  local rc=$?
  set -e
  return "${rc}"
}

run_s_client() {
  local id="$1" endpoint="$2" group="$3" cipher="$4" log="$5"
  set +e
  printf 'GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n' | timeout 20 "${OPENSSL}" s_client \
    -connect "${endpoint}" \
    -servername localhost \
    -tls1_3 \
    -groups "${group}" \
    -ciphersuites "${cipher}" \
    -state -tlsextdebug -msg >"${log}" 2>&1
  local rc=$?
  set -e
  return "${rc}"
}

echo "== environment =="
bash "${ROOT}/scripts/PQC_CROSS_VALIDATE/inspect-env.sh"

echo "== strict target nginx config =="
make_conf "X25519MLKEM768" "curveSM2MLKEM768"
if run_cmd_log "${LOG_DIR}/local_nginx_strict_config_test.log" \
  "${ROOT}/nginx-install/sbin/nginx" -p "${ROOT}/nginx-work/cross-pqc" \
  -c "${ROOT}/nginx-work/cross-pqc/nginx.conf" -t; then
  strict_config_ok=1
else
  strict_config_ok=0
fi

if [ "${strict_config_ok}" -eq 1 ]; then
  stop_nginx
  "${ROOT}/nginx-install/sbin/nginx" -p "${ROOT}/nginx-work/cross-pqc" \
    -c "${ROOT}/nginx-work/cross-pqc/nginx.conf"

  if run_s_client L3 "127.0.0.1:4433" "X25519MLKEM768" "TLS_AES_128_GCM_SHA256" \
    "${LOG_DIR}/local_nginx_x25519mlkem768_s_client.log"; then
    if grep -q 'Cipher is TLS_AES_128_GCM_SHA256' "${LOG_DIR}/local_nginx_x25519mlkem768_s_client.log"; then
      record L3 "Tongsuo s_client -> local Nginx" "127.0.0.1:4433" "X25519MLKEM768 / 0x11EC" "TLS_AES_128_GCM_SHA256" PASS "logs/cross-pqc/local_nginx_x25519mlkem768_s_client.log"
    else
      record L3 "Tongsuo s_client -> local Nginx" "127.0.0.1:4433" "X25519MLKEM768 / 0x11EC" "TLS_AES_128_GCM_SHA256" FAIL "log lacks expected cipher"
    fi
  else
    record L3 "Tongsuo s_client -> local Nginx" "127.0.0.1:4433" "X25519MLKEM768 / 0x11EC" "TLS_AES_128_GCM_SHA256" FAIL "logs/cross-pqc/local_nginx_x25519mlkem768_s_client.log"
  fi

  if run_s_client L4 "127.0.0.1:4434" "curveSM2MLKEM768" "TLS_SM4_GCM_SM3" \
    "${LOG_DIR}/local_nginx_sm2mlkem768_s_client.log"; then
    if grep -q 'Cipher is TLS_SM4_GCM_SM3' "${LOG_DIR}/local_nginx_sm2mlkem768_s_client.log"; then
      record L4 "Tongsuo s_client -> local Nginx" "127.0.0.1:4434" "curveSM2MLKEM768 / 0x11EE" "TLS_SM4_GCM_SM3" PASS "logs/cross-pqc/local_nginx_sm2mlkem768_s_client.log"
    else
      record L4 "Tongsuo s_client -> local Nginx" "127.0.0.1:4434" "curveSM2MLKEM768 / 0x11EE" "TLS_SM4_GCM_SM3" FAIL "log lacks expected cipher"
    fi
  else
    record L4 "Tongsuo s_client -> local Nginx" "127.0.0.1:4434" "curveSM2MLKEM768 / 0x11EE" "TLS_SM4_GCM_SM3" FAIL "logs/cross-pqc/local_nginx_sm2mlkem768_s_client.log"
  fi
else
  cp "${LOG_DIR}/local_nginx_strict_config_test.log" "${LOG_DIR}/local_nginx_x25519mlkem768_s_client.log"
  cp "${LOG_DIR}/local_nginx_strict_config_test.log" "${LOG_DIR}/local_nginx_sm2mlkem768_s_client.log"
  record L3 "Tongsuo s_client -> local Nginx" "127.0.0.1:4433" "X25519MLKEM768 / 0x11EC" "TLS_AES_128_GCM_SHA256" CONFIG_FAIL "logs/cross-pqc/local_nginx_strict_config_test.log"
  record L4 "Tongsuo s_client -> local Nginx" "127.0.0.1:4434" "curveSM2MLKEM768 / 0x11EE" "TLS_SM4_GCM_SM3" CONFIG_FAIL "logs/cross-pqc/local_nginx_strict_config_test.log"
fi

echo "== fallback current Tongsuo SM2:KYBER768 local config =="
make_conf "X25519:prime256v1" "SM2:KYBER768"
if run_cmd_log "${LOG_DIR}/local_nginx_current_sm2_kyber768_config_test.log" \
  "${ROOT}/nginx-install/sbin/nginx" -p "${ROOT}/nginx-work/cross-pqc" \
  -c "${ROOT}/nginx-work/cross-pqc/nginx.conf" -t; then
  stop_nginx
  "${ROOT}/nginx-install/sbin/nginx" -p "${ROOT}/nginx-work/cross-pqc" \
    -c "${ROOT}/nginx-work/cross-pqc/nginx.conf"
  set +e
  printf 'GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n' | TS_HYBRID_KEX_DEBUG=1 timeout 20 "${OPENSSL}" s_client \
    -connect 127.0.0.1:4434 \
    -servername localhost \
    -tls1_3 \
    -enable_sm_tls13_strict \
    -enable_sm2_kyber768_tls13 \
    -groups "SM2:KYBER768" \
    -ciphersuites "TLS_SM4_GCM_SM3" \
    -CAfile "${CERTS}/loose/ca_sm2.crt" \
    -verify_return_error \
    -state -tlsextdebug -msg >"${LOG_DIR}/local_nginx_current_sm2_kyber768_s_client.log" 2>&1
  set -e
fi

echo "== remote ZoTrus tests =="
run_remote() {
  local id="$1" group="$2" cipher="$3" log="$4"
  set +e
  timeout 35 "${OPENSSL}" s_client \
    -connect www.zotrus.com:443 \
    -servername www.zotrus.com \
    -tls1_3 \
    -groups "${group}" \
    -ciphersuites "${cipher}" \
    -state -tlsextdebug -msg >"${log}" 2>&1
  local rc=$?
  set -e
  return "${rc}"
}

if run_remote R1 "curveSM2MLKEM768:X25519MLKEM768" "TLS_SM4_GCM_SM3:TLS_AES_128_GCM_SHA256" "${LOG_DIR}/zotrus_auto_pqc_s_client.log"; then
  record R1 "Tongsuo s_client -> ZoTrus" "www.zotrus.com:443" "auto" "auto" PASS "logs/cross-pqc/zotrus_auto_pqc_s_client.log"
else
  record R1 "Tongsuo s_client -> ZoTrus" "www.zotrus.com:443" "auto" "auto" FAIL "logs/cross-pqc/zotrus_auto_pqc_s_client.log"
fi

if run_remote R2 "curveSM2MLKEM768" "TLS_SM4_GCM_SM3" "${LOG_DIR}/zotrus_forced_sm2mlkem768_s_client.log"; then
  record R2 "Tongsuo s_client -> ZoTrus" "www.zotrus.com:443" "curveSM2MLKEM768 / 0x11EE" "TLS_SM4_GCM_SM3" PASS "logs/cross-pqc/zotrus_forced_sm2mlkem768_s_client.log"
else
  record R2 "Tongsuo s_client -> ZoTrus" "www.zotrus.com:443" "curveSM2MLKEM768 / 0x11EE" "TLS_SM4_GCM_SM3" FAIL "logs/cross-pqc/zotrus_forced_sm2mlkem768_s_client.log"
fi

if run_remote R3 "X25519MLKEM768" "TLS_AES_128_GCM_SHA256" "${LOG_DIR}/zotrus_forced_x25519mlkem768_s_client.log"; then
  record R3 "Tongsuo s_client -> ZoTrus" "www.zotrus.com:443" "X25519MLKEM768 / 0x11EC" "TLS_AES_128_GCM_SHA256" PASS "logs/cross-pqc/zotrus_forced_x25519mlkem768_s_client.log"
else
  record R3 "Tongsuo s_client -> ZoTrus" "www.zotrus.com:443" "X25519MLKEM768 / 0x11EC" "TLS_AES_128_GCM_SHA256" FAIL "logs/cross-pqc/zotrus_forced_x25519mlkem768_s_client.log"
fi

{
  echo "| Test ID | Direction | Endpoint | Expected Group | Expected Cipher | Result | Evidence |"
  echo "|---|---|---|---|---|---|---|"
  awk -F '\t' 'NR>1 { printf("| %s | %s | %s | %s | %s | %s | %s |\n", $1,$2,$3,$4,$5,$6,$7) }' "${TSV}"
  echo "| L1 | ZTBrowser -> local Nginx | localhost:4433 | X25519MLKEM768 / 0x11EC | TLS_AES_128_GCM_SHA256 | NOT_RUN | requires manual ZTBrowser UI capture after strict config support |"
  echo "| L2 | ZTBrowser -> local Nginx | localhost:4434 | curveSM2MLKEM768 / 0x11EE | TLS_SM4_GCM_SM3 | NOT_RUN | requires manual ZTBrowser UI capture after strict config support |"
} > "${SUMMARY}"

cat "${SUMMARY}"
echo "Logs: ${LOG_DIR}"
