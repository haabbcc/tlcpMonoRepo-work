# demo 测试说明

本文档说明 `demo/demo` 下的独立 server/client 示例如何编译和运行。

## 1. 文件结构

| 文件 | 协议 | 角色 | 作用 |
|---|---|---|---|
| `demo/demo/server.c` | NTLS/TLCP | 服务端 | 国密双证书 PQC/GM server demo |
| `demo/demo/client.c` | NTLS/TLCP | 客户端 | 国密双证书 PQC/GM client demo |
| `demo/demo/server_tls.c` | TLS | 服务端 | 标准 TLS server demo |
| `demo/demo/client_tls.c` | TLS | 客户端 | 标准 TLS client demo |
| `demo/demo/mk.sh` | 编译脚本 | 脚本 | 同时编译 NTLS 和 TLS demo |
| `demo/demo/test_tls.sh` | TLS 测试脚本 | 脚本 | 自动运行 TLS server/client 接口测试 |

## 2. 编译 demo

先编译核心项目：

```bash
cd /root/tlcpMonoRepo-work
./build-all.sh
source ./env.sh
```

再编译 demo：

```bash
cd /root/tlcpMonoRepo-work/demo/demo
./mk.sh
```

`mk.sh` 会生成：

```text
server
client
server_tls
client_tls
```

如果 `demo/certs/` 下没有 TLS 测试证书，脚本会自动生成 TLS CA、服务端证书和客户端证书。

## 3. NTLS demo

NTLS demo 使用 Tongsuo 的 NTLS/TLCP 双证书接口。

服务端接口路径：

```text
NTLS_server_method()
SSL_CTX_new()
SSL_CTX_enable_ntls()
SSL_CTX_use_sign_certificate_file()
SSL_CTX_use_sign_PrivateKey_file()
SSL_CTX_use_enc_certificate_file()
SSL_CTX_use_enc_PrivateKey_file()
SSL_CTX_set_cipher_list("ECC-KYBER-SM4-GCM-SM3")
SSL_CTX_load_verify_locations()
SSL_CTX_set_verify()
SSL_accept()
```

客户端接口路径：

```text
NTLS_client_method()
SSL_CTX_new()
SSL_CTX_enable_ntls()
SSL_CTX_use_sign_certificate_file()
SSL_CTX_use_sign_PrivateKey_file()
SSL_CTX_use_enc_certificate_file()
SSL_CTX_use_enc_PrivateKey_file()
SSL_CTX_load_verify_locations()
SSL_CTX_set_verify()
SSL_CTX_set_cipher_list("ECC-KYBER-SM4-GCM-SM3")
SSL_connect()
```

手动运行服务端：

```bash
cd /root/tlcpMonoRepo-work/demo/demo
./server
```

另开一个终端运行客户端：

```bash
cd /root/tlcpMonoRepo-work/demo/demo
./client
```

默认 NTLS 测试端口：

```text
4433
```

server 和 client 都从 `certs/loose` 读取签名证书和加密证书。

## 4. TLS demo

TLS demo 明确不启用 NTLS，用于验证普通 TLS API 路径。

服务端接口路径：

```text
TLS_server_method()
SSL_CTX_new()
SSL_CTX_use_certificate_file()
SSL_CTX_use_PrivateKey_file()
SSL_CTX_check_private_key()
SSL_CTX_load_verify_locations()
SSL_CTX_set_verify()
SSL_accept()
```

客户端接口路径：

```text
TLS_client_method()
SSL_CTX_new()
SSL_CTX_use_certificate_file()
SSL_CTX_use_PrivateKey_file()
SSL_CTX_check_private_key()
SSL_CTX_load_verify_locations()
SSL_CTX_set_verify()
SSL_connect()
```

运行自动 TLS 接口测试：

```bash
cd /root/tlcpMonoRepo-work/demo/demo
./test_tls.sh
```

期望输出：

```text
TLS interface test ok
```

已验证的 TLS 路径：

```text
TLSv1.3
TLS_AES_256_GCM_SHA384
```

默认 TLS 测试端口：

```text
4443
```

## 5. NTLS 与 TLS demo 差异

| 项目 | NTLS demo | TLS demo |
|---|---|---|
| 源码文件 | `server.c`, `client.c` | `server_tls.c`, `client_tls.c` |
| 协议方法 | `NTLS_server_method()`, `NTLS_client_method()` | `TLS_server_method()`, `TLS_client_method()` |
| NTLS 开关 | `SSL_CTX_enable_ntls()` | 不使用 |
| 证书模型 | 签名证书 + 加密证书 | 单证书 |
| 套件设置 | `ECC-KYBER-SM4-GCM-SM3` | TLS 默认协商 |
| 测试端口 | `4433` | `4443` |
| 测试目的 | 验证国密双证书 PQC/GM NTLS 路径 | 验证标准 TLS 双向认证路径 |
