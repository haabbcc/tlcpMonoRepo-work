# Build Nginx 1.30.2

This project can build upstream Nginx 1.30.2 against the already-built local
Tongsuo library.

Nginx is intentionally built without Angie NTLS support. Do not use Angie-only
directives such as `ssl_ntls`, `sign:` certificates, or `enc:` certificates in
the Nginx configuration.

## Source

The tested source tree is:

```bash
/root/tlcpMonoRepo-work/third_party/nginx-1.30.2
```

If it is missing, download and unpack:

```bash
cd /root/tlcpMonoRepo-work
curl -fL -o /tmp/nginx-1.30.2.tar.gz https://nginx.org/download/nginx-1.30.2.tar.gz
tar -xzf /tmp/nginx-1.30.2.tar.gz -C third_party
```

## Build

```bash
cd /root/tlcpMonoRepo-work
bash scripts/Nginx_TLS_PQC/build-nginx.sh
```

The script links against the existing Tongsuo headers and shared libraries:

```text
-I/root/tlcpMonoRepo-work/tongsuo/include
-L/root/tlcpMonoRepo-work/tongsuo -Wl,-rpath,/root/tlcpMonoRepo-work/tongsuo
```

Do not use Nginx `--with-openssl=/root/tlcpMonoRepo-work/tongsuo` for this
repository. That option makes Nginx run `make clean` and reconfigure the Tongsuo
source tree, which can break the existing TLCP/PQC build.

The tested build disables `http_gzip_module` because Nginx 1.30.2's certificate
compression integration expects a callback ABI that differs from this Tongsuo
tree.

## Verify

```bash
cd /root/tlcpMonoRepo-work
nginx-install/sbin/nginx -V
bash scripts/Nginx_TLS_PQC/render-conf.sh
nginx-install/sbin/nginx -p /root/tlcpMonoRepo-work/nginx-work \
  -c /root/tlcpMonoRepo-work/nginx-work/nginx.conf -t
```
