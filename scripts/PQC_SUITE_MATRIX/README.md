# PQC 套件矩阵测试

运行当前 Tongsuo 暴露的所有 PQC 相关握手套件：

```bash
cd /root/tlcpMonoRepo-work
bash scripts/PQC_SUITE_MATRIX/test-all.sh
```

覆盖的套件：

| 套件 | 协议 | 证书模型 |
|---|---|---|
| `ECC-KYBER-SM4-GCM-SM3` | NTLSv1.1 | `certs/loose` SM2 双证书 |
| `ECDHE-KYBER-SM4-GCM-SM3` | NTLSv1.1 | `certs/loose` SM2 双证书 |
| `KYBER-DILITHIUM-SM4-GCM-SM3` | NTLSv1.1 | `certs/1` PQC 双证书 |
| `KYBER-AIGIS-ENC-DILITHIUM-SM4-GCM-SM3` | NTLSv1.1 | `certs/1` PQC 双证书 |
| `TLS13-SM2-KYBER768` | TLSv1.3 | `certs/loose` SM2 证书 |

日志写入：

```text
.tmp/pqc-suite-matrix/
```
