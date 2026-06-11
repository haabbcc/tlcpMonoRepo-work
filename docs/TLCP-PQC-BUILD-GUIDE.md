# TLCP-PQC 构建指南

本文档说明如何在 Linux/WSL 环境中构建 `tlcpMonoRepo`，并完成基础的 TLCP/NTLS + PQC 联调验证。

## 1. 目标

本项目的目标是提供一套可重复的实验环境，用于验证：

- TLCP/NTLS 协议栈
- 国密双证书
- PQC provider
- PQC/国密混合密钥交换
- Tongsuo `s_client` / `s_server`
- demo 程序与 Angie 服务端联调

## 2. 环境要求

建议环境：

- Ubuntu / Debian / WSL2
- `gcc`
- `make`
- `cmake`
- `perl`
- `git`

安装命令：

```bash
sudo apt update
sudo apt install -y build-essential cmake perl git
```

## 3. 仓库准备

```bash
git clone https://github.com/haabbcc/tlcpMonoRepo-work.git
cd tlcpMonoRepo-work
```

如果之前拉取过旧版本，建议先确认分支和提交：

```bash
git checkout main
git pull
git rev-parse HEAD
```

## 4. 一键构建

```bash
./build-all.sh
```

`build-all.sh` 会依次完成：

1. 构建 `pqmagic`
2. 构建 `tongsuo`
3. 构建 `providers/pqmagic-algorithms`
4. 构建 `providers/ntls-aigis`
5. 生成 `config/openssl-providers.cnf`

默认 Tongsuo 配置为：

```bash
linux-x86_64 enable-ntls
```

如果是其他架构，可自行覆盖：

```bash
TONGSUO_CONFIGURE="linux-aarch64 enable-ntls" ./build-all.sh
```

## 5. 加载运行环境

```bash
source ./env.sh
```

`env.sh` 会设置：

- `TLCP_ROOT`
- `TONGSUO_ROOT`
- `OPENSSL`
- `PQMAGIC_PREFIX`
- `LD_LIBRARY_PATH`
- `OPENSSL_MODULES`
- `OPENSSL_CONF`

## 6. 构建后检查

### 6.1 OpenSSL/Tongsuo

```bash
${OPENSSL} version -a
```

### 6.2 Provider

```bash
${OPENSSL} list -providers
```

预期至少包含：

- `default`
- `pqmagic`
- `aigis_enc`

### 6.3 证书解析

```bash
${OPENSSL} x509 -in certs/1/user_sig.crt -text -noout
${OPENSSL} x509 -in certs/loose/sign_sm2.crt -text -noout
```

如果 `certs/1` 证书能正确解析，通常说明 `pqmagic` provider 已生效。

## 7. TLCP 联调脚本

### 7.1 `certs/1`：PQC 双证书

服务端：

```bash
./scripts/Kyber_Dilithium_SM4_GCM_SM3/server1.sh
```

客户端：

```bash
./scripts/Kyber_Dilithium_SM4_GCM_SM3/client1.sh
```

目标套件：

```text
KYBER-DILITHIUM-SM4-GCM-SM3
```

### 7.2 `certs/loose`：SM2 双证书

ECC 版本：

```bash
./scripts/ECC_Kyber_SM4_GCM_SM3/server.sh
./scripts/ECC_Kyber_SM4_GCM_SM3/client.sh
```

ECDHE 版本：

```bash
./scripts/ECDHE_Kyber_SM4_GCM_SM3/server.sh
./scripts/ECDHE_Kyber_SM4_GCM_SM3/client.sh
```

目标套件：

```text
ECC-KYBER-SM4-GCM-SM3
ECDHE-KYBER-SM4-GCM-SM3
```

## 8. demo 构建

进入 demo 目录：

```bash
cd demo/demo
./mk.sh
```

当前 `mk.sh` 默认链接仓库里的：

- `tongsuo/include`
- `tongsuo/`

生成：

- `server`
- `client`

### 8.1 运行 demo

服务端：

```bash
./server
```

客户端：

```bash
./client
```

demo 目前已经切换到 NTLS 双证书接口，目标套件是：

```text
ECC-KYBER-SM4-GCM-SM3
```

### 8.2 运行普通 TLS 接口测试

除 NTLS 双证书 demo 外，`demo/demo` 还提供普通 TLS 接口测试：

```bash
cd demo/demo
./test_tls.sh
```

该测试会：

1. 自动生成 TLS 测试 CA、服务端证书和客户端证书
2. 编译 `server_tls` 与 `client_tls`
3. 启动 TLS 服务端
4. 使用 TLS 客户端完成双向证书握手和数据收发

该路径使用普通 TLS 接口：

```c
TLS_server_method()
TLS_client_method()
SSL_CTX_use_certificate_file()
SSL_CTX_use_PrivateKey_file()
```

它不调用 `SSL_CTX_enable_ntls()`，也不使用国密双证书接口。

## 9. 常见问题

### 9.1 `Makefile wasn't produced`

如果 `./Configure` 报某些 `tongsuo/test/*.c` 缺失，优先检查：

```bash
git pull
git rev-parse HEAD
```

该问题曾由远端仓库中 `tongsuo/test` 源文件缺失引起，现已修复。

### 9.2 `apps/openssl: No such file`

说明 `tongsuo` 尚未成功编译。

### 9.3 `Unable to load Public Key`

通常表示 provider 环境未加载：

```bash
source ./env.sh
${OPENSSL} list -providers
```

### 9.4 握手失败

重点检查：

- 双证书路径是否正确
- `OPENSSL_CONF` 是否正确
- `OPENSSL_MODULES` 是否正确
- `LD_LIBRARY_PATH` 是否正确
- 客户端与服务端 cipher 是否一致

## 10. 建议验收项

- `./build-all.sh` 成功完成
- `${OPENSSL} list -providers` 正常
- `certs/1` 与 `certs/loose` 证书可解析
- `scripts/` 中至少一组 TLCP 握手成功
- `demo/demo/server` 与 `demo/demo/client` 握手成功
