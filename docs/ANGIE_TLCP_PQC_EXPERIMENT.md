# Angie + Tongsuo + TLCP-PQC 实验说明

本文档记录如何基于本仓库完成：

- Tongsuo 构建
- PQC provider 构建
- Angie 编译与配置
- 双证书 NTLS/PQC 混合握手验证

## 1. 实验目标

目标是验证以下链路：

```text
Tongsuo s_client  <->  Angie(链接本仓库 Tongsuo)
```

验证重点：

- `NTLSv1.1`
- 国密双证书
- `ECC-KYBER-SM4-GCM-SM3`
- provider 环境是否对 Angie 生效

## 2. 构建步骤

### 2.1 构建基础组件

```bash
bash scripts/Angie_TLCP_PQC/build-stack.sh
```

该脚本会执行两步：

1. `build-all.sh`
2. `scripts/Angie_TLCP_PQC/build-angie.sh`

### 2.2 单独构建 Angie

如果基础组件已构建，也可以单独执行：

```bash
bash scripts/Angie_TLCP_PQC/build-angie.sh
```

构建时使用：

```text
--with-http_ssl_module
--with-ntls
--with-openssl=<repo>/tongsuo
```

## 3. 生成配置并启动

### 3.1 生成配置

```bash
bash scripts/Angie_TLCP_PQC/render-conf.sh
```

配置模板：

- `config/angie-tlcp-pqc.conf.in`

运行配置输出位置：

- `angie-work/angie.conf`

### 3.2 启动 Angie

```bash
bash scripts/Angie_TLCP_PQC/start-angie.sh
```

默认监听：

```text
127.0.0.1:8443
```

## 4. 核心配置项

典型配置包含：

```nginx
ssl_ntls on;

ssl_certificate     sign:/root/tlcpMonoRepo-work/certs/loose/sign_sm2.crt;
ssl_certificate_key sign:/root/tlcpMonoRepo-work/certs/loose/sign_sm2.key;

ssl_certificate     enc:/root/tlcpMonoRepo-work/certs/loose/enc_sm2.crt;
ssl_certificate_key enc:/root/tlcpMonoRepo-work/certs/loose/enc_sm2.key;

ssl_ciphers ECC-KYBER-SM4-GCM-SM3;
```

说明：

- `sign:` 表示签名证书
- `enc:` 表示加密证书
- 这是 Tongsuo/NTLS 的双证书加载方式

## 5. 客户端验证

```bash
bash scripts/Angie_TLCP_PQC/test-client.sh
```

该脚本内部使用 `s_client` 做协议级验证，典型参数如下：

```bash
${OPENSSL} s_client \
  -connect 127.0.0.1:8443 \
  -ntls -enable_ntls \
  -sign_cert certs/loose/sign_sm2.crt \
  -sign_key certs/loose/sign_sm2.key \
  -enc_cert certs/loose/enc_sm2.crt \
  -enc_key certs/loose/enc_sm2.key \
  -CAfile certs/loose/ca_sm2.crt \
  -cipher ECC-KYBER-SM4-GCM-SM3
```

### 5.1 预期结果

握手成功时，输出中应看到类似内容：

```text
Protocol version: NTLSv1.1
Ciphersuite: ECC-KYBER-SM4-GCM-SM3
Verification: OK
```

## 6. 停止 Angie

```bash
bash scripts/Angie_TLCP_PQC/stop-angie.sh
```

## 7. 运行环境要求

Angie 运行时必须继承以下环境：

```bash
export LD_LIBRARY_PATH="${TONGSUO_ROOT}:${PQMAGIC_LIB}:${LD_LIBRARY_PATH:-}"
export OPENSSL_CONF="${TLCP_ROOT}/config/openssl-providers.cnf"
export OPENSSL_MODULES="${TLCP_ROOT}/providers/pqmagic-algorithms/build"
```

否则常见问题包括：

- provider 加载失败
- `certs/1` 证书无法识别
- PQC cipher 无法协商

## 8. 实验结论

本项目当前实验路径可用于验证：

- Angie 已适配 Tongsuo NTLS 接口
- Angie 可加载双证书
- Tongsuo `s_client` 可与 Angie 完成 `NTLSv1.1` 握手
- `ECC-KYBER-SM4-GCM-SM3` 可作为验证套件

如果需要继续验证 Angie 客户端侧能力，下一步应搭建：

```text
Angie A (proxy/client) -> Angie B (server)
```

而不是只做 `s_client -> Angie` 单臂测试。
