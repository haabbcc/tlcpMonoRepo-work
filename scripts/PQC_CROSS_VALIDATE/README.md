# PQC TLS 交叉验证

运行：

```bash
cd /root/tlcpMonoRepo-work
bash scripts/PQC_CROSS_VALIDATE/run-cross-validation.sh
```

日志和 Markdown 汇总写入：

```text
logs/cross-pqc/
```

该流程用于测试公开生态中的 TLS Supported Group 名称：

```text
X25519MLKEM768 / 0x11EC
curveSM2MLKEM768 / 0x11EE
```

同时记录当前本地实现的回退/实验路径：

```text
SM2:KYBER768 + TLS_SM4_GCM_SM3
```

判断规则：除非抓包或服务端/客户端输出明确显示 `0x11EE`，否则不能把 `SM2:KYBER768` 当作 `curveSM2MLKEM768 / 0x11EE` 互通证据。
