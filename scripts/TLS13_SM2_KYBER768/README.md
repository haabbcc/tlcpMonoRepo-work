# TLS 1.3 SM2+Kyber768 混合握手测试

这组脚本用于验证当前 Tongsuo 是否支持 TLS 1.3 PQC 混合密钥交换。

当前仓库中的 Tongsuo 源码已经暴露：

- `s_server -enable_sm2_kyber768_tls13`
- `s_client -enable_sm2_kyber768_tls13`
- `SSL_CTX_enable_sm2_kyber768_tls13()`

该模式不是 NTLS/TLCP 双证书模式，而是 TLS 1.3 下的 SM2 + Kyber768 混合密钥交换。客户端会发送两个 `KeyShareEntry`：`SM2` 和 `KYBER768`。服务端处理 SM2 经典共享密钥，同时对 Kyber768 公钥封装，并把两个共享密钥组合后进入 TLS 1.3 密钥派生流程。

## 运行

先完成核心构建并加载环境：

```bash
cd /root/tlcpMonoRepo-work
./build-all.sh
source ./env.sh
```

自动测试：

```bash
bash scripts/TLS13_SM2_KYBER768/test.sh
```

期望输出：

```text
TLS 1.3 SM2+Kyber768 hybrid demo test ok
```

## 手动测试

终端 1：

```bash
bash scripts/TLS13_SM2_KYBER768/server.sh
```

终端 2：

```bash
bash scripts/TLS13_SM2_KYBER768/client.sh
```

默认端口是 `4453`，可以通过 `TLS13_PQ_PORT` 修改。

## 验证点

`test.sh` 会检查：

- 客户端日志出现 `TLSv1.3`
- 客户端日志出现 `TLS_SM4_GCM_SM3`
- 服务端和客户端日志出现 `SM2KYBER-DBG`
- 服务端和客户端日志出现 Kyber 相关调试信息

`SM2KYBER-DBG` 来自 Tongsuo 源码中的 `TS_HYBRID_KEX_DEBUG=1` 调试开关，可证明握手进入了 SM2+Kyber768 混合路径。
