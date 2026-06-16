# TLS 码点验证快速入口

该文档指向 `scripts/TLS_CODEPOINT_VERIFY/` 工作流，用于生成本地 Nginx/Tongsuo SM2+Kyber768 测试的码点证据。

```bash
cd /root/tlcpMonoRepo-work
bash scripts/TLS_CODEPOINT_VERIFY/verify.sh
```

该流程会记录：

- `nginx -t` 对不同 `Groups` 名称的接受或拒绝结果
- 本地 `s_client` 日志
- 本地 tcpdump pcap 与 tshark 解析结果
- ZoTrus 上 `curveSM2MLKEM768`、`X25519MLKEM768` 和 `SM2:KYBER768` 的探测结果

结论报告写入：

```text
logs/tls-codepoint-verify/REPORT.md
```

注意：只有抓包看到 `0x11EE`，才能说明公开 `curveSM2MLKEM768` 码点路径成立。
