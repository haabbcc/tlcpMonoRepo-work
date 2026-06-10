# Angie + Tongsuo + TLCP-PQC Build And Test Runbook

This document records the build, deployment, and verification flow for Angie + Tongsuo + tlcpMonoRepo inside WSL.

## 1. Environment

All build and runtime files were kept inside the WSL filesystem:

```bash
/root/tlcpMonoRepo-work
```

| Item | Value |
|---|---|
| WSL distro | `TLCPTest` |
| Project root | `/root/tlcpMonoRepo-work` |
| Tongsuo | `Tongsuo 8.5.0-dev`, OpenSSL `3.0.3` |
| Angie | `Angie/1.12.0` |
| Protocol | `NTLSv1.1` |
| Cipher | `ECC-KYBER-SM4-GCM-SM3` |
| Certificates | `/root/tlcpMonoRepo-work/certs/loose` |

## 2. Build TLCP-PQC Components

Run the repository build script:

```bash
cd /root/tlcpMonoRepo-work
JOBS=4 bash ./build-all.sh
```

The script builds these components:

| Component | Output |
|---|---|
| PQMagic | `/root/tlcpMonoRepo-work/pqmagic/install` |
| Tongsuo | `/root/tlcpMonoRepo-work/tongsuo` |
| pqmagic provider | `/root/tlcpMonoRepo-work/providers/pqmagic-algorithms/build/pqmagic.so` |
| aigis_enc provider | `/root/tlcpMonoRepo-work/providers/ntls-aigis/build/aigis_enc.so` |
| provider config | `/root/tlcpMonoRepo-work/config/openssl-providers.cnf` |

Verify provider loading:

```bash
cd /root/tlcpMonoRepo-work
OPENSSL_CONF=/root/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/root/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/root/tlcpMonoRepo-work/tongsuo:/root/tlcpMonoRepo-work/pqmagic/install/lib \
/root/tlcpMonoRepo-work/tongsuo/apps/openssl list -providers
```

Expected active providers:

```text
default
pqmagic
aigis_enc
```

## 3. Fetch Angie Source

`git clone` was unstable in this environment, so the source was fetched as a GitHub tarball:

```bash
cd /root/tlcpMonoRepo-work
mkdir -p third_party
curl -L --retry 5 --retry-delay 3 --http1.1 \
  -o third_party/angie.tar.gz \
  https://github.com/webserver-llc/angie/archive/refs/heads/master.tar.gz
mkdir -p third_party/angie
tar -xzf third_party/angie.tar.gz -C third_party/angie --strip-components=1
```

The tested source reported `Angie/1.12.0`.

## 4. Build Angie With Tongsuo NTLS

```bash
cd /root/tlcpMonoRepo-work/third_party/angie
./configure \
  --prefix=/root/tlcpMonoRepo-work/angie-install \
  --conf-path=/root/tlcpMonoRepo-work/angie-work/angie.conf \
  --pid-path=/root/tlcpMonoRepo-work/angie-work/angie.pid \
  --error-log-path=/root/tlcpMonoRepo-work/angie-work/error.log \
  --http-log-path=/root/tlcpMonoRepo-work/angie-work/access.log \
  --with-http_ssl_module \
  --with-http_v2_module \
  --with-stream \
  --with-stream_ssl_module \
  --with-ntls \
  --with-openssl=/root/tlcpMonoRepo-work/tongsuo \
  --with-openssl-opt=enable-ntls
make -j4
make install
```

Verify build options:

```bash
/root/tlcpMonoRepo-work/angie-install/sbin/angie -V
```

The output should include:

```text
--with-ntls
--with-openssl=/root/tlcpMonoRepo-work/tongsuo
--with-openssl-opt=enable-ntls
```

## 5. Angie Config

Config path:

```bash
/root/tlcpMonoRepo-work/angie-work/angie.conf
```

Working minimal config:

```nginx
worker_processes 1;
error_log /root/tlcpMonoRepo-work/angie-work/error.log debug;
pid /root/tlcpMonoRepo-work/angie-work/angie.pid;

events {
    worker_connections 1024;
}

http {
    access_log /root/tlcpMonoRepo-work/angie-work/access.log;

    server {
        listen 127.0.0.1:8443 ssl;
        server_name localhost;
        ssl_ntls on;

        ssl_certificate     sign:/root/tlcpMonoRepo-work/certs/loose/sign_sm2.crt;
        ssl_certificate_key sign:/root/tlcpMonoRepo-work/certs/loose/sign_sm2.key;
        ssl_certificate     enc:/root/tlcpMonoRepo-work/certs/loose/enc_sm2.crt;
        ssl_certificate_key enc:/root/tlcpMonoRepo-work/certs/loose/enc_sm2.key;

        ssl_client_certificate /root/tlcpMonoRepo-work/certs/loose/ca_sm2.crt;
        ssl_verify_client optional;
        ssl_ciphers ECC-KYBER-SM4-GCM-SM3;

        location / {
            return 200 angie_tlcp_pqc_ok;
        }
    }
}
```

Angie uses `sign:` and `enc:` prefixes for NTLS dual certificates. Source inspection confirmed that these map to Tongsuo APIs:

```text
SSL_CTX_use_sign_certificate()
SSL_CTX_use_sign_PrivateKey()
SSL_CTX_use_enc_certificate()
SSL_CTX_use_enc_PrivateKey()
```

## 6. Validate And Start Angie

Validate config:

```bash
cd /root/tlcpMonoRepo-work
OPENSSL_CONF=/root/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/root/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/root/tlcpMonoRepo-work/pqmagic/install/lib \
/root/tlcpMonoRepo-work/angie-install/sbin/angie \
  -t -c /root/tlcpMonoRepo-work/angie-work/angie.conf
```

Observed result:

```text
syntax is ok
configuration file ... test is successful
```

Start Angie:

```bash
cd /root/tlcpMonoRepo-work
OPENSSL_CONF=/root/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/root/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/root/tlcpMonoRepo-work/pqmagic/install/lib \
/root/tlcpMonoRepo-work/angie-install/sbin/angie \
  -c /root/tlcpMonoRepo-work/angie-work/angie.conf
```

Check process:

```bash
ps -ef | grep angie | grep -v grep
```

Observed process state:

```text
angie: master process v1.12.0
angie: worker process #1
```

## 7. Verify Angie Handshake

Run Tongsuo `s_client` against Angie:

```bash
cd /root/tlcpMonoRepo-work
OPENSSL_CONF=/root/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/root/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/root/tlcpMonoRepo-work/pqmagic/install/lib \
timeout 12 /root/tlcpMonoRepo-work/tongsuo/apps/openssl s_client \
  -connect 127.0.0.1:8443 \
  -enable_ntls -ntls \
  -sign_cert /root/tlcpMonoRepo-work/certs/loose/sign_sm2.crt \
  -sign_key /root/tlcpMonoRepo-work/certs/loose/sign_sm2.key \
  -enc_cert /root/tlcpMonoRepo-work/certs/loose/enc_sm2.crt \
  -enc_key /root/tlcpMonoRepo-work/certs/loose/enc_sm2.key \
  -CAfile /root/tlcpMonoRepo-work/certs/loose/ca_sm2.crt \
  -cipher ECC-KYBER-SM4-GCM-SM3 \
  -brief
```

Observed successful output:

```text
Protocol version: NTLSv1.1
Ciphersuite: ECC-KYBER-SM4-GCM-SM3
Peer certificate: C = CN, ST = Beijing, L = Beijing, O = Test Org, CN = Test Sign Cert
Hash used: SM3
Signature type: SM2
Verification: OK
```

This confirms Angie + Tongsuo + TLCP-PQC completed a dual-certificate NTLS PQC hybrid handshake.

## 8. Demo Server/Client Test

The WSL copy of the demo uses dual-certificate NTLS/PQC APIs.

| File | Key points |
|---|---|
| `demo/demo/server.c` | `NTLS_server_method()`, `SSL_CTX_enable_ntls()`, sign/enc cert APIs, `ECC-KYBER-SM4-GCM-SM3` |
| `demo/demo/client.c` | `NTLS_client_method()`, `SSL_CTX_enable_ntls()`, sign/enc cert APIs, `ECC-KYBER-SM4-GCM-SM3` |
| `demo/demo/mk.sh` | links against `/root/tlcpMonoRepo-work/tongsuo` |

Compile demo:

```bash
cd /root/tlcpMonoRepo-work/demo/demo
TONGSUO_LIB_DIR=/root/tlcpMonoRepo-work/tongsuo \
TONGSUO_INC_DIR=/root/tlcpMonoRepo-work/tongsuo/include \
sh ./mk.sh
```

Server terminal:

```bash
cd /root/tlcpMonoRepo-work/demo/demo
OPENSSL_CONF=/root/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/root/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/root/tlcpMonoRepo-work/pqmagic/install/lib:/root/tlcpMonoRepo-work/tongsuo \
./server
```

Client terminal:

```bash
cd /root/tlcpMonoRepo-work/demo/demo
OPENSSL_CONF=/root/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/root/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/root/tlcpMonoRepo-work/pqmagic/install/lib:/root/tlcpMonoRepo-work/tongsuo \
./client
```

Observed server-side result:

```text
sign cert/key set ok
enc cert/key set ok
cipher set ok: ECC-KYBER-SM4-GCM-SM3
server handshake ok
SSL connection using ECC-KYBER-SM4-GCM-SM3
Received 23 chars:'hello i am from client!'
```

Observed client-side result:

```text
handshake  ok
SSL connection using ECC-KYBER-SM4-GCM-SM3
SSL recv: -----This message is from the SSL server-----.
```

## 9. Stop Angie

```bash
OPENSSL_CONF=/root/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/root/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/root/tlcpMonoRepo-work/pqmagic/install/lib \
/root/tlcpMonoRepo-work/angie-install/sbin/angie \
  -s stop -c /root/tlcpMonoRepo-work/angie-work/angie.conf
```

## 10. Final Status

| Item | Status |
|---|---|
| WSL filesystem project copy | Passed |
| `build-all.sh` base build | Passed |
| provider loading | Passed |
| Angie + Tongsuo NTLS build | Passed |
| Angie config validation | Passed |
| Angie runtime | Passed |
| Angie + `s_client` dual-cert NTLS/PQC handshake | Passed |
| demo server/client dual-cert NTLS/PQC handshake | Passed |

Final verified stack:

```text
Angie/1.12.0 + Tongsuo 8.5.0-dev + TLCP-PQC providers
NTLSv1.1 + SM2 sign/enc dual certificates + ECC-KYBER-SM4-GCM-SM3
```
