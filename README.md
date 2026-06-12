# tlcpMonoRepo

`tlcpMonoRepo` 是一个用于构建和测试 TLCP/NTLS、国密双证书以及 PQC/国密混合套件的单仓库工程。项目基于 Tongsuo，包含协议库、PQC 算法库、provider、demo、Angie 适配脚本和测试证书。

## 目录结构

| 路径 | 说明 |
|---|---|
| `tongsuo/` | 带 NTLS/TLCP 能力的 Tongsuo 源码 |
| `pqmagic/` | PQMagic 算法库 |
| `providers/` | 本项目使用的 OpenSSL provider |
| `certs/` | 开发和联调用证书、私钥 |
| `demo/` | 最小化 NTLS/TLS server-client 接口测试 |
| `scripts/` | TLCP/PQC 与 Angie 联调脚本 |
| `third_party/angie/` | 已整理成可提交状态的 Angie 源码 |
| `docs/` | 按主题拆分后的项目文档 |

## 快速开始

```bash
cd /root
git clone https://github.com/haabbcc/tlcpMonoRepo-work.git
cd tlcpMonoRepo-work

sudo apt update
sudo apt install -y build-essential cmake perl git

./build-all.sh
source ./env.sh
${OPENSSL} version -a
${OPENSSL} list -providers
```

## 文档分类

`docs/` 下文档已经按职责拆成 5 个文件：

| 文档 | 内容 |
|---|---|
| [`docs/01_BUILD_TLCP_MONOREPO.md`](docs/01_BUILD_TLCP_MONOREPO.md) | tlcpMonoRepo 编译 |
| [`docs/02_DEMO_TESTS.md`](docs/02_DEMO_TESTS.md) | demo 测试 |
| [`docs/03_ANGIE_TLCP_MONOREPO_INTEGRATION.md`](docs/04_ANGIE_TLCP_MONOREPO_INTEGRATION.md) | Angie + tongsuo + tlcpMonoRepo 测试 |
| [`docs/04_NTLS_TLS_COMMUNICATION_STATE_MACHINE.md`](docs/05_NTLS_TLS_COMMUNICATION_STATE_MACHINE.md) | demo 测试过程分析 |


## 注意事项

- 证书目录中的证书和私钥仅用于开发、实验和联调。
- 构建产物、日志、临时文件不应提交。
