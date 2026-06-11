好的，这是您提供的 Markdown 文档的汉化版本。

---

# title: NTLS 与 TLS 通信状态机
content:

本文档通过使用 Mermaid 流程图和状态机，展示了 NTLS 和 TLS 演示程序的服务器/客户端通信测试。



# title: NTLS 与 TLS 通信状态机  1. 测试入口流程
content:

```mermaid
flowchart TD
    A["开始测试"] --> B["构建演示程序二进制文件"]
    B --> C{"协议模式"}

    C -->|NTLS| N1["在 4433 端口启动 demo/demo/server"]
    N1 --> N2["加载 NTLS 签名证书/密钥"]
    N2 --> N3["加载 NTLS 加密证书/密钥"]
    N3 --> N4["启用 NTLS 并设置 ECC-KYBER-SM4-GCM-SM3"]
    N4 --> N5["运行 demo/demo/client"]
    N5 --> N6["客户端加载 NTLS 签名和加密证书"]
    N6 --> N7["NTLS 握手及证书验证"]
    N7 --> N8["应用数据交换"]
    N8 --> N9["关闭连接"]

    C -->|TLS| T1["运行 demo/demo/test_tls.sh"]
    T1 --> T2["如果不存在，则生成 TLS CA/服务器/客户端证书"]
    T2 --> T3["在 4443 端口启动 demo/demo/server_tls"]
    T3 --> T4["服务器加载一个 TLS 证书/密钥"]
    T4 --> T5["运行 demo/demo/client_tls"]
    T5 --> T6["客户端加载一个 TLS 证书/密钥"]
    T6 --> T7["TLS 握手及双向证书验证"]
    T7 --> T8["应用数据交换"]
    T8 --> T9["关闭连接"]
```



# title: NTLS 与 TLS 通信状态机  2. NTLS 双证书握手
content:

NTLS 演示程序使用铜锁 NTLS/TLCP 接口。服务器和客户端都加载两个证书/密钥对：一个用于签名，一个用于加密。

```mermaid
sequenceDiagram
    participant S as NTLS 服务器<br/>demo/demo/server.c
    participant C as NTLS 客户端<br/>demo/demo/client.c

    S->>S: SSL_library_init()
    S->>S: NTLS_server_method()
    S->>S: SSL_CTX_new()
    S->>S: SSL_CTX_enable_ntls()
    S->>S: SSL_CTX_use_sign_certificate_file()
    S->>S: SSL_CTX_use_sign_PrivateKey_file()
    S->>S: SSL_CTX_use_enc_certificate_file()
    S->>S: SSL_CTX_use_enc_PrivateKey_file()
    S->>S: SSL_CTX_set_cipher_list("ECC-KYBER-SM4-GCM-SM3")
    S->>S: SSL_CTX_load_verify_locations()
    S->>S: SSL_CTX_set_verify(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
    S->>S: 在 4433 端口进行 bind/listen/accept

    C->>C: SSL_library_init()
    C->>C: NTLS_client_method()
    C->>C: SSL_CTX_new()
    C->>C: SSL_CTX_enable_ntls()
    C->>C: SSL_CTX_use_sign_certificate_file()
    C->>C: SSL_CTX_use_sign_PrivateKey_file()
    C->>C: SSL_CTX_use_enc_certificate_file()
    C->>C: SSL_CTX_use_enc_PrivateKey_file()
    C->>C: SSL_CTX_load_verify_locations()
    C->>C: SSL_CTX_set_verify(SSL_VERIFY_PEER)
    C->>C: SSL_CTX_set_cipher_list("ECC-KYBER-SM4-GCM-SM3")

    C->>S: TCP 连接 127.0.0.1:4433
    C->>S: ClientHello，附带 NTLS 密码套件列表
    S->>C: ServerHello，选择 ECC-KYBER-SM4-GCM-SM3
    S->>C: 服务器签名证书
    S->>C: 服务器加密证书
    S->>C: 证书请求 (CertificateRequest)
    C->>S: 客户端签名证书
    C->>S: 客户端加密证书
    C->>S: 密钥交换 / 验证 / 完成 (Finished)
    S->>C: 验证 / 完成 (Finished)
    C->>S: 应用数据
    S->>C: 应用数据
    C->>S: close_notify / TCP 关闭
```



# title: NTLS 与 TLS 通信状态机  2. NTLS 双证书握手  NTLS 状态机
content:

```mermaid
stateDiagram-v2
    [*] --> 初始化
    初始化 --> 方法已创建: NTLS_server_method / NTLS_client_method
    方法已创建 --> CTX已创建: SSL_CTX_new
    CTX已创建 --> NTLS已启用: SSL_CTX_enable_ntls
    NTLS已启用 --> 签名证书已加载: 加载签名证书/密钥
    签名证书已加载 --> 加密证书已加载: 加载加密证书/密钥
    加密证书已加载 --> 密码套件已配置: SSL_CTX_set_cipher_list
    密码套件已配置 --> 验证已配置: 加载 CA 并设置验证模式
    验证已配置 --> TCP已连接: accept/connect
    TCP已连接 --> 握手中: SSL_accept / SSL_connect
    握手中 --> 已建立: 对端证书已验证且 Finished 完成
    已建立 --> 数据交换: SSL_read / SSL_write
    数据交换 --> 已关闭: SSL_shutdown / 关闭 socket
    已关闭 --> [*]

    握手中 --> 失败: 证书/密码套件/密钥/验证错误
    TCP已连接 --> 失败: socket 错误
    失败 --> [*]
```



# title: NTLS 与 TLS 通信状态机  3. 标准 TLS 双向认证握手
content:

TLS 演示程序使用 `server_tls.c` 和 `client_tls.c`。此路径不调用 `SSL_CTX_enable_ntls()`。它使用标准的 TLS 方法和单证书 API。

```mermaid
sequenceDiagram
    participant S as TLS 服务器<br/>demo/demo/server_tls.c
    participant C as TLS 客户端<br/>demo/demo/client_tls.c

    S->>S: SSL_library_init()
    S->>S: TLS_server_method()
    S->>S: SSL_CTX_new()
    S->>S: SSL_CTX_use_certificate_file(tls_server.crt)
    S->>S: SSL_CTX_use_PrivateKey_file(tls_server.key)
    S->>S: SSL_CTX_check_private_key()
    S->>S: SSL_CTX_load_verify_locations(tls_ca.crt)
    S->>S: SSL_CTX_set_verify(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
    S->>S: 在 4443 端口进行 bind/listen/accept

    C->>C: SSL_library_init()
    C->>C: TLS_client_method()
    C->>C: SSL_CTX_new()
    C->>C: SSL_CTX_use_certificate_file(tls_client.crt)
    C->>C: SSL_CTX_use_PrivateKey_file(tls_client.key)
    C->>C: SSL_CTX_check_private_key()
    C->>C: SSL_CTX_load_verify_locations(tls_ca.crt)
    C->>C: SSL_CTX_set_verify(SSL_VERIFY_PEER)

    C->>S: TCP 连接 127.0.0.1:4443
    C->>S: ClientHello
    S->>C: ServerHello
    S->>C: 服务器证书
    S->>C: 证书请求 (CertificateRequest)
    C->>S: 客户端证书
    C->>S: 证书验证 / 完成 (Finished)
    S->>C: 完成 (Finished)
    C->>S: "hello from tls client"
    S->>C: "hello from tls server"
    C->>S: close_notify / TCP 关闭
```



# title: NTLS 与 TLS 通信状态机  3. 标准 TLS 双向认证握手  TLS 状态机
content:

```mermaid
stateDiagram-v2
    [*] --> 初始化
    初始化 --> 方法已创建: TLS_server_method / TLS_client_method
    方法已创建 --> CTX已创建: SSL_CTX_new
    CTX已创建 --> 证书已加载: 加载一个证书/密钥
    证书已加载 --> 验证已配置: 加载 CA 并设置验证模式
    验证已配置 --> TCP已连接: accept/connect
    TCP已连接 --> 握手中: SSL_accept / SSL_connect
    握手中 --> 已建立: 已选择 TLS 版本/密码套件
    已建立 --> 数据交换: SSL_read / SSL_write
    数据交换 --> 已关闭: SSL_shutdown / 关闭 socket
    已关闭 --> [*]

    握手中 --> 失败: 证书/密钥/验证错误
    TCP已连接 --> 失败: socket 错误
    失败 --> [*]
```



# title: NTLS 与 TLS 通信状态机  4. 通信路径中的接口差异
content:

| 项目 | NTLS 演示程序 | TLS 演示程序 |
|---|---|---|
| 服务器源码 | `demo/demo/server.c` | `demo/demo/server_tls.c` |
| 客户端源码 | `demo/demo/client.c` | `demo/demo/client_tls.c` |
| 方法接口 | `NTLS_server_method()`, `NTLS_client_method()` | `TLS_server_method()`, `TLS_client_method()` |
| NTLS 开关 | `SSL_CTX_enable_ntls()` | 未使用 |
| 服务器证书模型 | 签名证书 + 加密证书 | 一个 TLS 证书 |
| 客户端证书模型 | 签名证书 + 加密证书 | 一个 TLS 证书 |
| 证书加载接口 | `SSL_CTX_use_sign_certificate_file()`, `SSL_CTX_use_sign_PrivateKey_file()`, `SSL_CTX_use_enc_certificate_file()`, `SSL_CTX_use_enc_PrivateKey_file()` | `SSL_CTX_use_certificate_file()`, `SSL_CTX_use_PrivateKey_file()` |
| 验证 | 通过 `SSL_CTX_load_verify_locations()` 加载 CA；启用对端证书验证 | 通过 `SSL_CTX_load_verify_locations()` 加载 CA；启用对端证书验证 |
| 密码套件配置 | `ECC-KYBER-SM4-GCM-SM3` | 默认 TLS 密码套件协商 |
| 测试端口 | `4433` | `4443` |
| 验证结果 | 已为 NTLS 双证书 PQC/GM 握手配置 | 已使用 TLSv1.3 和双向证书认证进行测试 |



# title: NTLS 与 TLS 通信状态机  5. 实际解读
content:

NTLS 的关键区别不仅在于它多使用了一个证书。其状态机有两条独立的证书/密钥加载路径：一条用于签名身份，另一条用于加密或密钥交换。

标准的 TLS 测试证实，同一个协议库也可以通过非 NTLS API 路径运行。它验证了单证书路径、标准 TLS 握手和双向证书认证，但并未验证国密双证书 NTLS 状态机。
