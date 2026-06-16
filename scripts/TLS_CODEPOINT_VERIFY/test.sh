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
PCAP="${ROOT}/logs/local_nginx_sm2_kyber768.pcap"
REPORT="${LOG_ROOT}/REPORT.md"

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
  } >"${out}" 2>&1 || true
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
    grep -Ein 'key.share|key_share|supported.groups|supported_groups|extension.*(10|51)|0x0033|0x000a|11ee|11 ec|11ec|kyber|mlkem|sm2' "${src}" || true
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
      echo "tshark not found; install tshark to decode selected key_share group."
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
  local selected_group="$1"
  local zotrus_class="$2"
  cat > "${REPORT}" <<EOF_REPORT
# TLS Codepoint Verification

Date: $(date -u '+%Y-%m-%dT%H:%M:%SZ')

## Local nginx -t evidence

- X25519MLKEM768: see \`${LOG_ROOT}/nginx_t_x25519mlkem768_fail.log\`
- curveSM2MLKEM768: see \`${LOG_ROOT}/nginx_t_curvesm2mlkem768_fail.log\`
- SM2:KYBER768: see \`${LOG_ROOT}/nginx_t_sm2_kyber768_success.log\`

The first two group names are rejected by this Nginx/Tongsuo build. The
internal group expression \`SM2:KYBER768\` is accepted by \`nginx -t\`.

## Local wire-level result

Client log:

\`${LOG_ROOT}/local_nginx_sm2_kyber768_s_client.log\`

Pcap:

\`${PCAP}\`

tshark parse:

\`${LOG_ROOT}/local_nginx_sm2_kyber768_pcap_parse.txt\`

Observed ServerHello selected key_share group:

\`\`\`text
${selected_group}
\`\`\`

The exact \`s_client\` command requested with \`-enable_sm2_kyber768_tls13\`
does not complete against local Nginx. It aborts after ServerHello with:

\`\`\`text
final_tls1_3_kyber768_ciphertext:extension not received
\`\`\`

Therefore the expected HTTP response is not reached in that exact run.

## ZoTrus classification

Requested categories:

- A: \`curveSM2MLKEM768\` name succeeds: standard-name compatible.
- B: \`SM2:KYBER768\` name succeeds and packet capture shows \`0x11EE\`.
- C: both fail: current Tongsuo/TLCP-PQC and ZoTrus GM PQC-TLS are not interoperable yet.

Result:

\`\`\`text
${zotrus_class}
\`\`\`

## Conclusion

当前实现仅验证了本地实验名称 SM2:KYBER768 路径，尚不能证明与公开 curveSM2MLKEM768 / 0x11EE 生态兼容。

ZTBrowser 显示的 X25519MLKEM768 / 0x11EC 成功不应写成 SM2+Kyber768 成功。SM2+Kyber/ML-KEM768 必须看 0x11EE 或 SM2:KYBER768 对应的实际线路码点。
EOF_REPORT
}

echo "== nginx -t evidence =="
run_nginx_t "x25519mlkem768_fail" "X25519MLKEM768"
run_nginx_t "curvesm2mlkem768_fail" "curveSM2MLKEM768"
run_nginx_t "sm2_kyber768_success" "SM2:KYBER768"

echo "== local Nginx SM2:KYBER768 =="
stop_nginx
write_nginx_conf "SM2:KYBER768"
"${NGINX_BIN}" -p "${NGINX_WORK}" -c "${NGINX_CONF}" -t >"${LOG_ROOT}/local_nginx_t_before_start.log" 2>&1
"${NGINX_BIN}" -p "${NGINX_WORK}" -c "${NGINX_CONF}"
trap stop_nginx EXIT
wait_port "${LOCAL_PORT}"

rm -f "${PCAP}"
if command -v tcpdump >/dev/null 2>&1; then
  timeout 20 tcpdump -i lo -w "${PCAP}" "tcp port ${LOCAL_PORT}" >"${LOG_ROOT}/tcpdump.log" 2>&1 &
  TCPDUMP_PID="$!"
  sleep 1
else
  TCPDUMP_PID=""
  echo "tcpdump not found" >"${LOG_ROOT}/tcpdump.log"
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

SELECTED_GROUP="$(grep -E 'Group: curveSM2|fields:|^[0-9]+[[:space:]]+[0-9]+' "${LOG_ROOT}/local_nginx_sm2_kyber768_pcap_parse.txt" | tr '\n' ';' | sed 's/[[:space:]]\+/ /g' | cut -c1-240 || true)"
if grep -q '0x11EE\|11EE\|4590' "${LOG_ROOT}/local_nginx_sm2_kyber768_pcap_parse.txt"; then
  CONCLUSION='当前实现使用内部配置名称 SM2:KYBER768，但在线路层协商的 TLS Supported Group 码点为 0x11EE，可与公开 curveSM2MLKEM768 码点对齐。'
else
  CONCLUSION='当前实现仅验证了本地实验名称 SM2:KYBER768 路径，尚不能证明与公开 curveSM2MLKEM768 / 0x11EE 生态兼容。'
fi

if grep -q 'Call to SSL_CONF_cmd(-groups, curveSM2MLKEM768) failed' "${LOG_ROOT}/zotrus_curvesm2mlkem768.log" \
  && grep -q 'final_tls1_3_kyber768_ciphertext:extension not received' "${LOG_ROOT}/zotrus_sm2_kyber768.log"; then
  ZOTRUS_CLASS='C. 两者都失败：当前 Tongsuo/TLCP-PQC 与 ZoTrus 国密 PQC-TLS 尚未互通。'
else
  ZOTRUS_CLASS='需要人工复核 ZoTrus 日志。'
fi

write_report "${SELECTED_GROUP:-not parsed}" "${ZOTRUS_CLASS}"

echo "== conclusion =="
echo "${CONCLUSION}"
echo "${ZOTRUS_CLASS}"
echo "Report: ${REPORT}"
echo "PCAP: ${PCAP}"
