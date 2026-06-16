# TLS Supported Group 码点验证

当需要确认本地 Tongsuo/Nginx 的 SM2+Kyber768 路径是否映射到公开 `curveSM2MLKEM768` TLS Supported Group 码点时，使用该流程。

## 运行

```bash
cd /root/tlcpMonoRepo-work
bash scripts/TLS_CODEPOINT_VERIFY/verify.sh
```

## 产物

```text
logs/tls-codepoint-verify/REPORT.md
logs/local_nginx_sm2_kyber768.pcap
logs/tls-codepoint-verify/*.log
logs/tls-codepoint-verify/*_extract.txt
```

## 判断重点

`nginx -t` 接受 `Groups SM2:KYBER768` 只说明 Nginx/Tongsuo 接受这个内部配置名称。

是否兼容公开 SM2+ML-KEM768 生态，必须检查线路上 ServerHello 选中的 `key_share` group。公开预期码点是：

```text
0x11EE
```

如果抓包只显示 `0x0029` 或其他非 `0x11EE` 码点，则只能说明本地实验路径可运行，不能说明公开 `curveSM2MLKEM768 / 0x11EE` 互通。
