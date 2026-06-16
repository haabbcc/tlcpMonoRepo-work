# TLS 码点验证

该流程用于在 TLS Supported Group 码点层验证 SM2+Kyber768 行为。

它会记录：

- `nginx -t` 拒绝 `Groups X25519MLKEM768`
- `nginx -t` 拒绝 `Groups curveSM2MLKEM768`
- `nginx -t` 接受 `Groups SM2:KYBER768`
- 本地 Nginx TLS 1.3 的 `s_client -msg` 日志
- 本地 loopback pcap 与 tshark 解析出的选中 `key_share` group
- ZoTrus 上 `curveSM2MLKEM768`、`X25519MLKEM768` 和 `SM2:KYBER768` 的探测结果

运行：

```bash
cd /root/tlcpMonoRepo-work
bash scripts/TLS_CODEPOINT_VERIFY/verify.sh
```

输出：

```text
logs/tls-codepoint-verify/REPORT.md
logs/local_nginx_sm2_kyber768.pcap
logs/tls-codepoint-verify/*.log
logs/tls-codepoint-verify/*_extract.txt
```

解释规则：

- 如果 ServerHello 选中的 `key_share` group 是 `0x11EE`，说明内部名称 `SM2:KYBER768` 映射到了公开 `curveSM2MLKEM768` 码点。
- 否则，本轮只能证明本地实验路径 `SM2:KYBER768` 可用，不能证明公开 `curveSM2MLKEM768 / 0x11EE` 互通。
