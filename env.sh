#!/usr/bin/env bash
# 在仓库根目录 source 本文件，统一设置构建与运行环境变量。
# 用法: source /path/to/tlcpMonoRepo/env.sh

TLCP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export TLCP_ROOT

export TONGSUO_ROOT="${TLCP_ROOT}/tongsuo"
export OPENSSL="${TONGSUO_ROOT}/apps/openssl"
export PQMAGIC_PREFIX="${TLCP_ROOT}/pqmagic/install"
export PQMAGIC_LIB="${PQMAGIC_PREFIX}/lib"
export CERTS="${TLCP_ROOT}/certs"

export PROVIDER_PQMAGIC_DIR="${TLCP_ROOT}/providers/pqmagic-algorithms/build"
export PROVIDER_AIGIS_DIR="${TLCP_ROOT}/providers/ntls-aigis/build"

export PQMAGIC_PROVIDER="${PROVIDER_PQMAGIC_DIR}/pqmagic.so"
export AIGIS_ENC_PROVIDER="${PROVIDER_AIGIS_DIR}/aigis_enc.so"

export LD_LIBRARY_PATH="${TONGSUO_ROOT}:${PQMAGIC_LIB}:${LD_LIBRARY_PATH:-}"
export OPENSSL_MODULES="${PROVIDER_PQMAGIC_DIR}"
export OPENSSL_CONF="${TLCP_ROOT}/config/openssl-providers.cnf"
