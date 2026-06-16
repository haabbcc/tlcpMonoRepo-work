# PQC TLS cross-validation

Run:

```bash
cd /root/tlcpMonoRepo-work
bash scripts/PQC_CROSS_VALIDATE/run-cross-validation.sh
```

Logs and the Markdown summary are written under:

```text
logs/cross-pqc/
```

This workflow tests the requested public ecosystem group names:

```text
X25519MLKEM768 / 0x11EC
curveSM2MLKEM768 / 0x11EE
```

It also records the current local implementation fallback:

```text
SM2:KYBER768 + TLS_SM4_GCM_SM3
```

Do not treat `SM2:KYBER768` as proof of `curveSM2MLKEM768 / 0x11EE`
interoperability unless packet capture or server/client output shows the 0x11EE
group.
