# PQC suite matrix

Run all currently exposed PQC-capable Tongsuo suites:

```bash
cd /root/tlcpMonoRepo-work
bash scripts/PQC_SUITE_MATRIX/test-all.sh
```

Covered suites:

| Suite | Protocol | Certificate model |
|---|---|---|
| `ECC-KYBER-SM4-GCM-SM3` | NTLSv1.1 | `certs/loose` SM2 dual certificate |
| `ECDHE-KYBER-SM4-GCM-SM3` | NTLSv1.1 | `certs/loose` SM2 dual certificate |
| `KYBER-DILITHIUM-SM4-GCM-SM3` | NTLSv1.1 | `certs/1` PQC dual certificate |
| `KYBER-AIGIS-ENC-DILITHIUM-SM4-GCM-SM3` | NTLSv1.1 | `certs/1` PQC dual certificate |
| `TLS13-SM2-KYBER768` | TLSv1.3 | `certs/loose` SM2 certificate |

Logs are written to `.tmp/pqc-suite-matrix/`.
