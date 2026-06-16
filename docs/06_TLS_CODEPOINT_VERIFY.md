# TLS Codepoint Verification

Use this workflow when checking whether the local Tongsuo/Nginx SM2+Kyber768
path maps to the public TLS Supported Group codepoint for `curveSM2MLKEM768`.

```bash
cd /root/tlcpMonoRepo-work
bash scripts/TLS_CODEPOINT_VERIFY/verify.sh
```

The report is written to:

```text
logs/tls-codepoint-verify/REPORT.md
```

The pcap is written to:

```text
logs/local_nginx_sm2_kyber768.pcap
```

Important distinction:

`nginx -t` accepting `Groups SM2:KYBER768` only proves that Nginx/Tongsuo accept
the internal configuration name. Compatibility with the public SM2+ML-KEM768
ecosystem requires checking the ServerHello selected key_share group on the
wire. The expected public codepoint is `0x11EE`.

Do not treat `X25519MLKEM768 / 0x11EC` success as SM2+Kyber/ML-KEM768 success.
