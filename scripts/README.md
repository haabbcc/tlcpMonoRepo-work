# scripts 目录说明

该目录保存项目联调和实验脚本。

## 子目录

- `Kyber_Dilithium_SM4_GCM_SM3/`
  - 使用 `certs/1`
  - 目标套件：`KYBER-DILITHIUM-SM4-GCM-SM3`

- `ECC_Kyber_SM4_GCM_SM3/`
  - 使用 `certs/loose`
  - 目标套件：`ECC-KYBER-SM4-GCM-SM3`

- `ECDHE_Kyber_SM4_GCM_SM3/`
  - 使用 `certs/loose`
  - 目标套件：`ECDHE-KYBER-SM4-GCM-SM3`

- `Angie_TLCP_PQC/`
  - Angie 编译、配置、启动、停止和客户端验证脚本

## 使用前提

先在仓库根目录完成：

```bash
./build-all.sh
source ./env.sh
```

部分脚本内部会自动 `source ../../env.sh`，但建议先在当前 shell 中手动加载环境，避免 provider 路径不一致。

## 参考文档

- [构建指南](C:/Users/14050/Desktop/dpdk/tlcpMonoRepo/tlcpMonoRepo-main/docs/TLCP-PQC-BUILD-GUIDE.md)
- [Angie 实验说明](C:/Users/14050/Desktop/dpdk/tlcpMonoRepo/tlcpMonoRepo-main/docs/ANGIE_TLCP_PQC_EXPERIMENT.md)
