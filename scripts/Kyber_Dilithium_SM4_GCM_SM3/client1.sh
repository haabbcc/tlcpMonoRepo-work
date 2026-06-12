#!/usr/bin/env bash
# TLCP client for the certs/1 PQC double-certificate server.
set -euo pipefail
source "$(dirname "$0")/../../env.sh"

exec "${OPENSSL}" s_client -connect 127.0.0.1:4433 \
  -ntls -enable_ntls \
  -CAfile /tmp/tlcp_ca_chain.pem \
  -cipher 'KYBER-DILITHIUM-SM4-GCM-SM3'
