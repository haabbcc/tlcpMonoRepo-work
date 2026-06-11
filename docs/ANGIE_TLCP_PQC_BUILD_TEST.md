# Angie + Tongsuo + TLCP-PQC 构建与测试运行手册

本文档记录了在 WSL 中构建、部署和验证 Angie + Tongsuo + tlcpMonoRepo 的流程。

## 1. 环境

所有构建和运行时文件均保存在 WSL 文件系统内：

```bash
/home/houzhiqing/tlcpMonoRepo-work
```

| 项目 | 值 |
|---|---|
| WSL 发行版 | `TLCPTest` |
| 项目根目录 | `/home/houzhiqing/tlcpMonoRepo-work` |
| Tongsuo | `Tongsuo 8.5.0-dev`，OpenSSL `3.0.3` |
| Angie | `Angie/1.12.0` |
| 协议 | `NTLSv1.1` |
| 密码套件 | `ECC-KYBER-SM4-GCM-SM3` |
| 证书 | `/home/houzhiqing/tlcpMonoRepo-work/certs/loose` |

## 2. 构建 TLCP-PQC 组件

运行仓库构建脚本：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work
JOBS=4 bash ./build-all.sh
```

该脚本会构建以下组件：

| 组件 | 输出 |
|---|---|
| PQMagic | `/home/houzhiqing/tlcpMonoRepo-work/pqmagic/install` |
| Tongsuo | `/home/houzhiqing/tlcpMonoRepo-work/tongsuo` |
| pqmagic provider | `/home/houzhiqing/tlcpMonoRepo-work/providers/pqmagic-algorithms/build/pqmagic.so` |
| aigis_enc provider | `/home/houzhiqing/tlcpMonoRepo-work/providers/ntls-aigis/build/aigis_enc.so` |
| provider 配置 | `/home/houzhiqing/tlcpMonoRepo-work/config/openssl-providers.cnf` |

验证 provider 加载情况：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work
OPENSSL_CONF=/home/houzhiqing/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/home/houzhiqing/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/home/houzhiqing/tlcpMonoRepo-work/tongsuo:/home/houzhiqing/tlcpMonoRepo-work/pqmagic/install/lib \
/home/houzhiqing/tlcpMonoRepo-work/tongsuo/apps/openssl list -providers
```

预期处于激活状态的 providers：

```text
default
pqmagic
aigis_enc
```

## 3. 获取 Angie 源码

在该环境中，`git clone` 不稳定，因此使用 GitHub tarball 的方式获取源码：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work
mkdir -p third_party
curl -L --retry 5 --retry-delay 3 --http1.1 \
  -o third_party/angie.tar.gz \
  https://github.com/webserver-llc/angie/archive/refs/heads/master.tar.gz
mkdir -p third_party/angie
tar -xzf third_party/angie.tar.gz -C third_party/angie --strip-components=1
```

测试所用源码报告版本为 `Angie/1.12.0`。

## 4. 使用 Tongsuo NTLS 构建 Angie

```bash
cd /home/houzhiqing/tlcpMonoRepo-work/third_party/angie
./configure \
  --prefix=/home/houzhiqing/tlcpMonoRepo-work/angie-install \
  --conf-path=/home/houzhiqing/tlcpMonoRepo-work/angie-work/angie.conf \
  --pid-path=/home/houzhiqing/tlcpMonoRepo-work/angie-work/angie.pid \
  --error-log-path=/home/houzhiqing/tlcpMonoRepo-work/angie-work/error.log \
  --http-log-path=/home/houzhiqing/tlcpMonoRepo-work/angie-work/access.log \
  --with-http_ssl_module \
  --with-http_v2_module \
  --with-stream \
  --with-stream_ssl_module \
  --with-ntls \
  --with-openssl=/home/houzhiqing/tlcpMonoRepo-work/tongsuo \
  --with-openssl-opt=enable-ntls
make -j4
make install
```

验证构建选项：

```bash
/home/houzhiqing/tlcpMonoRepo-work/angie-install/sbin/angie -V
```

输出中应包含：

```text
--with-ntls
--with-openssl=/home/houzhiqing/tlcpMonoRepo-work/tongsuo
--with-openssl-opt=enable-ntls
```

## 5. Angie 配置

配置文件路径：

```bash
/home/houzhiqing/tlcpMonoRepo-work/angie-work/angie.conf
```

可工作的最小配置：

```nginx
worker_processes 1;
error_log /home/houzhiqing/tlcpMonoRepo-work/angie-work/error.log debug;
pid /home/houzhiqing/tlcpMonoRepo-work/angie-work/angie.pid;

events {
    worker_connections 1024;
}

http {
    access_log /home/houzhiqing/tlcpMonoRepo-work/angie-work/access.log;

    server {
        listen 127.0.0.1:8443 ssl;
        server_name localhost;
        ssl_ntls on;

        ssl_certificate     sign:/home/houzhiqing/tlcpMonoRepo-work/certs/loose/sign_sm2.crt;
        ssl_certificate_key sign:/home/houzhiqing/tlcpMonoRepo-work/certs/loose/sign_sm2.key;
        ssl_certificate     enc:/home/houzhiqing/tlcpMonoRepo-work/certs/loose/enc_sm2.crt;
        ssl_certificate_key enc:/home/houzhiqing/tlcpMonoRepo-work/certs/loose/enc_sm2.key;

        ssl_client_certificate /home/houzhiqing/tlcpMonoRepo-work/certs/loose/ca_sm2.crt;
        ssl_verify_client optional;
        ssl_ciphers ECC-KYBER-SM4-GCM-SM3;

        location / {
            return 200 angie_tlcp_pqc_ok;
        }
    }
}
```

Angie 使用 `sign:` 和 `enc:` 前缀配置 NTLS 双证书。源码检查确认，这些前缀会映射到 Tongsuo API：

```text
SSL_CTX_use_sign_certificate()
SSL_CTX_use_sign_PrivateKey()
SSL_CTX_use_enc_certificate()
SSL_CTX_use_enc_PrivateKey()
```

## 6. 验证并启动 Angie

验证配置：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work
OPENSSL_CONF=/home/houzhiqing/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/home/houzhiqing/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/home/houzhiqing/tlcpMonoRepo-work/pqmagic/install/lib \
/home/houzhiqing/tlcpMonoRepo-work/angie-install/sbin/angie \
  -t -c /home/houzhiqing/tlcpMonoRepo-work/angie-work/angie.conf
```

观察到的结果：

```text
syntax is ok
configuration file ... test is successful
```

启动 Angie：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work
OPENSSL_CONF=/home/houzhiqing/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/home/houzhiqing/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/home/houzhiqing/tlcpMonoRepo-work/pqmagic/install/lib \
/home/houzhiqing/tlcpMonoRepo-work/angie-install/sbin/angie \
  -c /home/houzhiqing/tlcpMonoRepo-work/angie-work/angie.conf
```

检查进程：

```bash
ps -ef | grep angie | grep -v grep
```

观察到的进程状态：

```text
angie: master process v1.12.0
angie: worker process #1
```

## 7. 验证 Angie 握手

使用 Tongsuo `s_client` 连接 Angie：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work
OPENSSL_CONF=/home/houzhiqing/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/home/houzhiqing/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/home/houzhiqing/tlcpMonoRepo-work/pqmagic/install/lib \
timeout 12 /home/houzhiqing/tlcpMonoRepo-work/tongsuo/apps/openssl s_client \
  -connect 127.0.0.1:8443 \
  -enable_ntls -ntls \
  -sign_cert /home/houzhiqing/tlcpMonoRepo-work/certs/loose/sign_sm2.crt \
  -sign_key /home/houzhiqing/tlcpMonoRepo-work/certs/loose/sign_sm2.key \
  -enc_cert /home/houzhiqing/tlcpMonoRepo-work/certs/loose/enc_sm2.crt \
  -enc_key /home/houzhiqing/tlcpMonoRepo-work/certs/loose/enc_sm2.key \
  -CAfile /home/houzhiqing/tlcpMonoRepo-work/certs/loose/ca_sm2.crt \
  -cipher ECC-KYBER-SM4-GCM-SM3 \
  -brief
```

观察到的成功输出：

```text
Protocol version: NTLSv1.1
Ciphersuite: ECC-KYBER-SM4-GCM-SM3
Peer certificate: C = CN, ST = Beijing, L = Beijing, O = Test Org, CN = Test Sign Cert
Hash used: SM3
Signature type: SM2
Verification: OK
```

这确认 Angie + Tongsuo + TLCP-PQC 已完成一次基于双证书的 NTLS/PQC 混合握手。

## 8. Demo 服务端/客户端测试

WSL 副本中的 demo 使用双证书 NTLS/PQC API。

| 文件 | 关键点 |
|---|---|
| `demo/demo/server.c` | `NTLS_server_method()`、`SSL_CTX_enable_ntls()`、签名/加密证书 API、`ECC-KYBER-SM4-GCM-SM3` |
| `demo/demo/client.c` | `NTLS_client_method()`、`SSL_CTX_enable_ntls()`、签名/加密证书 API、`ECC-KYBER-SM4-GCM-SM3` |
| `demo/demo/mk.sh` | 链接到 `/home/houzhiqing/tlcpMonoRepo-work/tongsuo` |

编译 demo：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work/demo/demo
TONGSUO_LIB_DIR=/home/houzhiqing/tlcpMonoRepo-work/tongsuo \
TONGSUO_INC_DIR=/home/houzhiqing/tlcpMonoRepo-work/tongsuo/include \
sh ./mk.sh
```

服务端终端：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work/demo/demo
OPENSSL_CONF=/home/houzhiqing/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/home/houzhiqing/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/home/houzhiqing/tlcpMonoRepo-work/pqmagic/install/lib:/home/houzhiqing/tlcpMonoRepo-work/tongsuo \
./server
```

客户端终端：

```bash
cd /home/houzhiqing/tlcpMonoRepo-work/demo/demo
OPENSSL_CONF=/home/houzhiqing/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/home/houzhiqing/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/home/houzhiqing/tlcpMonoRepo-work/pqmagic/install/lib:/home/houzhiqing/tlcpMonoRepo-work/tongsuo \
./client
```

观察到的服务端结果：

```text
sign cert/key set ok
enc cert/key set ok
cipher set ok: ECC-KYBER-SM4-GCM-SM3
server handshake ok
SSL connection using ECC-KYBER-SM4-GCM-SM3
Received 23 chars:'hello i am from client!'
```

观察到的客户端结果：

```text
handshake  ok
SSL connection using ECC-KYBER-SM4-GCM-SM3
SSL recv: -----This message is from the SSL server-----.
```

## 9. 停止 Angie

```bash
OPENSSL_CONF=/home/houzhiqing/tlcpMonoRepo-work/config/openssl-providers.cnf \
OPENSSL_MODULES=/home/houzhiqing/tlcpMonoRepo-work/providers/pqmagic-algorithms/build \
LD_LIBRARY_PATH=/home/houzhiqing/tlcpMonoRepo-work/pqmagic/install/lib \
/home/houzhiqing/tlcpMonoRepo-work/angie-install/sbin/angie \
  -s stop -c /home/houzhiqing/tlcpMonoRepo-work/angie-work/angie.conf
```

## 10. 最终状态

| 项目 | 状态 |
|---|---|
| WSL 文件系统项目副本 | 通过 |
| `build-all.sh` 基础构建 | 通过 |
| provider 加载 | 通过 |
| Angie + Tongsuo NTLS 构建 | 通过 |
| Angie 配置验证 | 通过 |
| Angie 运行时 | 通过 |
| Angie + `s_client` 双证书 NTLS/PQC 握手 | 通过 |
| demo 服务端/客户端双证书 NTLS/PQC 握手 | 通过 |

最终验证的技术栈：

```text
Angie/1.12.0 + Tongsuo 8.5.0-dev + TLCP-PQC providers
NTLSv1.1 + SM2 sign/enc dual certificates + ECC-KYBER-SM4-GCM-SM3
```
