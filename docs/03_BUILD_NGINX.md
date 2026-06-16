# 编译 Nginx 1.30.2

本项目可以将上游 Nginx 1.30.2 链接到已经构建好的本地 Tongsuo 库，用于验证标准 TLS 路径下的 TLS 1.3 / PQC 配置行为。

Nginx 路径不启用 Angie NTLS 支持。不要在 Nginx 配置中使用 Angie 专有指令，例如：

```nginx
ssl_ntls;
sign:
enc:
```

## 源码位置

测试使用的源码树：

```text
third_party/nginx-1.30.2/
```

如果该目录缺失，可以下载并解包上游源码后放到同一位置。

## 编译命令

```bash
cd /root/tlcpMonoRepo-work
bash scripts/Nginx_TLS_PQC/build-nginx.sh
```

安装目录：

```text
/root/tlcpMonoRepo-work/nginx-install/
```

运行目录：

```text
/root/tlcpMonoRepo-work/nginx-work/
```

## 链接方式

脚本链接现有 Tongsuo 头文件和共享库：

```text
--with-cc-opt=-I/root/tlcpMonoRepo-work/tongsuo/include
--with-ld-opt='-L/root/tlcpMonoRepo-work/tongsuo -Wl,-rpath,/root/tlcpMonoRepo-work/tongsuo'
```

不要使用：

```text
--with-openssl=/root/tlcpMonoRepo-work/tongsuo
```

该选项会让 Nginx 执行 `make clean` 并重新配置 Tongsuo 源码树，可能破坏已经构建好的 TLCP/PQC 环境。

## 已知构建约束

当前测试构建关闭了 `http_gzip_module`。原因是 Nginx 1.30.2 的证书压缩集成期望的回调 ABI 与当前 Tongsuo 构建不一致。
