# Nginx TLS/PQC integration

This directory builds and runs upstream Nginx 1.30.2 with the local Tongsuo library. It intentionally does not enable Angie NTLS directives such as `ssl_ntls` or `sign:/enc:` dual certificate syntax.

```bash
cd /root/tlcpMonoRepo-work
bash scripts/Nginx_TLS_PQC/build-nginx.sh
bash scripts/Nginx_TLS_PQC/render-conf.sh
bash scripts/Nginx_TLS_PQC/start-nginx.sh
bash scripts/Nginx_TLS_PQC/test-client.sh
bash scripts/Nginx_TLS_PQC/stop-nginx.sh
```

The build uses external Tongsuo headers and shared libraries instead of Nginx `--with-openssl=...`, because that option reconfigures and cleans the Tongsuo source tree.
