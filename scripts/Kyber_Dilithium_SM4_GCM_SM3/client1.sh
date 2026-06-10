#!/usr/bin/env bash
# TLCP 服务端（certs/1 裸 PQC 双证，KYBER-DILITHIUM-SM4-GCM-SM3）
source "$(dirname "$0")/../../env.sh"

cat "${CERTS}/1/root.crt" "${CERTS}/1/yunying.crt" > /tmp/tlcp_ca_chain.pem

exec "${OPENSSL}" s_server -accept 4433 \
  -ntls -enable_ntls \
  -sign_cert "${CERTS}/1/user_sig.crt" \
  -sign_key "${CERTS}/1/user_sig_pkcs8.pem" \
  -enc_cert "${CERTS}/1/user_enc.crt" \
  -enc_key "${CERTS}/1/user_enc_pkcs8.pem" \
  -CAfile /tmp/tlcp_ca_chain.pem \
  -cipher 'KYBER-DILITHIUM-SM4-GCM-SM3'
