# 抗量子 TLCP 单仓交付（tlcpMonoRepo）

本仓库包含从零构建并验证**抗量子 TLCP（国密双证 + 后量子算法）**所需的源码、Provider、测试证书与联调脚本。

当前验收范围：**certs/1（裸 PQC 双证）** 与 **certs/loose（SM2 双证）**。

## 仓库结构

```
tlcpMonoRepo/
├── build-all.sh          # 一键构建
├── env.sh                # 环境变量
├── tongsuo/              # Tongsuo 源码（含 NTLS/PQC 扩展）
├── pqmagic/              # PQMagic 后量子算法库
├── providers/
│   ├── pqmagic-algorithms/   # 裸 PQC SPKI（certs/1）
│   └── ntls-aigis/           # NTLS 用 Aigis-Enc-2 KEM
├── certs/
│   ├── 1/                # 裸 PQC 双证（ML-DSA 签名 + ML-KEM 加密）
│   └── loose/            # SM2 双证（ECC/ECDHE-Kyber 套件）
├── scripts/              # TLCP 联调脚本
├── config/               # Provider 配置模板
└── docs/
    └── TLCP-PQC-BUILD-GUIDE.md
```

## 快速开始

```bash
git clone https://gitcode.com/stella_moment/tlcpMonoRepo.git
cd tlcpMonoRepo

./build-all.sh
source ./env.sh

${OPENSSL} version -a
${OPENSSL} list -providers
```

## TLCP 联调

**certs/1 — KYBER-DILITHIUM-SM4-GCM-SM3：**

```bash
# 终端 1
./scripts/Kyber_Dilithium_SM4_GCM_SM3/client1.sh

# 终端 2
./scripts/Kyber_Dilithium_SM4_GCM_SM3/server1.sh
```

**certs/loose — SM2 + Kyber：**

```bash
# 终端 1
./scripts/ECC_Kyber_SM4_GCM_SM3/server.sh

# 终端 2
./scripts/ECC_Kyber_SM4_GCM_SM3/client.sh
```

详细说明见 [docs/TLCP-PQC-BUILD-GUIDE.md](docs/TLCP-PQC-BUILD-GUIDE.md)。

## 许可证

各子目录沿用上游项目原有许可证。测试证书私钥**仅用于开发联调**，请勿用于生产环境。
