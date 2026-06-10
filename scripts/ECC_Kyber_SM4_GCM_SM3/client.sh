#!/usr/bin/env bash
# TLCP 客户端（ECC-KYBER-SM4-GCM-SM3）
source "$(dirname "$0")/../../env.sh"

CERT="${CERTS}/loose"
CA="${CERT}/ca_sm2.crt"
SIGN_CRT="${CERT}/sign_sm2.crt"
SIGN_KEY="${CERT}/sign_sm2.key"
ENC_CRT="${CERT}/enc_sm2.crt"
ENC_KEY="${CERT}/enc_sm2.key"

exec "${OPENSSL}" s_client -connect 127.0.0.1:4433 \
  -enable_ntls -ntls \
  -sign_cert "$SIGN_CRT" -sign_key "$SIGN_KEY" \
  -enc_cert "$ENC_CRT" -enc_key "$ENC_KEY" \
  -CAfile "$CA" \
  -cipher 'ECC-KYBER-SM4-GCM-SM3'
