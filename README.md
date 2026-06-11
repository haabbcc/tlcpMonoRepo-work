# tlcpMonoRepo

`tlcpMonoRepo` 是一个用于构建和验证 TLCP/NTLS + 国密双证书 + PQC 混合握手的单仓库工程。

仓库当前包含以下几类内容：

- `tongsuo/`：带 NTLS/TLCP 能力的 Tongsuo 源码
- `pqmagic/`：PQC 算法库
- `providers/`：为证书解析和 KEM 注册提供的 OpenSSL provider
- `certs/`：测试证书与私钥
- `scripts/`：握手联调与 Angie 实验脚本
- `demo/`：基于 Tongsuo 接口的最小 client/server 示例
- `docs/`：构建、实验与接口差异文档
- `third_party/angie/`：整理后的 Angie 源码

## 目录结构

```text
tlcpMonoRepo/
├── build-all.sh
├── env.sh
├── certs/
│   ├── 1/
│   └── loose/
├── demo/
├── docs/
├── pqmagic/
├── providers/
│   ├── ntls-aigis/
│   └── pqmagic-algorithms/
├── scripts/
├── third_party/
│   └── angie/
└── tongsuo/
```

## 支持的主要验证场景

1. `certs/1`：PQC 双证书
   - 签名证书：ML-DSA
   - 加密证书：ML-KEM
   - 典型套件：`KYBER-DILITHIUM-SM4-GCM-SM3`

2. `certs/loose`：SM2 双证书
   - 签名证书：SM2 sign
   - 加密证书：SM2 enc
   - 典型套件：
     - `ECC-KYBER-SM4-GCM-SM3`
     - `ECDHE-KYBER-SM4-GCM-SM3`

3. Angie + Tongsuo + TLCP-PQC
   - Angie 作为 NTLS 服务端
   - Tongsuo `s_client` 作为验证客户端
   - 可验证双证书 PQC/国密混合握手

## 快速开始

### 1. 安装依赖

以 Ubuntu 为例：

```bash
sudo apt update
sudo apt install -y build-essential cmake perl git
```

### 2. 构建基础组件

```bash
git clone https://github.com/haabbcc/tlcpMonoRepo-work.git
cd tlcpMonoRepo-work

./build-all.sh
source ./env.sh
```

### 3. 检查构建结果

```bash
${OPENSSL} version -a
${OPENSSL} list -providers
```

预期至少能看到：

- `default`
- `pqmagic`
- `aigis_enc`

## 关键脚本

### TLCP 联调

```bash
# certs/1
./scripts/Kyber_Dilithium_SM4_GCM_SM3/server1.sh
./scripts/Kyber_Dilithium_SM4_GCM_SM3/client1.sh

# certs/loose - ECC
./scripts/ECC_Kyber_SM4_GCM_SM3/server.sh
./scripts/ECC_Kyber_SM4_GCM_SM3/client.sh

# certs/loose - ECDHE
./scripts/ECDHE_Kyber_SM4_GCM_SM3/server.sh
./scripts/ECDHE_Kyber_SM4_GCM_SM3/client.sh
```

### Angie 实验

```bash
bash scripts/Angie_TLCP_PQC/build-stack.sh
bash scripts/Angie_TLCP_PQC/render-conf.sh
bash scripts/Angie_TLCP_PQC/start-angie.sh
bash scripts/Angie_TLCP_PQC/test-client.sh
```

## demo 说明

`demo/demo/client.c` 和 `demo/demo/server.c` 使用 Tongsuo 的 NTLS 双证书接口：

- `NTLS_client_method()`
- `NTLS_server_method()`
- `SSL_CTX_enable_ntls()`
- `SSL_CTX_use_sign_certificate_file()`
- `SSL_CTX_use_sign_PrivateKey_file()`
- `SSL_CTX_use_enc_certificate_file()`
- `SSL_CTX_use_enc_PrivateKey_file()`

可用于最小化验证 `ECC-KYBER-SM4-GCM-SM3` 双证书握手。

`demo/demo/server_tls.c`和- `demo/demo/client_tls.c`使用 Tongsuo 的 TLS 接口：
- `demo/demo/server_tls.c`
- `demo/demo/client_tls.c`
- `demo/demo/test_tls.sh`

这条路径使用 `TLS_server_method()` / `TLS_client_method()` 和普通单证书接口，不启用 NTLS。

## 文档

- [构建指南](docs/TLCP-PQC-BUILD-GUIDE.md)
- [Angie 实验说明](docs/ANGIE_TLCP_PQC_EXPERIMENT.md)

## 备注

- `certs/` 中的证书和私钥仅用于开发与联调。
- `third_party/angie/` 当前保存的是可提交源码树，不包含内部 `.git` 和构建产物。
- 远端仓库曾因 `.gitignore` 误配置漏掉部分 `tongsuo/test` 源文件，现已修复。
