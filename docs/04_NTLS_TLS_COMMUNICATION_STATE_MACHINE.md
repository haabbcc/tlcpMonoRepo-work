# NTLS 与 TLS 通信测试流程和状态机

本文档用 Mermaid 流程图和状态机说明当前 `demo/demo` 中 NTLS 与 TLS 两组 server/client 的通信测试过程。

## 1. 测试入口流程

```mermaid
flowchart TD
    A["开始测试"] --> B["编译 demo 程序"]
    B --> C{"协议模式"}

    C -->|NTLS| N1["启动 demo/demo/server，监听 4433"]
    N1 --> N2["创建 NTLS SSL_CTX"]
    N2 --> N3["启用 NTLS"]
    N3 --> N4["加载签名证书和私钥"]
    N4 --> N5["加载加密证书和私钥"]
    N5 --> N6["设置 ECC-KYBER-SM4-GCM-SM3"]
    N6 --> N7["加载 CA 并要求验证客户端证书"]

    C -->|TLS| T1["运行 demo/demo/test_tls.sh"]
    T1 --> T2["如不存在则生成 TLS CA/server/client 证书"]
    T2 --> T3["启动 demo/demo/server_tls，监听 4443"]
    T3 --> T4["服务端加载单张 TLS 证书和私钥"]
    T4 --> T5["运行 demo/demo/client_tls"]
    T5 --> T6["客户端加载单张 TLS 证书和私钥"]
    T6 --> T7["TLS 握手和双向证书验证"]
    T7 --> T8["应用数据收发"]
    T8 --> T9["关闭连接"]
```

## 2. NTLS 双证书握手时序

NTLS demo 使用 Tongsuo 的 NTLS/TLCP 接口。server 和 client 都加载两组证书和私钥：一组用于签名身份认证，一组用于加密或密钥交换路径。

```mermaid
sequenceDiagram
    participant S as NTLS Server<br/>demo/demo/server.c
    participant C as NTLS Client<br/>demo/demo/client.c

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
    S->>S: bind/listen/accept on 4433

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

    C->>S: TCP connect 127.0.0.1:4433
    C->>S: ClientHello，携带 NTLS cipher list
    S->>C: ServerHello，选择 ECC-KYBER-SM4-GCM-SM3
    S->>C: 服务端签名证书
    S->>C: 服务端加密证书
    S->>C: CertificateRequest
    C->>S: 客户端签名证书
    C->>S: 客户端加密证书
    C->>S: Key exchange / verify / Finished
    S->>C: Verify / Finished
    C->>S: 应用数据
    S->>C: 应用数据
    C->>S: close_notify / TCP close
```

## 3. NTLS 状态机

```mermaid
stateDiagram-v2
    [*] --> Init
    Init --> MethodCreated: NTLS_server_method / NTLS_client_method
    MethodCreated --> CtxCreated: SSL_CTX_new
    CtxCreated --> NtlsEnabled: SSL_CTX_enable_ntls
    NtlsEnabled --> SignCertLoaded: 加载签名证书和私钥
    SignCertLoaded --> EncCertLoaded: 加载加密证书和私钥
    EncCertLoaded --> CipherConfigured: SSL_CTX_set_cipher_list
    CipherConfigured --> VerifyConfigured: 加载 CA 并设置验证模式
    VerifyConfigured --> TcpConnected: accept/connect
    TcpConnected --> Handshaking: SSL_accept / SSL_connect
    Handshaking --> Established: 对端证书验证完成且 Finished 完成
    Established --> DataExchange: SSL_read / SSL_write
    DataExchange --> Closed: SSL_shutdown / close socket
    Closed --> [*]

    Handshaking --> Failed: 证书/套件/私钥/验证错误
    TcpConnected --> Failed: socket 错误
    Failed --> [*]
```

## 4. 普通 TLS 双向认证握手时序

TLS demo 使用 `server_tls.c` 和 `client_tls.c`。这一路径不调用 `SSL_CTX_enable_ntls()`，只使用普通 TLS 方法和单证书接口。

```mermaid
sequenceDiagram
    participant S as TLS Server<br/>demo/demo/server_tls.c
    participant C as TLS Client<br/>demo/demo/client_tls.c

    S->>S: SSL_library_init()
    S->>S: TLS_server_method()
    S->>S: SSL_CTX_new()
    S->>S: SSL_CTX_use_certificate_file(tls_server.crt)
    S->>S: SSL_CTX_use_PrivateKey_file(tls_server.key)
    S->>S: SSL_CTX_check_private_key()
    S->>S: SSL_CTX_load_verify_locations(tls_ca.crt)
    S->>S: SSL_CTX_set_verify(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
    S->>S: bind/listen/accept on 4443

    C->>C: SSL_library_init()
    C->>C: TLS_client_method()
    C->>C: SSL_CTX_new()
    C->>C: SSL_CTX_use_certificate_file(tls_client.crt)
    C->>C: SSL_CTX_use_PrivateKey_file(tls_client.key)
    C->>C: SSL_CTX_check_private_key()
    C->>C: SSL_CTX_load_verify_locations(tls_ca.crt)
    C->>C: SSL_CTX_set_verify(SSL_VERIFY_PEER)

    C->>S: TCP connect 127.0.0.1:4443
    C->>S: ClientHello
    S->>C: ServerHello
    S->>C: 服务端证书
    S->>C: CertificateRequest
    C->>S: 客户端证书
    C->>S: CertificateVerify / Finished
    S->>C: Finished
    C->>S: "hello from tls client"
    S->>C: "hello from tls server"
    C->>S: close_notify / TCP close
```

## 5. TLS 状态机

```mermaid
stateDiagram-v2
    [*] --> Init
    Init --> MethodCreated: TLS_server_method / TLS_client_method
    MethodCreated --> CtxCreated: SSL_CTX_new
    CtxCreated --> CertLoaded: 加载单张证书和私钥
    CertLoaded --> VerifyConfigured: 加载 CA 并设置验证模式
    VerifyConfigured --> TcpConnected: accept/connect
    TcpConnected --> Handshaking: SSL_accept / SSL_connect
    Handshaking --> Established: 协议版本和密码套件协商完成
    Established --> DataExchange: SSL_read / SSL_write
    DataExchange --> Closed: SSL_shutdown / close socket
    Closed --> [*]

    Handshaking --> Failed: 证书/私钥/验证错误
    TcpConnected --> Failed: socket 错误
    Failed --> [*]
```
