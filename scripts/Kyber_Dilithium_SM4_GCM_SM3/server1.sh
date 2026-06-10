#!/usr/bin/env bash
# TLCP 客户端（连接 certs/1 服务端）
source "$(dirname "$0")/../../env.sh"

exec "${OPENSSL}" s_client -connect 127.0.0.1:4433 \
  -ntls -enable_ntls \
  -CAfile /tmp/tlcp_ca_chain.pem \
  -cipher 'KYBER-DILITHIUM-SM4-GCM-SM3'
