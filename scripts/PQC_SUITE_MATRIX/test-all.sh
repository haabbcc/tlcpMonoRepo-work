#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
source "${ROOT}/env.sh"

OUT_DIR="${ROOT}/.tmp/pqc-suite-matrix"
mkdir -p "${OUT_DIR}"
rm -f "${OUT_DIR}"/*.log "${OUT_DIR}"/summary.tsv

CERT_LOOSE="${CERTS}/loose"
CA_LOOSE="${CERT_LOOSE}/ca_sm2.crt"
SIGN_CRT_LOOSE="${CERT_LOOSE}/sign_sm2.crt"
SIGN_KEY_LOOSE="${CERT_LOOSE}/sign_sm2.key"
ENC_CRT_LOOSE="${CERT_LOOSE}/enc_sm2.crt"
ENC_KEY_LOOSE="${CERT_LOOSE}/enc_sm2.key"

CERT_PQC="${CERTS}/1"
CA_CHAIN="${OUT_DIR}/tlcp_ca_chain.pem"
cat "${CERT_PQC}/root.crt" "${CERT_PQC}/yunying.crt" > "${CA_CHAIN}"

printf 'suite\tprotocol\tport\tresult\tdetail\n' > "${OUT_DIR}/summary.tsv"

cleanup_pid=""
cleanup() {
  if [ -n "${cleanup_pid}" ] && kill -0 "${cleanup_pid}" 2>/dev/null; then
    kill "${cleanup_pid}" 2>/dev/null || true
    wait "${cleanup_pid}" 2>/dev/null || true
  fi
}
trap cleanup EXIT

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

record() {
  local suite="$1" protocol="$2" port="$3" result="$4" detail="$5"
  printf '%s\t%s\t%s\t%s\t%s\n' "$suite" "$protocol" "$port" "$result" "$detail" \
    | tee -a "${OUT_DIR}/summary.tsv"
}

run_ntls_suite() {
  local suite="$1" cert_model="$2" port="$3"
  local server_log="${OUT_DIR}/${suite}.server.log"
  local client_log="${OUT_DIR}/${suite}.client.log"
  local -a server_args client_args

  if [ "${cert_model}" = "loose" ]; then
    server_args=(
      "${OPENSSL}" s_server -accept "${port}"
      -enable_ntls -ntls
      -sign_cert "${SIGN_CRT_LOOSE}" -sign_key "${SIGN_KEY_LOOSE}"
      -enc_cert "${ENC_CRT_LOOSE}" -enc_key "${ENC_KEY_LOOSE}"
      -CAfile "${CA_LOOSE}"
      -cipher "${suite}" -www
    )
    client_args=(
      "${OPENSSL}" s_client -connect "127.0.0.1:${port}"
      -enable_ntls -ntls
      -sign_cert "${SIGN_CRT_LOOSE}" -sign_key "${SIGN_KEY_LOOSE}"
      -enc_cert "${ENC_CRT_LOOSE}" -enc_key "${ENC_KEY_LOOSE}"
      -CAfile "${CA_LOOSE}"
      -cipher "${suite}" -brief
    )
  else
    server_args=(
      "${OPENSSL}" s_server -accept "${port}"
      -enable_ntls -ntls
      -sign_cert "${CERT_PQC}/user_sig.crt"
      -sign_key "${CERT_PQC}/user_sig_pkcs8.pem"
      -enc_cert "${CERT_PQC}/user_enc.crt"
      -enc_key "${CERT_PQC}/user_enc_pkcs8.pem"
      -CAfile "${CA_CHAIN}"
      -cipher "${suite}" -www
    )
    client_args=(
      "${OPENSSL}" s_client -connect "127.0.0.1:${port}"
      -enable_ntls -ntls
      -CAfile "${CA_CHAIN}"
      -cipher "${suite}" -brief
    )
  fi

  echo "==> NTLS ${suite}"
  timeout 20 "${server_args[@]}" >"${server_log}" 2>&1 &
  cleanup_pid="$!"

  if ! wait_port "${port}"; then
    record "${suite}" "NTLSv1.1" "${port}" "FAIL" "server did not listen"
    cleanup
    cleanup_pid=""
    return 1
  fi

  if printf 'GET / HTTP/1.0\r\n\r\nQ\n' | timeout 12 "${client_args[@]}" >"${client_log}" 2>&1; then
    if grep -q "Cipher is ${suite}" "${client_log}" \
      || grep -q "Ciphersuite: ${suite}" "${client_log}" \
      || grep -q "Protocol version: NTLS" "${client_log}"; then
      local detail
      detail="$(grep -E 'Protocol|Cipher|Ciphersuite|Verification|Verify return code' "${client_log}" | tr '\n' ';' | sed 's/[[:space:]]\+/ /g' | cut -c1-220)"
      record "${suite}" "NTLSv1.1" "${port}" "PASS" "${detail:-handshake ok}"
    else
      record "${suite}" "NTLSv1.1" "${port}" "FAIL" "client completed but expected cipher/protocol marker missing"
    fi
  else
    local detail
    detail="$(tail -n 20 "${client_log}" | tr '\n' ';' | sed 's/[[:space:]]\+/ /g' | cut -c1-220)"
    record "${suite}" "NTLSv1.1" "${port}" "FAIL" "${detail:-client failed}"
  fi

  cleanup
  cleanup_pid=""
}

run_tls13_sm2_kyber768() {
  local suite="TLS13-SM2-KYBER768"
  local port="$1"
  local server_log="${OUT_DIR}/${suite}.server.log"
  local client_log="${OUT_DIR}/${suite}.client.log"

  echo "==> TLS 1.3 SM2+Kyber768"
  TS_HYBRID_KEX_DEBUG=1 timeout 20 "${OPENSSL}" s_server \
    -accept "${port}" \
    -tls1_3 \
    -enable_sm_tls13_strict \
    -enable_sm2_kyber768_tls13 \
    -groups "SM2:KYBER768" \
    -ciphersuites "TLS_SM4_GCM_SM3" \
    -cert "${SIGN_CRT_LOOSE}" \
    -key "${SIGN_KEY_LOOSE}" \
    -CAfile "${CA_LOOSE}" \
    -verify_return_error \
    -www >"${server_log}" 2>&1 &
  cleanup_pid="$!"

  if ! wait_port "${port}"; then
    record "${suite}" "TLSv1.3" "${port}" "FAIL" "server did not listen"
    cleanup
    cleanup_pid=""
    return 1
  fi

  if printf 'GET / HTTP/1.0\r\n\r\nQ\n' | TS_HYBRID_KEX_DEBUG=1 timeout 12 "${OPENSSL}" s_client \
    -connect "127.0.0.1:${port}" \
    -tls1_3 \
    -enable_sm_tls13_strict \
    -enable_sm2_kyber768_tls13 \
    -groups "SM2:KYBER768" \
    -ciphersuites "TLS_SM4_GCM_SM3" \
    -CAfile "${CA_LOOSE}" \
    -verify_return_error >"${client_log}" 2>&1; then
    if grep -q "Cipher is TLS_SM4_GCM_SM3" "${client_log}" \
      && grep -q "Hybrid IKM \(ECDH\|\|Kyber\)" "${client_log}" "${server_log}"; then
      local detail
      detail="$(grep -E 'New, TLSv1.3|Shared groups|Verify return code|Hybrid IKM' "${client_log}" "${server_log}" | tr '\n' ';' | sed 's/[[:space:]]\+/ /g' | cut -c1-220)"
      record "${suite}" "TLSv1.3" "${port}" "PASS" "${detail:-handshake ok}"
    else
      record "${suite}" "TLSv1.3" "${port}" "FAIL" "expected TLS_SM4_GCM_SM3/hybrid KDF markers missing"
    fi
  else
    local detail
    detail="$(tail -n 20 "${client_log}" | tr '\n' ';' | sed 's/[[:space:]]\+/ /g' | cut -c1-220)"
    record "${suite}" "TLSv1.3" "${port}" "FAIL" "${detail:-client failed}"
  fi

  cleanup
  cleanup_pid=""
}

run_ntls_suite "ECC-KYBER-SM4-GCM-SM3" "loose" 4501 || true
run_ntls_suite "ECDHE-KYBER-SM4-GCM-SM3" "loose" 4502 || true
run_ntls_suite "KYBER-DILITHIUM-SM4-GCM-SM3" "pqc" 4503 || true
run_ntls_suite "KYBER-AIGIS-ENC-DILITHIUM-SM4-GCM-SM3" "pqc" 4504 || true
run_tls13_sm2_kyber768 4505 || true

echo
if command -v column >/dev/null 2>&1; then
  column -t -s $'\t' "${OUT_DIR}/summary.tsv"
else
  cat "${OUT_DIR}/summary.tsv"
fi

if grep -q $'\tFAIL\t' "${OUT_DIR}/summary.tsv"; then
  echo "One or more PQC suite tests failed. Logs: ${OUT_DIR}" >&2
  exit 1
fi

echo "All PQC suite tests passed. Logs: ${OUT_DIR}"
