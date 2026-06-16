# Nginx TLS/PQC 联调脚本

本目录用于编译并运行上游 Nginx 1.30.2，使其链接本地已经构建好的 Tongsuo 库。

该路径刻意不启用 Angie 专有 NTLS 指令，例如 `ssl_ntls` 或 `sign:/enc:` 双证书语法。这里验证的是标准 TLS 服务端路径，以及 `ssl_conf_command` 传递给 Tongsuo 后的 TLS 1.3 / PQC 相关行为。

## 运行流程

```bash
cd /root/tlcpMonoRepo-work
bash scripts/Nginx_TLS_PQC/build-nginx.sh
bash scripts/Nginx_TLS_PQC/render-conf.sh
bash scripts/Nginx_TLS_PQC/start-nginx.sh
bash scripts/Nginx_TLS_PQC/test-client.sh
bash scripts/Nginx_TLS_PQC/stop-nginx.sh
```

## 构建方式

构建脚本使用外部 Tongsuo 头文件和共享库：

```text
--with-cc-opt=-I/root/tlcpMonoRepo-work/tongsuo/include
--with-ld-opt='-L/root/tlcpMonoRepo-work/tongsuo -Wl,-rpath,/root/tlcpMonoRepo-work/tongsuo'
```

不要使用 Nginx 的 `--with-openssl=...` 指向本仓库里的 Tongsuo 源码。该选项会让 Nginx 重新配置并清理 Tongsuo 源码树，可能破坏当前 TLCP/PQC 实验环境。
