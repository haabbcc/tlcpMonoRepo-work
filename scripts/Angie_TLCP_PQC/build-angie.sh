#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
source "${ROOT}/env.sh"

JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
ANGIE_REPO="${ANGIE_REPO:-https://github.com/webserver-llc/angie.git}"
ANGIE_REF="${ANGIE_REF:-master}"
ANGIE_SRC="${ANGIE_SRC:-${ROOT}/third_party/angie}"
ANGIE_PREFIX="${ANGIE_PREFIX:-${ROOT}/angie-install}"
ANGIE_EXTRA_CONFIGURE="${ANGIE_EXTRA_CONFIGURE:-}"

if [[ ! -x "${OPENSSL}" ]]; then
  echo "Missing ${OPENSSL}; run ./build-all.sh first." >&2
  exit 1
fi

mkdir -p "$(dirname "${ANGIE_SRC}")" "${ANGIE_PREFIX}"

if [[ ! -d "${ANGIE_SRC}/.git" ]]; then
  git clone "${ANGIE_REPO}" "${ANGIE_SRC}"
fi

(
  cd "${ANGIE_SRC}"
  git fetch --tags origin
  git checkout "${ANGIE_REF}"

  ./configure \
    --prefix="${ANGIE_PREFIX}" \
    --with-http_ssl_module \
    --with-ntls \
    --with-openssl="${TONGSUO_ROOT}" \
    --with-openssl-opt="enable-ntls" \
    ${ANGIE_EXTRA_CONFIGURE}

  make -j"${JOBS}"
  make install
)

echo "Angie installed to ${ANGIE_PREFIX}"
