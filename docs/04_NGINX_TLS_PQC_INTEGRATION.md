# Nginx TLS/PQC 联调说明

该集成路径使用上游 Nginx 1.30.2 作为标准 TLS 服务端，链接本地 Tongsuo 库，并使用现有 SM2 测试证书。

该路径不要求 Nginx 自身支持 NTLS。

## 文件说明

| 文件 | 作用 |
|---|---|
| `scripts/Nginx_TLS_PQC/build-nginx.sh` | 使用本地 Tongsuo 编译 Nginx 1.30.2 |
| `scripts/Nginx_TLS_PQC/render-conf.sh` | 渲染测试配置 |
| `scripts/Nginx_TLS_PQC/start-nginx.sh` | 启动测试 Nginx 实例 |
| `scripts/Nginx_TLS_PQC/stop-nginx.sh` | 停止测试 Nginx 实例 |
| `scripts/Nginx_TLS_PQC/test-client.sh` | 使用 Tongsuo `s_client` 连接测试 |
| `scripts/Nginx_TLS_PQC/test.sh` | 端到端 Nginx TLS 测试 |
| `config/nginx-tls-pqc.conf.in` | Nginx 配置模板 |

## 运行

```bash
cd /root/tlcpMonoRepo-work
bash scripts/Nginx_TLS_PQC/test.sh
```

期望输出：

```text
Nginx TLS test ok
```

## 测试配置

测试配置使用：

```nginx
ssl_protocols TLSv1.3;
ssl_conf_command Ciphersuites TLS_SM4_GCM_SM3;
ssl_conf_command Groups SM2:KYBER768;
```

因为这里没有使用 Angie NTLS 支持，服务端只呈现一个标准 TLS 证书，而不是 NTLS 双证书。

该路径主要用于验证：

- Nginx 是否能正确链接本地 Tongsuo
- `ssl_conf_command` 是否能把 `Ciphersuites` 和 `Groups` 传给 Tongsuo
- Tongsuo 是否接受指定 group 名称
- ClientHello / ServerHello 在线路层实际使用了哪些 Supported Group 码点
