# ZoTrus / 本地 Tongsuo 兼容性矩阵

日期：2026-06-16

测试环境：

- WSL 发行版：`TLCPTest`
- 工作目录：`/root/tlcpMonoRepo-work`
- Tongsuo：`Tongsuo 8.5.0-dev`，OpenSSL library `3.0.3`
- 目标站点：`www.zotrus.com:443`
- 原始证据目录：`/root/tlcpMonoRepo-work/logs/zotrus-matrix-20260616/`

## 摘要

当前本地 Tongsuo 与 ZoTrus 仅在本轮测试的国际 ECDSA TLS 1.3 基线路径上兼容：

- A2：ECDSA 证书 + X25519 + `TLS_AES_128_GCM_SHA256`
- A3：ECDSA 证书 + `secp256r1` + `TLS_AES_128_GCM_SHA256`

当前本地 Tongsuo 与 ZoTrus 的公开 PQC hybrid 组名不兼容。原因不是 ZoTrus 对端握手失败，而是这些公开组名在本地参数解析层已经失败，无法构造对应的 ClientHello：

- `X25519MLKEM768`
- `SecP256r1MLKEM768`
- `SecP384r1MLKEM1024`
- `curveSM2MLKEM768`
- `SM2MLKEM768`

本地实验路径 `SM2:KYBER768` 和 `curveSM2:KYBER768` 不能写成公开 `curveSM2MLKEM768 / 0x11EE` 兼容。抓包显示其线路码点为 `0x0029,0x11EF`，不是 `0x11EE`。

## 一、本地参数解析层

| Group | `openssl s_client -groups` | `nginx ssl_conf_command Groups` |
|---|---:|---:|
| `X25519` | PASS | PASS |
| `secp256r1` | PASS | PASS |
| `curveSM2` | PASS | PASS |
| `SM2` | PASS | PASS |
| `X25519MLKEM768` | FAIL | FAIL |
| `SecP256r1MLKEM768` | FAIL | FAIL |
| `SecP384r1MLKEM1024` | FAIL | FAIL |
| `curveSM2MLKEM768` | FAIL | FAIL |
| `SM2MLKEM768` | FAIL | FAIL |
| `SM2:KYBER768` | PASS | PASS |
| `SM2:MLKEM768` | FAIL | FAIL |
| `curveSM2:KYBER768` | PASS | PASS |
| `curveSM2:MLKEM768` | FAIL | FAIL |

解释：对 ZoTrus 公开 PQC-TLS 兼容性最关键的 hybrid 公开名称，在当前本地 Tongsuo 构建中不能用于构造 ClientHello。

## 二、矩阵测试结果

| ID | 请求组合 | 结果 |
|---|---|---|
| A1 | RSA + X25519 + `TLS_AES_128_GCM_SHA256` | 本地解析 PASS，ClientHello 已发出；强制 RSA 签名算法路径时，ZoTrus 返回 handshake alert。 |
| A2 | ECDSA + X25519 + `TLS_AES_128_GCM_SHA256` | PASS：TLS 1.3，`TLS_AES_128_GCM_SHA256`，ECDSA，verify code 0，HTTP/1.1 200 OK。 |
| A3 | ECDSA + `secp256r1` + `TLS_AES_128_GCM_SHA256` | PASS：TLS 1.3，`TLS_AES_128_GCM_SHA256`，ECDSA，verify code 0，HTTP/1.1 200 OK。 |
| B1 | SM2 + `curveSM2` + `TLS_SM4_GCM_SM3` | ClientHello 使用 `0x0029` 发出；强制 SM2 路径时，ZoTrus 返回 handshake alert。 |
| B2 | SM2 + `SM2` + `TLS_SM4_GCM_SM3` | 与 B1 相同；Tongsuo 将 `SM2` 映射到 `curveSM2 / 0x0029`，随后收到 handshake alert。 |
| C1 | RSA + `X25519MLKEM768` + `TLS_AES_128_GCM_SHA256` | FAIL：本地 group 解析层失败。 |
| C2 | ECDSA + `X25519MLKEM768` + `TLS_AES_128_GCM_SHA256` | FAIL：本地 group 解析层失败。 |
| C3 | ECDSA + `SecP256r1MLKEM768` + `TLS_AES_128_GCM_SHA256` | FAIL：本地 group 解析层失败。 |
| C4 | ECDSA + `SecP384r1MLKEM1024` + `TLS_AES_256_GCM_SHA384` | FAIL：本地 group 解析层失败。 |
| D1 | SM2 + `curveSM2MLKEM768` + `TLS_SM4_GCM_SM3` | FAIL：本地 group 解析层失败。 |
| D2 | SM2 + `SM2MLKEM768` + `TLS_SM4_GCM_SM3` | FAIL：本地 group 解析层失败。 |
| E1 | SM2 + `SM2:KYBER768` + `TLS_SM4_GCM_SM3` + `-enable_sm2_kyber768_tls13` | 本地实验路径发出 `0x0029,0x11EF`，不是 `0x11EE`；ZoTrus 返回 handshake alert。 |
| E2 | `SM2:MLKEM768`、`curveSM2:KYBER768`、`curveSM2:MLKEM768` | `SM2:MLKEM768` 和 `curveSM2:MLKEM768` 在本地解析层失败。`curveSM2:KYBER768` 行为与 E1 相同：发出 `0x0029,0x11EF`，不是 `0x11EE`，随后收到 alert。 |

## 三、线路码点证据

以下为 pcap 中用 `tshark` 提取的原始字段：

| Case | ClientHello supported_groups | ClientHello key_share | ServerHello key_share |
|---|---|---|---|
| A2 | `0x001d` | `29` / X25519 | `29` / X25519 |
| A3 | `0x0017` | `23` / secp256r1 | `23` / secp256r1 |
| B1 | `0x0029` | `41` / curveSM2 | 无；对端 alert |
| B2 | `0x0029` | `41` / curveSM2 | 无；对端 alert |
| E1 | `0x0029,0x11ef` | `41,4591` | 无；对端 alert |
| E2 `curveSM2:KYBER768` | `0x0029,0x11ef` | `41,4591` | 无；对端 alert |

本轮没有观察到以下公开码点：

- `0x11EC`：对应 `X25519MLKEM768`。未观察到，原因是本地组名解析失败。
- `0x11EE`：对应 `curveSM2MLKEM768` / `SM2MLKEM768`。未观察到；公开名称在本地解析失败，实验路径使用的是 `0x11EF`。

## 结论

当前本地 Tongsuo 构建可以与 ZoTrus 的国际 ECDSA TLS 1.3 基线流量互通，但不能与 ZoTrus 的公开 PQC-TLS hybrid group 互通。

主要阻塞点在本地：公开 hybrid group 名称的注册、`SSL_CONF` 解析或 ClientHello 构造尚不支持。`SM2:KYBER768` 只是本地实验路径，不能报告为公开 `curveSM2MLKEM768 / 0x11EE` 兼容。
