# TLS codepoint verification

This workflow verifies SM2+Kyber768 behavior at the TLS Supported Group codepoint level.

It records:

- `nginx -t` rejection for `Groups X25519MLKEM768`
- `nginx -t` rejection for `Groups curveSM2MLKEM768`
- `nginx -t` success for `Groups SM2:KYBER768`
- local Nginx TLS 1.3 `s_client -msg` logs
- local loopback pcap and tshark selected key_share group parse
- ZoTrus tests for `curveSM2MLKEM768`, `X25519MLKEM768`, and `SM2:KYBER768`

Run:

```bash
cd /root/tlcpMonoRepo-work
bash scripts/TLS_CODEPOINT_VERIFY/verify.sh
```

Outputs:

```text
logs/tls-codepoint-verify/REPORT.md
logs/local_nginx_sm2_kyber768.pcap
logs/tls-codepoint-verify/*.log
logs/tls-codepoint-verify/*_extract.txt
```

Interpretation rule:

- If ServerHello selected key_share group is `0x11EE`, the internal `SM2:KYBER768` name maps to the public `curveSM2MLKEM768` codepoint.
- Otherwise, the run only validates the local experimental `SM2:KYBER768` path and does not prove public `curveSM2MLKEM768 / 0x11EE` interoperability.
