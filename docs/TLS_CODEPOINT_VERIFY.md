# TLS Codepoint Verification

Use `scripts/TLS_CODEPOINT_VERIFY/verify.sh` to preserve codepoint-level
evidence for Nginx/Tongsuo SM2+Kyber768 tests.

The script records:

- `nginx -t` failure for `Groups X25519MLKEM768`
- `nginx -t` failure for `Groups curveSM2MLKEM768`
- `nginx -t` success for `Groups SM2:KYBER768`
- local Nginx 4434 `s_client -msg -tlsextdebug` log
- local tcpdump pcap and tshark parse
- ZoTrus tests for `curveSM2MLKEM768`, `X25519MLKEM768`, and `SM2:KYBER768`

The important distinction is:

```text
X25519MLKEM768 / 0x11EC is generic PQC-TLS.
SM2+Kyber/ML-KEM768 must be judged by 0x11EE, or by proving that
SM2:KYBER768 maps to 0x11EE on the wire.
```
