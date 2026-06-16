# Nginx TLS/PQC Integration

This integration runs upstream Nginx 1.30.2 as a standard TLS server using the
local Tongsuo library and the existing SM2 test certificate.

It does not require NTLS support in Nginx.

## Files

| Path | Purpose |
|---|---|
| `scripts/Nginx_TLS_PQC/build-nginx.sh` | Build Nginx 1.30.2 against local Tongsuo |
| `scripts/Nginx_TLS_PQC/render-conf.sh` | Render `nginx-work/nginx.conf` |
| `scripts/Nginx_TLS_PQC/start-nginx.sh` | Start Nginx on `127.0.0.1:8443` |
| `scripts/Nginx_TLS_PQC/stop-nginx.sh` | Stop the test Nginx instance |
| `scripts/Nginx_TLS_PQC/test-client.sh` | Connect with Tongsuo `s_client` |
| `scripts/Nginx_TLS_PQC/test.sh` | End-to-end Nginx TLS test |
| `config/nginx-tls-pqc.conf.in` | Nginx configuration template |

## Test

```bash
cd /root/tlcpMonoRepo-work
bash scripts/Nginx_TLS_PQC/test.sh
```

Expected result:

```text
New, TLSv1.3, Cipher is TLS_SM4_GCM_SM3
HTTP/1.1 200 OK
nginx_tls_pqc_ok
Nginx TLS test ok
```

The tested configuration uses:

```nginx
ssl_protocols TLSv1.2 TLSv1.3;
ssl_certificate /root/tlcpMonoRepo-work/certs/loose/sign_sm2.crt;
ssl_certificate_key /root/tlcpMonoRepo-work/certs/loose/sign_sm2.key;
ssl_conf_command Ciphersuites TLS_SM4_GCM_SM3;
ssl_conf_command Groups SM2:KYBER768;
```

Because Nginx is not using Angie NTLS support, the server presents one standard
certificate instead of the NTLS signing/encryption dual-certificate model.
