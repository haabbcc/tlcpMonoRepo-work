# TLCP-PQC 与 demo 协议库接口差异

本文档对比两个项目中协议库接口的使用情况：

- 项目 1：`TLCP-PQC`
- 项目 2：`demo`

对比范围聚焦在：

- `s_client` / `s_server`
- demo 中的 `client.c` / `server.c` 等示例
- 接口数量
- 接口名称
- 参数形式
- 证书相关差异

## 1. 统计口径

### TLCP-PQC

统计对象：

- `tongsuo/apps/s_client.c`
- `tongsuo/apps/s_server.c`

### demo

统计对象：

- `demo/demo/client.c`
- `demo/demo/server.c`
- `demo/demo/server_tls.c`
- `demo/demo/server_P.c`
- `demo/demo/server_PP.c`

说明：

- 只统计实际调用的协议库接口
- 注释代码和 `#if 0` 中的代码不计入“实际启用”
- 关注 `SSL*`、`SSL_CTX*`、`BIO*`、`X509*`、`EVP*`、`ERR*`、`ENGINE*` 等接口
- `socket`、`printf`、`pthread` 等系统接口不纳入本对比

## 2. 总体结论

| 项目 | 范围 | 特征 |
|---|---|---|
| TLCP-PQC | `s_client.c` + `s_server.c` | 完整的命令行测试工具，覆盖 TLS/DTLS/NTLS、证书链、验证、双证书、PQC 开关、会话、扩展与调试能力 |
| demo | `demo/demo/*.c` | 面向最小联调示例，接口调用少，逻辑直接，便于验证单条握手路径 |

核心差异不是“能不能握手”，而是：

1. TLCP-PQC 更像完整测试框架
2. demo 更像最小样例程序
3. 证书加载层面，TLCP-PQC 更偏向“先解析对象，再设置到 `SSL_CTX`”
4. demo 更偏向“直接用文件接口加载”

## 3. 证书相关差异

这是两个项目差异最大的部分。

### 3.1 普通证书与私钥加载

| 对比项 | TLCP-PQC | demo |
|---|---|---|
| 证书加载方式 | 先读成 `X509 *`，再调用 `SSL_CTX_use_certificate()` 或 helper | 直接调用 `SSL_CTX_use_certificate_file()` |
| 私钥加载方式 | 先读成 `EVP_PKEY *`，再调用 `SSL_CTX_use_PrivateKey()` 或 helper | 直接调用 `SSL_CTX_use_PrivateKey_file()` |
| 证书链 | 支持显式链与自动构链 | 基本未覆盖 |
| 验证源 | 支持 `CAfile`、`CApath`、`CAstore` | 主要是 `CAfile` |

### 3.2 国密双证书

| 对比项 | TLCP-PQC | demo |
|---|---|---|
| 签名证书 | 支持 | 已支持 |
| 加密证书 | 支持 | 已支持 |
| 接口形式 | 对象接口 | 文件接口 |
| 典型接口 | `SSL_CTX_use_sign_certificate()` | `SSL_CTX_use_sign_certificate_file()` |
| 典型接口 | `SSL_CTX_use_enc_certificate()` | `SSL_CTX_use_enc_certificate_file()` |

### 3.3 当前 demo 的状态

`demo/demo/client.c` 和 `demo/demo/server.c` 现在已经切换到 NTLS 双证书接口：

```c
SSL_CTX_use_sign_certificate_file(...)
SSL_CTX_use_sign_PrivateKey_file(...)
SSL_CTX_use_enc_certificate_file(...)
SSL_CTX_use_enc_PrivateKey_file(...)
```

这意味着：

- demo 主路径已经不是“单证书 SM2 示例”
- 而是“基于文件接口的双证书 NTLS 示例”

但它依然与 TLCP-PQC 主工具存在差异：

- TLCP-PQC 主工具大量使用 `X509 *` / `EVP_PKEY *`
- demo 主路径仍使用文件路径作为输入参数

## 4. 协议与握手开关差异

| 对比项 | TLCP-PQC | demo |
|---|---|---|
| NTLS 启用 | 支持 | 已启用 |
| 强制 NTLS | 支持 | 未覆盖 |
| TLS 1.3 国密双证书 | 支持 | 未覆盖 |
| SM2 + Kyber TLS 1.3 | 支持 | 未覆盖 |
| 委托凭证 | 支持 | 未覆盖 |
| OCSP/CRL/DANE | 支持 | 未覆盖 |

demo 当前更聚焦于：

- `NTLS_client_method()`
- `NTLS_server_method()`
- `SSL_CTX_enable_ntls()`
- `ECC-KYBER-SM4-GCM-SM3`

## 5. 参数形式差异

### TLCP-PQC 常见风格

```c
SSL_CTX_use_sign_certificate(ctx, X509 *cert)
SSL_CTX_use_sign_PrivateKey(ctx, EVP_PKEY *pkey)
SSL_CTX_use_enc_certificate(ctx, X509 *cert)
SSL_CTX_use_enc_PrivateKey(ctx, EVP_PKEY *pkey)
```

特点：

- 输入参数是已解析对象
- 便于统一处理证书链、验证和高级选项

### demo 常见风格

```c
SSL_CTX_use_sign_certificate_file(ctx, const char *file, int type)
SSL_CTX_use_sign_PrivateKey_file(ctx, const char *file, int type)
SSL_CTX_use_enc_certificate_file(ctx, const char *file, int type)
SSL_CTX_use_enc_PrivateKey_file(ctx, const char *file, int type)
```

特点：

- 输入参数是文件路径
- 对最小示例更简单直接
- 对扩展场景不如对象接口灵活

## 6. demo 中值得注意的其他变体

### `server_P.c`

该文件包含 SDF engine 私钥加载路径，例如：

```c
"engine:sdf_cipher:2:12345678"
```

这条路径属于：

- engine/硬件设备接入方式
- 与 TLCP-PQC `s_server` 的主路径不是同一层接口风格

### `server_PP.c`

该文件还保留了一些较旧的 engine 和线程锁处理逻辑，更多是历史示例，不代表当前推荐接口路径。

## 7. 推荐结论

### 如果目标是“快速验证握手”

demo 当前形式已经足够：

- 接口少
- 调用路径直
- 适合最小复现

### 如果目标是“与 TLCP-PQC 主工具保持一致”

建议继续向 TLCP-PQC 风格收敛：

1. 先加载成 `X509 *` / `EVP_PKEY *`
2. 通过对象接口设置到 `SSL_CTX`
3. 统一处理证书链、验证源和错误路径

### 如果目标是“验证国密双证书”

当前 demo 主路径已经满足“使用双证书”的要求，但它验证的是：

- 双证书文件接口

而不是：

- TLCP-PQC 主工具的对象接口封装路径

## 8. 一句话总结

TLCP-PQC 是完整的 NTLS/PQC 测试工具链，demo 是最小联调样例；二者最大的接口差异集中在证书加载层，尤其是“双证书是文件接口还是对象接口”这一点。
