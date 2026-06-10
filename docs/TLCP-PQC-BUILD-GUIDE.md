# 抗量子 TLCP 从零构建指南

本文档说明如何在本仓库中从零编译全部组件，并使用 **certs/1** 与 **SM2（certs/loose）** 完成抗量子 TLCP 验收。

---

## 1. 方案概述

### 1.1 目标

在 GB/T 38636-2020 TLCP（国密双证）框架下，集成后量子密码（PQC）算法，实现抗量子传输层密码协议联调。

### 1.2 架构

```
┌─────────────────────────────────────────────────────────┐
│         s_server / s_client（Tongsuo apps/openssl）      │
└────────────────────────┬────────────────────────────────┘
                         │ NTLS 状态机
┌────────────────────────▼────────────────────────────────┐
│  密码套件：                                               │
│  • KYBER-DILITHIUM-SM4-GCM-SM3  （certs/1）              │
│  • ECC-KYBER-SM4-GCM-SM3        （certs/loose，SM2）     │
│  • ECDHE-KYBER-SM4-GCM-SM3      （certs/loose，SM2）     │
└────────────────────────┬────────────────────────────────┘
                         │
        ┌────────────────┴────────────────┐
        ▼                                 ▼
  pqmagic-algorithms                  ntls-aigis
  （certs/1 证书解析）                 （Aigis-Enc KEM）
        │                                 │
        └────────────────┬────────────────┘
                         ▼
                    PQMagic 算法库
                         ▼
                    Tongsuo libcrypto/libssl
```

### 1.3 组件清单

| 目录 | 作用 |
|------|------|
| `tongsuo/` | 修改版 Tongsuo，启用 NTLS，扩展 Kyber/Dilithium/Aigis 密码套件 |
| `pqmagic/` | 后量子算法库（ML-DSA、ML-KEM、Aigis-Enc 等） |
| `providers/pqmagic-algorithms/` | 解析 certs/1 裸 PQC 公钥 SPKI |
| `providers/ntls-aigis/` | 注册 Aigis-Enc-2 KEM，供 NTLS 临时密钥交换 |
| `certs/1/` | 裸 PQC 双证（ML-DSA 签名 + ML-KEM 加密） |
| `certs/loose/` | SM2 双证，用于 ECC/ECDHE-Kyber 套件 |
| `scripts/` | TLCP 联调脚本 |

---

## 2. 环境要求

| 项目 | 要求 |
|------|------|
| 操作系统 | Linux x86_64 |
| 编译器 | gcc |
| 构建工具 | make、cmake ≥ 3.10、perl |

```bash
sudo apt update
sudo apt install -y build-essential cmake perl git
```

---

## 3. 获取源码

```bash
git clone https://gitcode.com/stella_moment/tlcpMonoRepo.git
cd tlcpMonoRepo
```

---

## 4. 构建

### 4.1 一键构建

```bash
./build-all.sh
```

构建顺序：PQMagic → Tongsuo → pqmagic-algorithms provider → ntls-aigis provider。

非 x86_64 平台：

```bash
TONGSUO_CONFIGURE="linux-aarch64 enable-ntls" ./build-all.sh
```

### 4.2 加载环境

```bash
source ./env.sh
```

---

## 5. 构建验收

```bash
source ./env.sh

${OPENSSL} version -a
${OPENSSL} list -providers
# 应看到 default、pqmagic、aigis_enc

${OPENSSL} x509 -in certs/1/user_sig.crt -text -noout
# Public Key 应正常显示
```

---

## 6. TLCP 联调

### 6.1 密码套件与证书对照

| 脚本 | 密码套件 | 证书 |
|------|---------|------|
| `Kyber_Dilithium_SM4_GCM_SM3/client1.sh` + `server1.sh` | KYBER-DILITHIUM-SM4-GCM-SM3 | certs/1 |
| `ECC_Kyber_SM4_GCM_SM3/` | ECC-KYBER-SM4-GCM-SM3 | certs/loose（SM2） |
| `ECDHE_Kyber_SM4_GCM_SM3/` | ECDHE-KYBER-SM4-GCM-SM3 | certs/loose（SM2） |

### 6.2 certs/1 — KYBER-DILITHIUM

**终端 1（服务端）：**

```bash
./scripts/Kyber_Dilithium_SM4_GCM_SM3/client1.sh
```

**终端 2（客户端）：**

```bash
./scripts/Kyber_Dilithium_SM4_GCM_SM3/server1.sh
```

成功标志：客户端 `Verify return code: 0 (ok)`，Cipher 为 `KYBER-DILITHIUM-SM4-GCM-SM3`。

### 6.3 certs/loose — SM2 + Kyber

```bash
# 终端 1
./scripts/ECC_Kyber_SM4_GCM_SM3/server.sh

# 终端 2
./scripts/ECC_Kyber_SM4_GCM_SM3/client.sh
```

ECDHE 版本将目录名中的 `ECC_Kyber` 换为 `ECDHE_Kyber` 即可。

---

## 7. 测试证书说明

| 目录 | 类型 | 用途 |
|------|------|------|
| `certs/1/` | 裸 PQC 双证 | KYBER-DILITHIUM 套件 |
| `certs/loose/` | SM2 双证 | ECC/ECDHE-Kyber 套件 |

私钥仅用于测试联调，禁止用于生产。

---

## 8. 常见问题

**`libpqmagic_std not found`** — 重新运行 `./build-all.sh`。

**`apps/openssl: No such file`** — Tongsuo 未编译，进入 `tongsuo/` 执行 `./Configure linux-x86_64 enable-ntls && make`。

**证书 `Unable to load Public Key`** — 确认已 `source env.sh`，且 `list -providers` 包含 pqmagic。

**TLCP 握手失败** — 检查双证路径、密码套件字符串、`LD_LIBRARY_PATH`。

---

## 9. 审查验收清单

- [ ] `./build-all.sh` 无报错完成
- [ ] `${OPENSSL} list -providers` 显示 default、pqmagic、aigis_enc
- [ ] `certs/1/user_sig.crt` 可 `x509 -text` 正常解析
- [ ] KYBER-DILITHIUM TLCP 握手成功（Verify return code: 0）
- [ ] （可选）ECC-KYBER 或 ECDHE-KYBER 握手成功

---

## 10. 参考

- GB/T 38636-2020 TLCP
- Tongsuo：https://github.com/Tongsuo-Project/Tongsuo
- PQMagic：https://pqcrypto.dev/
