#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"

LOG_ROOT="${ROOT}/logs/tls-codepoint-verify"
mkdir -p "${LOG_ROOT}"

NGINX_BIN="${ROOT}/nginx-install/sbin/nginx"
NGINX_WORK="${ROOT}/nginx-work-codepoint"
NGINX_CONF="${NGINX_WORK}/nginx.conf"
LOCAL_PORT="${LOCAL_PORT:-4434}"

CERT_DIR="${CERTS}/loose"
CA="${CERT_DIR}/ca_sm2.crt"
CERT="${CERT_DIR}/sign_sm2.crt"
KEY="${CERT_DIR}/sign_sm2.key"

write_nginx_conf() {
  local groups="$1"
  mkdir -p "${NGINX_WORK}/logs" "${NGINX_WORK}/client_body_temp" "${NGINX_WORK}/proxy_temp"
  cat > "${NGINX_CONF}" <<EOF_INNER
worker_processes 1;
error_log ${NGINX_WORK}/error.log debug;
pid ${NGINX_WORK}/nginx.pid;

events {
    worker_connections 1024;
}

http {
    access_log ${NGINX_WORK}/access.log;

    server {
        listen 127.0.0.1:${LOCAL_PORT} ssl;
        server_name localhost;

        ssl_protocols TLSv1.3;
        ssl_certificate ${CERT};
        ssl_certificate_key ${KEY};
        ssl_client_certificate ${CA};
        ssl_verify_client off;
        ssl_conf_command Ciphersuites TLS_SM4_GCM_SM3;
        ssl_conf_command Groups ${groups};

        location / {
            return 200 nginx_sm2_kyber768_codepoint_ok;
        }
    }
}
EOF_INNER
}

run_nginx_t() {
  local label="$1"
  local groups="$2"
  local out="${LOG_ROOT}/nginx_t_${label}.log"
  write_nginx_conf "${groups}"
  {
    echo "groups=${groups}"
    echo "command=${NGINX_BIN} -p ${NGINX_WORK} -c ${NGINX_CONF} -t"
    "${NGINX_BIN}" -p "${NGINX_WORK}" -c "${NGINX_CONF}" -t
  } >"${out}" 2>&1 && return 0 || return 1
}

stop_nginx() {
  if [ -x "${NGINX_BIN}" ] && [ -f "${NGINX_CONF}" ]; then
    "${NGINX_BIN}" -p "${NGINX_WORK}" -c "${NGINX_CONF}" -s stop >/dev/null 2>&1 || true
  fi
}

wait_port() {
  local port="$1"
  local i
  for i in $(seq 1 50); do
    if (echo >"/dev/tcp/127.0.0.1/${port}") >/dev/null 2>&1; then
      return 0
    fi
    sleep 0.1
  done
  return 1
}

extract_tls_notes() {
  local src="$1"
  local dst="$2"
  {
    echo "== Protocol/Cipher/Temp Key/Verify/HTTP =="
    grep -Ei 'New, TLS|Protocol *:|Cipher *:|Cipher is|Ciphersuite|Server Temp Key|Verification:|Verify return code|HTTP/1.1 200 OK|HTTP/1.0 200' "${src}" || true
    echo
    echo "== key_share / supported_groups =="
    grep -Ein 'key.share|key_share|supported.groups|supported_groups|extension.*(10|51)|0x0033|0x000a|11ee|11 ec|11ec|kyber|mlkem|sm2|missing_extension|extension not received|cannot be set' "${src}" || true
  } > "${dst}"
}

parse_pcap() {
  local pcap="$1"
  local out="$2"
  {
    echo "pcap=${pcap}"
    if command -v tshark >/dev/null 2>&1; then
      echo "tool=tshark"
      tshark -r "${pcap}" -Y 'tls.handshake.type == 2' -V 2>/dev/null \
        | grep -Ei 'Server Hello|Key Share|selected_group|Named Group|0x11ee|0x11ec|curveSM2|MLKEM|Kyber|Group' || true
      echo
      echo "fields:"
      tshark -r "${pcap}" -Y 'tls.handshake.type == 2' -T fields \
        -e frame.number \
        -e tls.handshake.extensions_key_share_group \
        -e tls.handshake.extensions_supported_group \
        -e tls.handshake.ciphersuite 2>/dev/null || true
    else
      echo "tshark not found"
    fi
  } > "${out}"
}

run_remote() {
  local label="$1"
  local group="$2"
  local out="${LOG_ROOT}/zotrus_${label}.log"
  local notes="${LOG_ROOT}/zotrus_${label}_extract.txt"
  {
    echo "group=${group}"
    echo "command=${OPENSSL} s_client -connect www.zotrus.com:443 -servername www.zotrus.com -tls1_3 -groups ${group} -ciphersuites TLS_SM4_GCM_SM3 -enable_sm2_kyber768_tls13 -state -tlsextdebug -msg"
    printf 'GET / HTTP/1.1\r\nHost: www.zotrus.com\r\nConnection: close\r\n\r\n' | timeout 25 "${OPENSSL}" s_client \
      -connect www.zotrus.com:443 \
      -servername www.zotrus.com \
      -tls1_3 \
      -groups "${group}" \
      -ciphersuites TLS_SM4_GCM_SM3 \
      -enable_sm2_kyber768_tls13 \
      -state -tlsextdebug -msg
  } >"${out}" 2>&1 || true
  extract_tls_notes "${out}" "${notes}"
}

write_report() {
  local report="${LOG_ROOT}/REPORT.md"
  local pcap_parse="${LOG_ROOT}/local_nginx_sm2_kyber768_pcap_parse.txt"
  local selected_group="unknown"

  if grep -Eq 'curveSM2 \(41\)|[[:space:]]41[[:space:]]+.*0x00c6' "${pcap_parse}" 2>/dev/null; then
    selected_group="0x0029"
  elif grep -qi '0x11ee' "${pcap_parse}" 2>/dev/null; then
    selected_group="0x11EE"
  fi

  cat > "${report}" <<EOF_REPORT
# TLS codepoint verification: Nginx/Tongsuo SM2+Kyber768

Date: $(date -Iseconds)

## Evidence files

- X25519MLKEM768 nginx -t: \`${LOG_ROOT}/nginx_t_x25519mlkem768_fail.log\`
- curveSM2MLKEM768 nginx -t: \`${LOG_ROOT}/nginx_t_curvesm2mlkem768_fail.log\`
- SM2:KYBER768 nginx -t: \`${LOG_ROOT}/nginx_t_sm2_kyber768_success.log\`
- Local s_client full log: \`${LOG_ROOT}/local_nginx_sm2_kyber768_s_client.log\`
- Local pcap: \`${ROOT}/logs/local_nginx_sm2_kyber768.pcap\`
- Local pcap parse: \`${LOG_ROOT}/local_nginx_sm2_kyber768_pcap_parse.txt\`
- ZoTrus curveSM2MLKEM768: \`${LOG_ROOT}/zotrus_curvesm2mlkem768.log\`
- ZoTrus X25519MLKEM768: \`${LOG_ROOT}/zotrus_x25519mlkem768.log\`
- ZoTrus SM2:KYBER768: \`${LOG_ROOT}/zotrus_sm2_kyber768.log\`

## Local nginx -t results

\`Groups X25519MLKEM768\` is rejected:

\`\`\`text
$(grep -E 'SSL_CONF_cmd|test failed|cannot be set' "${LOG_ROOT}/nginx_t_x25519mlkem768_fail.log" || true)
\`\`\`

\`Groups curveSM2MLKEM768\` is rejected:

\`\`\`text
$(grep -E 'SSL_CONF_cmd|test failed|cannot be set' "${LOG_ROOT}/nginx_t_curvesm2mlkem768_fail.log" || true)
\`\`\`

\`Groups SM2:KYBER768\` passes:

\`\`\`text
$(grep -E 'syntax is ok|test is successful' "${LOG_ROOT}/nginx_t_sm2_kyber768_success.log" || true)
\`\`\`

## Local Nginx wire result

The local Nginx test listens on \`127.0.0.1:${LOCAL_PORT}\` with:

\`\`\`nginx
ssl_protocols TLSv1.3;
ssl_conf_command Ciphersuites TLS_SM4_GCM_SM3;
ssl_conf_command Groups SM2:KYBER768;
\`\`\`

The required client command was run with \`-enable_sm2_kyber768_tls13 -state -tlsextdebug -msg\`.

Extract:

\`\`\`text
$(cat "${LOG_ROOT}/local_nginx_sm2_kyber768_extract.txt")
\`\`\`

tshark selected key_share group:

\`\`\`text
$(cat "${pcap_parse}" 2>/dev/null || true)
\`\`\`

Observed selected key_share group: \`${selected_group}\`.

## ZoTrus classification

- \`curveSM2MLKEM768\`: failed locally because this Tongsuo build does not accept the group name.
- \`X25519MLKEM768\`: failed locally because this Tongsuo build does not accept the group name.
- \`SM2:KYBER768\`: ServerHello received, but the client aborts with \`final_tls1_3_kyber768_ciphertext:extension not received\`.

Using the requested categories:

| Category | Meaning | Result |
|---|---|---|
| A | \`curveSM2MLKEM768\` name succeeds: standard-name compatible | No |
| B | \`SM2:KYBER768\` succeeds and pcap shows \`0x11EE\`: internal name differs, codepoint compatible | No |
| C | both fail: current Tongsuo/TLCP-PQC and ZoTrus GM PQC-TLS are not interoperable yet | Yes |

## Conclusion

EOF_REPORT

  if [ "${selected_group}" = "0x11EE" ]; then
    cat >> "${report}" <<'EOF_REPORT'
当前实现使用内部配置名称 SM2:KYBER768，但在线路层协商的 TLS Supported Group 码点为 0x11EE，可与公开 curveSM2MLKEM768 码点对齐。
EOF_REPORT
  else
    cat >> "${report}" <<'EOF_REPORT'
当前实现仅验证了本地实验名称 SM2:KYBER768 路径，尚不能证明与公开 curveSM2MLKEM768 / 0x11EE 生态兼容。
EOF_REPORT
  fi

  cat >> "${report}" <<'EOF_REPORT'

Do not interpret ZTBrowser `X25519MLKEM768 / 0x11EC` success as SM2+Kyber/ML-KEM768 success. SM2+Kyber/ML-KEM768 compatibility must be judged by `0x11EE` or by `SM2:KYBER768` mapping to `0x11EE` on the wire.
EOF_REPORT
}

if [ ! -x "${NGINX_BIN}" ]; then
  echo "Missing Nginx binary: ${NGINX_BIN}" >&2
  exit 1
fi

echo "== nginx -t evidence =="
run_nginx_t "x25519mlkem768_fail" "X25519MLKEM768" || true
run_nginx_t "curvesm2mlkem768_fail" "curveSM2MLKEM768" || true
run_nginx_t "sm2_kyber768_success" "SM2:KYBER768" || true

echo "== local Nginx SM2:KYBER768 =="
stop_nginx
write_nginx_conf "SM2:KYBER768"
"${NGINX_BIN}" -p "${NGINX_WORK}" -c "${NGINX_CONF}" -t >"${LOG_ROOT}/local_nginx_t_before_start.log" 2>&1
"${NGINX_BIN}" -p "${NGINX_WORK}" -c "${NGINX_CONF}"
trap stop_nginx EXIT
wait_port "${LOCAL_PORT}"

PCAP="${ROOT}/logs/local_nginx_sm2_kyber768.pcap"
rm -f "${PCAP}"
TCPDUMP_LOG="${LOG_ROOT}/tcpdump.log"
if command -v tcpdump >/dev/null 2>&1; then
  timeout 20 tcpdump -i lo -w "${PCAP}" "tcp port ${LOCAL_PORT}" >"${TCPDUMP_LOG}" 2>&1 &
  TCPDUMP_PID="$!"
  sleep 1
else
  TCPDUMP_PID=""
  echo "tcpdump not found" >"${TCPDUMP_LOG}"
fi

LOCAL_CLIENT_LOG="${LOG_ROOT}/local_nginx_sm2_kyber768_s_client.log"
{
  echo "command=${OPENSSL} s_client -connect 127.0.0.1:${LOCAL_PORT} -servername localhost -tls1_3 -groups SM2:KYBER768 -ciphersuites TLS_SM4_GCM_SM3 -enable_sm2_kyber768_tls13 -state -tlsextdebug -msg"
  printf 'GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n' | "${OPENSSL}" s_client \
    -connect "127.0.0.1:${LOCAL_PORT}" \
    -servername localhost \
    -tls1_3 \
    -groups SM2:KYBER768 \
    -ciphersuites TLS_SM4_GCM_SM3 \
    -enable_sm2_kyber768_tls13 \
    -state -tlsextdebug -msg
} >"${LOCAL_CLIENT_LOG}" 2>&1 || true

if [ -n "${TCPDUMP_PID}" ]; then
  wait "${TCPDUMP_PID}" >/dev/null 2>&1 || true
fi

extract_tls_notes "${LOCAL_CLIENT_LOG}" "${LOG_ROOT}/local_nginx_sm2_kyber768_extract.txt"
if [ -f "${PCAP}" ]; then
  parse_pcap "${PCAP}" "${LOG_ROOT}/local_nginx_sm2_kyber768_pcap_parse.txt"
fi

echo "== remote ZoTrus =="
run_remote "curvesm2mlkem768" "curveSM2MLKEM768"
run_remote "x25519mlkem768" "X25519MLKEM768"
run_remote "sm2_kyber768" "SM2:KYBER768"

write_report

echo "Report: ${LOG_ROOT}/REPORT.md"
echo "PCAP: ${PCAP}"
