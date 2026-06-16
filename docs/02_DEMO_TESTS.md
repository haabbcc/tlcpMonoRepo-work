# demo 测试说明

本文档说明 `demo/demo` 目录下的独立 server/client 示例如何编译、运行，以及这些 demo 实际测试了哪些 Tongsuo/OpenSSL 接口路径。

服务端/客户端握手均通过 `SSL_do_handshake()` 驱动；其中 NTLS 服务端显式调用 `SSL_set_accept_state()`，NTLS 客户端和 TLS 客户端显式调用 `SSL_set_connect_state()`。

接口测试汇总

server.c/client.c测试
| 抽象层       | 接口                                                                                                                                                | 测试目的                                      |
| --------- | --------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------- |
| 协议方法选择接口  | `NTLS_server_method()` / `NTLS_client_method()`                                                                                                     | 确认程序走 NTLS/TLCP 协议栈，而不是普通 TLS             |
| NTLS 开关接口 | `SSL_CTX_enable_ntls()`                                                                                                                             | 确认 SSL_CTX 被切换到 NTLS/TLCP 模式              |
| 双证书加载接口   | `SSL_CTX_use_sign_certificate_file()`、`SSL_CTX_use_sign_PrivateKey_file()`、`SSL_CTX_use_enc_certificate_file()`、`SSL_CTX_use_enc_PrivateKey_file()` | 验证 TLCP 双证书模型：签名证书和加密证书分别加载               |
| 私钥匹配检查接口  | `SSL_CTX_check_private_key()`                                                                                                                       | 验证证书与私钥是否匹配                               |
| 密码套件选择接口  | `SSL_CTX_set_cipher_list("ECC-KYBER-SM4-GCM-SM3")`                                                                                                  | 强制验证 ECC/Kyber + SM4-GCM + SM3 这条套件路径是否可用 |
| 证书验证接口    | `SSL_CTX_load_verify_locations()`、`SSL_CTX_set_verify()`                                                                                            | 验证 CA 信任链和双向认证路径                          |
| 连接绑定接口    | server: `SSL_set_fd()`；client: `SSL_set_bio()`                                                                                                      | 把 TCP 连接绑定到 SSL 对象                        |
| 握手接口      | `SSL_set_accept_state()` / `SSL_set_connect_state()` + `SSL_do_handshake()`                                                                         | 触发 NTLS/TLCP 握手状态机                        |
| 数据收发接口    | `SSL_read()` / `SSL_write()`                                                                                                                        | 验证握手后 record 层可以正常传输应用数据                  |
| 结果检查接口    | `SSL_get_cipher()`、`SSL_get_peer_certificate()`                                                                                                     | 查看最终协商套件和对端证书                             |

server_tls.c/client_tls.c
| 抽象层          | 接口                                                                    | 测试作用                                      |
| ------------ | ----------------------------------------------------------------------- | ----------------------------------------- |
| TLS 协议方法接口   | `TLS_server_method()` / `TLS_client_method()`                           | 创建普通 TLS server/client 协议方法               |
| SSL 上下文接口    | `SSL_CTX_new()`                                                         | 创建 TLS 上下文                                |
| 单证书加载接口      | `SSL_CTX_use_certificate_file()`                                        | 加载普通 TLS 证书                               |
| 单私钥加载接口      | `SSL_CTX_use_PrivateKey_file()`                                         | 加载普通 TLS 私钥                               |
| 证书私钥匹配检查     | `SSL_CTX_check_private_key()`                                           | 验证证书和私钥是否匹配                               |
| CA 验证接口      | `SSL_CTX_load_verify_locations()`                                       | 加载 CA，用于验证对端证书                            |
| 双向认证接口       | `SSL_CTX_set_verify()`                                                  | server 要求 client 提供证书，client 验证 server 证书 |
| TCP/TLS 绑定接口 | server: `SSL_set_fd()`；client: `SSL_set_bio()`                          | 把底层 TCP 连接绑定到 SSL 对象                      |
| TLS 握手接口     | `SSL_do_handshake()`                                                    | 驱动 TLS 握手                                 |
| 协商结果检查       | `SSL_get_version()` / `SSL_get_cipher()` / `SSL_get_peer_certificate()` | 打印协议版本、套件、对端证书                            |
| 应用数据接口       | `SSL_read()` / `SSL_write()`                                            | 验证 TLS record 层收发正常                       |


## 1. 文件结构

| 文件                       | 协议        | 角色  | 作用                                 |
| ------------------------ | --------- | --- | ---------------------------------- |
| `demo/demo/server.c`     | NTLS/TLCP | 服务端 | 国密双证书 + ECC/Kyber 混合套件 server demo |
| `demo/demo/client.c`     | NTLS/TLCP | 客户端 | 国密双证书 + ECC/Kyber 混合套件 client demo |
| `demo/demo/server_tls.c` | TLS       | 服务端 | 普通 TLS server demo                 |
| `demo/demo/client_tls.c` | TLS       | 客户端 | 普通 TLS client demo                 |
| `demo/demo/mk.sh`        | 编译脚本      | 脚本  | 编译 NTLS demo 和 TLS demo            |
| `demo/demo/test_tls.sh`  | TLS 测试脚本  | 脚本  | 自动运行 TLS server/client 接口测试        |

## 2. 编译 demo

先编译核心项目，并加载运行环境：

```bash
cd /root/tlcpMonoRepo-work
./build-all.sh
source ./env.sh
```

再编译 demo：

```bash
cd /root/tlcpMonoRepo-work/demo/demo
sh mk.sh
```

`mk.sh` 会生成以下可执行文件：

```text
server
client
server_tls
client_tls
```

`mk.sh` 编译时默认使用仓库内的 Tongsuo 头文件和库文件：

```text
TONGSUO_INC_DIR=${REPO_ROOT}/tongsuo/include
TONGSUO_LIB_DIR=${REPO_ROOT}/tongsuo
```

如果外部显式设置了 `TONGSUO_INC_DIR` 或 `TONGSUO_LIB_DIR`，则优先使用外部指定路径。

`env.sh` 会设置 Tongsuo 和 provider 相关运行环境变量，主要包括：

```text
LD_LIBRARY_PATH
OPENSSL_MODULES
OPENSSL_CONF
```

其中 `OPENSSL_CONF` 指向仓库内的 provider 配置文件，用于让 Tongsuo 在运行时加载相应 provider。

如果 `demo/certs/` 下没有 TLS 测试证书，`mk.sh` 会自动生成普通 TLS 测试所需的 CA、服务端证书和客户端证书。证书生成使用的 `openssl` 选择顺序如下：

```text
1. 显式设置的 CERT_OPENSSL_BIN
2. env.sh 中的 OPENSSL
3. 仓库内的 tongsuo/apps/openssl
4. 系统 openssl
```

在新机器上建议执行：

```bash
source /root/tlcpMonoRepo-work/env.sh
cd /root/tlcpMonoRepo-work/demo/demo
sh mk.sh
```

证书生成使用的 `openssl` 选择顺序如下：

1. 显式设置的 `CERT_OPENSSL_BIN`
2. `env.sh` 中的 `${OPENSSL}`
3. 仓库内的 `tongsuo/apps/openssl`
4. 系统 `openssl`

在新机器上建议先执行：

```bash
source /root/tlcpMonoRepo-work/env.sh
cd /root/tlcpMonoRepo-work/demo/demo
./mk.sh
```

如果系统没有可用的 `openssl.cnf`，`mk.sh` 已经给 `openssl req` 显式使用 `/dev/null` 配置，避免 TLS 测试证书生成阶段阻断 demo 编译。

## 3. NTLS demo

### 3.1 测试目标

NTLS demo 由 `server.c` 和 `client.c` 组成，用于验证 Tongsuo 的 NTLS/TLCP 双证书接口路径是否可端到端跑通。

该测试覆盖以下能力：

```text
1. 创建 NTLS server/client SSL_CTX
2. 显式启用 NTLS/TLCP 模式
3. 分别加载签名证书、签名私钥、加密证书、加密私钥
4. 检查证书和私钥匹配关系
5. 设置 ECC-KYBER-SM4-GCM-SM3 混合密码套件
6. 加载 CA 并启用对端证书验证
7. 建立 TCP 连接
8. 通过 SSL_do_handshake() 驱动 NTLS/TLCP 握手
9. 握手完成后通过 SSL_read()/SSL_write() 验证 record 层应用数据收发
```

该测试不是性能测试，也不是 HTTP 业务测试。它只验证一个最小 server/client 通信链路：握手成功后，客户端发送一条应用数据，服务端读取后返回一条应用数据。

### 3.2 证书与密码套件

NTLS demo 使用仓库根目录下的 `certs/loose` 证书：

```text
../../certs/loose/sign_sm2.crt
../../certs/loose/sign_sm2.key
../../certs/loose/enc_sm2.crt
../../certs/loose/enc_sm2.key
../../certs/loose/ca_sm2.crt
```

注意：这里的相对路径是以 `demo/demo` 作为运行目录计算的。

NTLS demo 固定设置密码套件：

```text
ECC-KYBER-SM4-GCM-SM3
```

默认服务端监听端口：

```text
4433
```

默认客户端连接地址：

```text
127.0.0.1:4433
```

### 3.3 服务端接口路径

`server.c` 的接口路径如下：

```text
SSL_library_init()
SSL_load_error_strings()

NTLS_server_method()
SSL_CTX_new()
SSL_CTX_enable_ntls()

SSL_CTX_use_sign_certificate_file()
SSL_CTX_use_sign_PrivateKey_file()
SSL_CTX_check_private_key()

SSL_CTX_use_enc_certificate_file()
SSL_CTX_use_enc_PrivateKey_file()
SSL_CTX_check_private_key()

SSL_CTX_set_cipher_list("ECC-KYBER-SM4-GCM-SM3")

SSL_CTX_load_verify_locations()
SSL_CTX_set_verify(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
SSL_CTX_set_verify_depth()

socket()
setsockopt()
bind()
listen()
accept()

SSL_new()
SSL_set_fd()

SSL_set_accept_state()
SSL_do_handshake()

SSL_get_cipher()
SSL_get_peer_certificate()

SSL_read()
SSL_write()

SSL_shutdown()
SSL_free()
SSL_CTX_free()
```

其中，NTLS 服务端握手不是通过 `SSL_accept()` 触发，而是由以下逻辑显式驱动：

```c
SSL_set_accept_state(ssl);

for (;;) {
    ret = SSL_do_handshake(ssl);
    if (ret > 0) {
        return 1;
    }

    if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_HSM_RESULT) {
        continue;
    }

    ERR_print_errors_fp(stderr);
    return 0;
}
```

这里的 `SSL_ERROR_WANT_HSM_RESULT` 是 demo 中定义的特殊错误码：

```c
#define SSL_ERROR_WANT_HSM_RESULT 10
```

因此该 demo 保留了对 HSM/异步密码设备返回状态的处理痕迹：当握手过程中出现该状态时，demo 会继续调用 `SSL_do_handshake()`。

### 3.4 客户端接口路径

`client.c` 的接口路径如下：

```text
SSL_library_init()
SSL_load_error_strings()

NTLS_client_method()
SSL_CTX_new()
SSL_CTX_enable_ntls()

SSL_CTX_use_sign_certificate_file()
SSL_CTX_use_sign_PrivateKey_file()
SSL_CTX_check_private_key()

SSL_CTX_use_enc_certificate_file()
SSL_CTX_use_enc_PrivateKey_file()
SSL_CTX_check_private_key()

SSL_CTX_load_verify_locations()
SSL_CTX_set_verify(SSL_VERIFY_PEER)

SSL_CTX_set_cipher_list("ECC-KYBER-SM4-GCM-SM3")

BIO_new_connect("127.0.0.1:4433")
BIO_do_connect()

SSL_new()
SSL_set_bio()

SSL_set_connect_state()
SSL_do_handshake()

SSL_get_cipher()
SSL_get_peer_certificate()

SSL_write()
SSL_read()

SSL_shutdown()
SSL_free()
SSL_CTX_free()
```

其中，NTLS 客户端握手不是通过 `SSL_connect()` 触发，而是由以下逻辑显式驱动：

```c
SSL_set_connect_state(ssl);

for (;;) {
    ret = SSL_do_handshake(ssl);
    if (ret > 0) {
        return 1;
    }

    if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_HSM_RESULT) {
        continue;
    }

    ERR_print_errors_fp(stderr);
    return 0;
}
```

### 3.5 手动运行 NTLS demo

终端 1 启动服务端：

```bash
cd /root/tlcpMonoRepo-work/demo/demo
./server
```

终端 2 启动客户端：

```bash
cd /root/tlcpMonoRepo-work/demo/demo
./client
```

服务端期望看到类似输出：

```text
sign cert/key set ok
enc cert/key set ok
cipher set ok: ECC-KYBER-SM4-GCM-SM3
NTLS server listening on 0.0.0.0:4433
NTLS client connected from 127.0.0.1:xxxxx
server handshake ok
SSL connection using ECC-KYBER-SM4-GCM-SM3
Peer certificate information:
Received ... chars:'hello i am from client!'
SSL write over
```

客户端期望看到类似输出：

```text
handshake ok
SSL connection using ECC-KYBER-SM4-GCM-SM3
Peer certificate information:
SSL recv: -----This message is from the SSL server-----.
```

## 4. TLS demo

### 4.1 测试目标

TLS demo 由 `server_tls.c` 和 `client_tls.c` 组成，用于验证普通 TLS API 路径。它不启用 NTLS，不使用 TLCP 双证书接口，也不设置 `ECC-KYBER-SM4-GCM-SM3`。

该测试覆盖以下能力：

```text
1. 创建普通 TLS server/client SSL_CTX
2. 加载普通 TLS 单证书和私钥
3. 检查证书和私钥匹配关系
4. 加载 CA 并启用对端证书验证
5. 建立 TCP 连接
6. 通过 SSL_do_handshake() 驱动 TLS 握手
7. 打印协商出的 TLS 版本和密码套件
8. 握手完成后通过 SSL_read()/SSL_write() 验证 record 层应用数据收发
```

TLS demo 的作用是作为 NTLS demo 的对照组。若 TLS demo 失败，说明基础 Tongsuo/OpenSSL、证书、库路径或 socket 环境可能存在问题；若 TLS demo 成功而 NTLS demo 失败，则问题更可能集中在 NTLS 开关、双证书接口、混合密码套件或 provider 配置路径。

### 4.2 证书与端口

TLS demo 使用 `demo/certs` 下的普通 TLS 测试证书：

```text
../certs/tls_ca.crt
../certs/tls_server.crt
../certs/tls_server.key
../certs/tls_client.crt
../certs/tls_client.key
```

注意：这里的相对路径是以 `demo/demo` 作为运行目录计算的。

默认服务端监听端口：

```text
4443
```

默认客户端连接地址：

```text
127.0.0.1:4443
```

### 4.3 服务端接口路径

`server_tls.c` 的接口路径如下：

```text
SSL_library_init()
SSL_load_error_strings()

TLS_server_method()
SSL_CTX_new()

SSL_CTX_use_certificate_file()
SSL_CTX_use_PrivateKey_file()
SSL_CTX_check_private_key()

SSL_CTX_load_verify_locations()
SSL_CTX_set_verify(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT)

socket()
setsockopt()
bind()
listen()
accept()

SSL_new()
SSL_set_fd()

SSL_do_handshake()

SSL_get_version()
SSL_get_cipher()
SSL_get_peer_certificate()

SSL_read()
SSL_write()

SSL_shutdown()
SSL_free()
SSL_CTX_free()
```

`server_tls.c` 没有调用 `SSL_accept()`。它在 `SSL_set_fd()` 后直接调用：

```c
SSL_do_handshake(ssl);
```

服务端通过 `SSL_CTX_set_verify()` 设置：

```text
SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT
```

因此 TLS demo 的服务端要求客户端必须提供证书，属于普通 TLS 双向认证测试。

### 4.4 客户端接口路径

`client_tls.c` 的接口路径如下：

```text
SSL_library_init()
SSL_load_error_strings()

TLS_client_method()
SSL_CTX_new()

SSL_CTX_use_certificate_file()
SSL_CTX_use_PrivateKey_file()
SSL_CTX_check_private_key()

SSL_CTX_load_verify_locations()
SSL_CTX_set_verify(SSL_VERIFY_PEER)

BIO_new_connect("127.0.0.1:4443")
BIO_do_connect()

SSL_new()
SSL_set_bio()

SSL_set_connect_state()
SSL_do_handshake()

SSL_get_version()
SSL_get_cipher()
SSL_get_peer_certificate()

SSL_write()
SSL_read()

SSL_shutdown()
SSL_free()
SSL_CTX_free()
```

`client_tls.c` 没有调用 `SSL_connect()`。它在 `SSL_set_bio()` 后显式调用：

```c
SSL_set_connect_state(ssl);
SSL_do_handshake(ssl);
```

### 4.5 自动运行 TLS 接口测试

执行：

```bash
cd /root/tlcpMonoRepo-work/demo/demo
./test_tls.sh
```

`test_tls.sh` 会执行以下动作：

```text
1. 调用 mk.sh 编译 demo
2. 后台启动 server_tls
3. 运行 client_tls
4. 打印 tls_server.log 和 tls_client.log
5. 检查服务端是否输出 TLS server handshake ok
6. 检查客户端是否输出 TLS client handshake ok
7. 检查客户端是否收到 hello from tls server
```

期望最终输出：

```text
TLS interface test ok
```

注意：`server_tls.c` 和 `client_tls.c` 会打印实际协商出的 TLS 协议版本和密码套件，例如：

```text
Protocol: ...
Cipher: ...
```

但 `test_tls.sh` 当前只检查握手成功和应用数据收发成功，不固定校验某一个具体 TLS 版本或某一个具体密码套件。

## 5. NTLS demo 与 TLS demo 差异

| 项目        | NTLS demo                                                                    | TLS demo                                     |
| --------- | ---------------------------------------------------------------------------- | -------------------------------------------- |
| 源码文件      | `server.c`, `client.c`                                                       | `server_tls.c`, `client_tls.c`               |
| 协议方法      | `NTLS_server_method()`, `NTLS_client_method()`                               | `TLS_server_method()`, `TLS_client_method()` |
| 是否启用 NTLS | 调用 `SSL_CTX_enable_ntls()`                                                   | 不调用                                          |
| 证书模型      | 签名证书 + 加密证书                                                                  | 单证书                                          |
| 证书接口      | `SSL_CTX_use_sign_certificate_file()` / `SSL_CTX_use_enc_certificate_file()` | `SSL_CTX_use_certificate_file()`             |
| 私钥接口      | `SSL_CTX_use_sign_PrivateKey_file()` / `SSL_CTX_use_enc_PrivateKey_file()`   | `SSL_CTX_use_PrivateKey_file()`              |
| 密码套件设置    | 固定设置 `ECC-KYBER-SM4-GCM-SM3`                                                 |  TLS默认套件TLS_AES_256_GCM_SHA384                |
| 服务端认证策略   | 要求客户端证书                                                                      | 要求客户端证书                                      |
| 客户端认证策略   | 验证服务端证书                                                                      | 验证服务端证书                                      |
| 握手驱动      | `SSL_do_handshake()`                                                         | `SSL_do_handshake()`                         |
| 默认端口      | `4433`                                                                       | `4443`                                       |
| 测试目的      | 验证 NTLS/TLCP 双证书 + 混合套件接口路径                                                  | 验证普通 TLS 单证书双向认证接口路径                         |

## 6. 接口测试边界

这组 demo 主要验证接口连通性，不覆盖以下内容：

```text
1. 不测试性能、吞吐量、延迟或并发连接能力
2. 不测试 HTTP/HTTPS 业务协议
3. 不测试 Angie 的 ssl_ntls 或 proxy_ssl_ntls 配置路径
4. 不测试多客户端连接
5. 不测试长连接、多轮 request/response 或异常断链恢复
6. 不测试证书生产级部署模型
```

其中 `server.c` 和 `server_tls.c` 都只 `accept()` 一个客户端连接，完成一次握手和一轮应用数据收发后退出。

## 7. 历史脚本说明

`demo/demo` 下还保留了几个早期硬件/SDF 或本地库布局相关脚本：

```text
conf.sh
km.sh
load.sh
```

这些脚本不属于当前 `tlcpMonoRepo` + Tongsuo demo 主流程。当前 demo 的推荐入口是：

```text
mk.sh
server
client
server_tls
client_tls
test_tls.sh
```

在新机器或新的实验环境中，应优先使用 `mk.sh` 编译，并使用 `server/client` 或 `test_tls.sh` 运行测试。

## 8. 推荐排查顺序

如果 demo 运行失败，建议按以下顺序排查：

```text
1. 确认已在仓库根目录执行 source ./env.sh
2. 确认 mk.sh 编译成功，并生成 server/client/server_tls/client_tls
3. 先运行 ./test_tls.sh，确认普通 TLS 路径可用
4. 再运行 ./server 和 ./client，确认 NTLS/TLCP 路径可用
5. 若 TLS 成功但 NTLS 失败，重点检查 provider、NTLS 开关、双证书和 ECC-KYBER-SM4-GCM-SM3 套件
6. 若 TLS 也失败，重点检查 Tongsuo 库路径、证书路径、CA、端口占用和运行目录
```

从定位问题的角度看，TLS demo 是基础路径对照组，NTLS demo 是双证书与混合套件路径验证组。两者结合可以区分基础 TLS 环境问题和 NTLS/TLCP 特有接口问题。
