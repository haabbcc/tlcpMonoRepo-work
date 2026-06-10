#!/usr/bin/env bash
# 使用 Tongsuo 的 openssl，并加载 pqmagic provider，解析/显示国密 PQC 证书公钥
set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${DIR}/../../env.sh"

exec "${OPENSSL}" "$@" \
  -provider-path "${PROVIDER_PQMAGIC_DIR}" \
  -provider default \
  -provider pqmagic
