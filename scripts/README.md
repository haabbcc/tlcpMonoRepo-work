# scripts 目录说明

该目录保存 TLCP/PQC 联调脚本、TLS 1.3 PQC 测试脚本和 Angie 集成脚本。

## 子目录

| 路径 | 说明 |
|---|---|
| `Kyber_Dilithium_SM4_GCM_SM3/` | 使用 `certs/1`，测试 `KYBER-DILITHIUM-SM4-GCM-SM3` 的 NTLS/TLCP 双证书路径 |
| `ECC_Kyber_SM4_GCM_SM3/` | 使用 `certs/loose`，测试 `ECC-KYBER-SM4-GCM-SM3` 的 NTLS/TLCP 路径 |
| `ECDHE_Kyber_SM4_GCM_SM3/` | 使用 `certs/loose`，测试 `ECDHE-KYBER-SM4-GCM-SM3` 的 NTLS/TLCP 路径 |
| `TLS13_SM2_KYBER768/` | 测试 TLS 1.3 下的 SM2+Kyber768 混合 key exchange |
| `Angie_TLCP_PQC/` | Angie 编译、配置生成、启动、停止和客户端测试脚本 |

## 使用前提

先在仓库根目录完成核心组件构建：

```bash
cd /root/tlcpMonoRepo-work
./build-all.sh
source ./env.sh
```

部分脚本内部会加载 `env.sh`，但建议在当前 shell 中先手动执行 `source ./env.sh`，这样 Tongsuo、provider 和证书路径更明确。

## TLS 1.3 PQC 测试

TLS 1.3 SM2+Kyber768 混合握手测试入口：

```bash
bash scripts/TLS13_SM2_KYBER768/test.sh
```

该测试会启用：

```text
-tls1_3
-enable_sm_tls13_strict
-enable_sm2_kyber768_tls13
-groups SM2:KYBER768
-ciphersuites TLS_SM4_GCM_SM3
```

验证通过时输出：

```text
TLS 1.3 SM2+Kyber768 hybrid demo test ok
```

## demo TLS 测试

普通 TLS 接口测试不在 `scripts/` 下，而是在：

```bash
demo/demo/test_tls.sh
```

该测试用于验证 `TLS_server_method()` / `TLS_client_method()` 路径，与 `scripts/` 下的 NTLS/TLCP 和 TLS 1.3 PQC 联调脚本互补。

## 相关文档

| 文档 | 内容 |
|---|---|
| `docs/01_BUILD_TLCP_MONOREPO.md` | 编译核心仓库 |
| `docs/02_DEMO_TESTS.md` | 运行 NTLS 和 TLS demo |
| `docs/03_BUILD_ANGIE.md` | 编译 Angie |
| `docs/04_ANGIE_TLCP_MONOREPO_INTEGRATION.md` | Angie 与本项目联调 |
| `docs/05_NTLS_TLS_COMMUNICATION_STATE_MACHINE.md` | NTLS/TLS 通信流程和状态机 |

| `Nginx_TLS_PQC/` | Nginx 1.30.2 build, config rendering, start/stop, and TLS client test scripts |

| `PQC_SUITE_MATRIX/` | Runs the full supported PQC suite handshake matrix and writes logs under `.tmp/pqc-suite-matrix/` |

| `TLS_CODEPOINT_VERIFY/` | TLS Supported Group codepoint verification for local Nginx/Tongsuo and ZoTrus probes |
