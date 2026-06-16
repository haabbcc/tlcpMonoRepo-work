#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
NGINX_VERSION="${NGINX_VERSION:-1.30.2}"
NGINX_SRC="${NGINX_SRC:-${ROOT}/third_party/nginx-${NGINX_VERSION}}"
NGINX_PREFIX="${NGINX_PREFIX:-${ROOT}/nginx-install}"
NGINX_WORK="${NGINX_WORK:-${ROOT}/nginx-work}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

if [ ! -f "${NGINX_SRC}/configure" ]; then
  echo "Missing Nginx source: ${NGINX_SRC}" >&2
  echo "Download https://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz into third_party first." >&2
  exit 1
fi

cd "${NGINX_SRC}"
./configure \
  --prefix="${NGINX_PREFIX}" \
  --conf-path="${NGINX_WORK}/nginx.conf" \
  --pid-path="${NGINX_WORK}/nginx.pid" \
  --error-log-path="${NGINX_WORK}/error.log" \
  --http-log-path="${NGINX_WORK}/access.log" \
  --without-http_gzip_module \
  --with-http_ssl_module \
  --with-http_v2_module \
  --with-stream \
  --with-stream_ssl_module \
  --with-cc-opt="-I${ROOT}/tongsuo/include" \
  --with-ld-opt="-L${ROOT}/tongsuo -Wl,-rpath,${ROOT}/tongsuo"
make -j"${JOBS}"
make install

"${NGINX_PREFIX}/sbin/nginx" -V
