#!/usr/bin/env bash
# 从零构建抗量子 TLCP（certs/1 + SM2 loose 联调所需组件）
set -euo pipefail

source "$(dirname "$0")/env.sh"

JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
TONGSUO_CONFIGURE="${TONGSUO_CONFIGURE:-linux-x86_64 enable-ntls}"

echo "==> TLCP_ROOT=${TLCP_ROOT}"

# 1. PQMagic
echo "==> [1/4] 构建 PQMagic (USE_SHAKE=ON) ..."
mkdir -p "${TLCP_ROOT}/pqmagic/build"
cmake -S "${TLCP_ROOT}/pqmagic" -B "${TLCP_ROOT}/pqmagic/build" \
  -DUSE_SHAKE=ON \
  -DCMAKE_INSTALL_PREFIX="${PQMAGIC_PREFIX}"
cmake --build "${TLCP_ROOT}/pqmagic/build" -j"${JOBS}"
cmake --install "${TLCP_ROOT}/pqmagic/build"

# 2. Tongsuo
echo "==> [2/4] 构建 Tongsuo (NTLS/TLCP) ..."
if [[ ! -f "${TONGSUO_ROOT}/Makefile" ]]; then
  (cd "${TONGSUO_ROOT}" && ./Configure ${TONGSUO_CONFIGURE})
fi
make -C "${TONGSUO_ROOT}" -j"${JOBS}"

# 3. pqmagic-algorithms provider
echo "==> [3/4] 构建 pqmagic-algorithms provider ..."
cmake -S "${TLCP_ROOT}/providers/pqmagic-algorithms" \
  -B "${TLCP_ROOT}/providers/pqmagic-algorithms/build"
cmake --build "${TLCP_ROOT}/providers/pqmagic-algorithms/build" -j"${JOBS}"

# 4. ntls-aigis provider
echo "==> [4/4] 构建 ntls-aigis provider ..."
cmake -S "${TLCP_ROOT}/providers/ntls-aigis" \
  -B "${TLCP_ROOT}/providers/ntls-aigis/build"
cmake --build "${TLCP_ROOT}/providers/ntls-aigis/build" -j"${JOBS}"

# 生成 provider 配置文件
sed "s|@TLCP_ROOT@|${TLCP_ROOT}|g" \
  "${TLCP_ROOT}/config/openssl-providers.cnf.in" \
  > "${TLCP_ROOT}/config/openssl-providers.cnf"

echo ""
echo "构建完成。验证："
echo "  source ${TLCP_ROOT}/env.sh"
echo "  \${OPENSSL} version -a"
echo "  \${OPENSSL} list -providers"
