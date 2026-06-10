# TLCP-PQC 与 demo 协议库接口差异对比

## 统计口径

- 项目1 TLCP-PQC：只统计 `tongsuo/apps/s_client.c` 与 `tongsuo/apps/s_server.c` 中直接出现的协议库接口调用。
- 项目2 demo：统计 `demo/demo/client.c`、`server.c`、`server_tls.c`、`server_P.c`、`server_PP.c` 中实际启用的调用；注释行和 `#if 0` 代码块不计入数量。
- 统计前缀：`SSL*`、`SSL_CTX*`、`SSL_SESSION*`、`BIO*`、`X509*`、`PEM*`、`ERR*`、`OBJ*`、`ENGINE*`、`CRYPTO*`、`OPENSSL*`、`EVP*` 等协议库/密码库接口。普通 socket、`printf`、`pthread` 不计入。
- 数量按唯一接口名统计，不按调用次数统计。

## 数量总览

| 项目 | 源码范围 | 唯一协议库接口数量 | 与对方重合 | 主要特征 |
|---|---|---:|---:|---|
| TLCP-PQC | `s_client.c` + `s_server.c` | 310 | 18 个 demo 实际接口同名出现 | 完整命令行工具，覆盖 TLS/DTLS/NTLS、双证书、证书链、CRL、OCSP、DANE、SNI、ALPN、PSK、会话、委托凭证、SM2+Kyber768 等。 |
| demo | `demo/demo/*.c` | 55 | 25 个同名出现在 TLCP-PQC 源码中 | `client.c` 与 `server.c` 已改为 Tongsuo NTLS/TLCP 双证书调用；其它 demo server 变体仍保留原有示例路径。 |
| demo 注释或 `#if 0` 中出现但未启用 | `demo/demo/*.c` | 未计入 55 | - | 包括部分旧证书加载、部分 cipher/protocol 设置。 |

demo 各文件实际启用接口数量：

| demo 源文件 | 唯一协议库接口数量 |
|---|---:|
| `client.c` | 31 |
| `server.c` | 36 |
| `server_tls.c` | 25 |
| `server_P.c` | 27 |
| `server_PP.c` | 28 |

## 证书相关核心差异

| 对比点 | TLCP-PQC `s_client/s_server` | demo | 参数差异 | 结论 |
|---|---|---|---|---|
| 普通证书加载 | `load_cert_pass()` / `load_cert()` 先读成 `X509 *`，再经 `set_cert_key_stuff()` 调 `SSL_CTX_use_certificate(ctx, cert)` | `client.c` / `server.c` 已改为双证书接口；其它 demo 变体仍有 `SSL_CTX_use_certificate_file(ctx, CERT, SSL_FILETYPE_PEM)` | TLCP-PQC 主工具使用对象参数；demo 当前主 client/server 使用文件路径形式的双证书接口 | demo 主路径已从单证书改为 Tongsuo 双证书文件接口。 |
| 普通私钥加载 | `load_key()` 先读成 `EVP_PKEY *`，再调 `SSL_CTX_use_PrivateKey(ctx, key)` | `client.c` / `server.c` 已改为 `SSL_CTX_use_sign_PrivateKey_file()` 和 `SSL_CTX_use_enc_PrivateKey_file()`；`server_P.c` 仍使用 `"engine:sdf_cipher:2:12345678"` + `SSL_FILETYPE_SDF` | TLCP-PQC 主工具使用 `EVP_PKEY *key`；demo 主路径使用文件路径 `const char *file` + `int type` | demo 主路径现在能覆盖 TLCP 双证书，但仍是文件接口，不是 TLCP-PQC app helper 的对象接口。 |
| 私钥匹配校验 | `set_cert_key_stuff()` 内部调用 `SSL_CTX_check_private_key(ctx)` | `client.c` / `server.c` 已启用 `SSL_CTX_check_private_key(ctx)`；其它变体不完全一致 | 参数都是 `const SSL_CTX *ctx` | demo 主路径现在会校验证书和私钥匹配。 |
| NTLS 签名证书 | 支持 `-sign_cert`、`-sign_key`，最终走 `SSL_CTX_use_sign_certificate(ctx, X509 *)` 和 `SSL_CTX_use_sign_PrivateKey(ctx, EVP_PKEY *)` | `client.c` / `server.c` 使用 `SSL_CTX_use_sign_certificate_file()` 和 `SSL_CTX_use_sign_PrivateKey_file()` | TLCP-PQC 对象接口；demo 文件接口 | demo 主路径已改为签名证书专用接口。 |
| NTLS 加密证书 | 支持 `-enc_cert`、`-enc_key`，最终走 `SSL_CTX_use_enc_certificate(ctx, X509 *)` 和 `SSL_CTX_use_enc_PrivateKey(ctx, EVP_PKEY *)` | `client.c` / `server.c` 使用 `SSL_CTX_use_enc_certificate_file()` 和 `SSL_CTX_use_enc_PrivateKey_file()` | TLCP-PQC 对象接口；demo 文件接口 | demo 主路径已实际启用国密双证书。 |
| 证书链 | `set_cert_key_stuff()` / `set_enc_cert_key_stuff()` 支持 `STACK_OF(X509) *chain` 与 `int build_chain`，内部可调 `SSL_CTX_set1_chain()`、`SSL_CTX_build_cert_chain()` | 无实际证书链接口 | TLCP-PQC 多 `chain` 与 `build_chain` 参数 | demo 只加载单张证书。 |
| CA/验证位置 | `ctx_set_verify_locations(ctx, CAfile, noCAfile, CApath, noCApath, CAstore, noCAstore)`；还支持 `ssl_load_stores()` 的 verify/chain store | `SSL_CTX_load_verify_locations(ctx, CA_CERT, NULL)` | TLCP-PQC 支持 CAfile/CApath/CAstore 及禁用默认 CA；demo 只有 `CAfile`，`CApath=NULL` | TLCP-PQC 验证源更完整。 |
| 验证策略 | `SSL_CTX_set_verify(ctx, verify, verify_callback)`；`verify` 来自命令行，可配深度、quiet、return_error、VPM | `SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback)`；`SSL_CTX_set_verify_depth(ctx, 1)` | TLCP-PQC mode 可配置；demo mode 固定，深度固定 1 | demo 的 `verify_callback()` 把失败强行改成成功，实际安全性较弱。 |
| 客户端证书请求 CA names | `SSL_add_file_cert_subjects_to_stack(nm, ReqCAfile)` + `SSL_CTX_set0_CA_list(ctx, nm)`；server 还可 `SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(CAfile))` | 无实际调用 | TLCP-PQC 传 `STACK_OF(X509_NAME) *` | demo 没有向对端声明可接受 CA 名称的逻辑。 |
| 对端证书读取 | `SSL_get0_peer_certificate(s)`、`SSL_get_peer_cert_chain(s)`、`PEM_write_bio_X509()`、`X509_print()` 等 | `SSL_get_peer_certificate(ssl)`、`X509_get_subject_name()`、`X509_get_issuer_name()`、`X509_NAME_oneline()` | TLCP-PQC 多用不增引用的 `get0` 与整链输出；demo 只拿单张证书并手动 `X509_free()` | TLCP-PQC 输出与诊断能力更完整。 |
| OCSP/CRL/DANE | 支持 OCSP status callback、CRL 加载、DANE TLSA | 无 | TLCP-PQC 多出相关 callback、store 和验证参数 | demo 不覆盖在线状态检查和 DANE 场景。 |
| TLCP/NTLS/PQC 开关 | `SSL_CTX_enable_ntls(ctx)`、`SSL_CTX_enable_force_ntls(ctx)`、`SSL_CTX_set_gm_tls13_dual_cert(ctx, 1)`、`SSL_CTX_enable_sm2_kyber768_tls13(ctx)`、`SSL_CTX_enable_sm_tls13_strict(ctx)` | `client.c` / `server.c` 已使用 `NTLS_client_method()` / `NTLS_server_method()` 和 `SSL_CTX_enable_ntls(ctx)`，cipher 为 `ECC-KYBER-SM4-GCM-SM3` | TLCP-PQC 覆盖更多开关；demo 主路径只启用 NTLS 和 cipher | demo 主路径已经转到 TLCP-PQC/Tongsuo NTLS 调用。 |

## demo 实际启用协议库接口明细

| 接口名 | 参数/原型要点 | demo 中用途 | TLCP-PQC 对应情况 |
|---|---|---|---|
| `BIO_new_connect` | `BIO *BIO_new_connect(const char *host_port)` | `client.c` 创建 `"ip:port"` 连接 BIO | `s_client` 走更复杂的 `init_client()`/socket/BIO 流程，未直接同名调用。 |
| `BIO_do_connect` | 宏，等价于 `BIO_do_handshake(BIO *b)` | `client.c` 发起 TCP BIO 连接 | TLCP-PQC 未直接同名调用。 |
| `SSL_library_init` | 宏，`OPENSSL_init_ssl(0, NULL)` | demo 初始化 SSL 库 | TLCP-PQC app 使用 OpenSSL 3/Tongsuo 初始化框架，不直接使用该旧式宏。 |
| `SSL_load_error_strings` | 宏，加载错误字符串 | demo 初始化错误字符串 | TLCP-PQC 不直接使用旧式宏。 |
| `SSL_CTX_new` | `SSL_CTX *SSL_CTX_new(const SSL_METHOD *meth)` | 创建上下文 | TLCP-PQC 使用 `SSL_CTX_new_ex(app_get0_libctx(), app_get0_propq(), meth)`，多 libctx/propq 参数。 |
| `SSL_CTX_free` | `void SSL_CTX_free(SSL_CTX *ctx)` | 释放 ctx | TLCP-PQC 同名使用。 |
| `TLS_client_method` | `const SSL_METHOD *TLS_client_method(void)` | `client.c` 创建客户端方法 | TLCP-PQC 同名使用，另支持 DTLS/TLS/NTLS 选项。 |
| `SSLv23_server_method` | `const SSL_METHOD *SSLv23_server_method(void)` | demo server 创建服务端方法 | TLCP-PQC 使用可配置的 server method，不以该旧式接口为主。 |
| `SSL_CTX_set_options` | `unsigned long SSL_CTX_set_options(SSL_CTX *ctx, unsigned long op)` | demo 禁用 TLS1.1/1.2/1.3 或设置选项 | TLCP-PQC 同名使用，但选项来自命令行。 |
| `SSL_CTX_set_cipher_list` | `int SSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *str)` | demo 可固定 `"ECC-SM4-SM3"`；部分文件启用 | TLCP-PQC 支持更完整 cipher 配置；同名未在 `s_client/s_server` 主体直接出现，通常经配置 helper。 |
| `SSL_new` | `SSL *SSL_new(SSL_CTX *ctx)` | 创建连接对象 | TLCP-PQC 同名使用。 |
| `SSL_free` | `void SSL_free(SSL *ssl)` | 释放连接对象 | TLCP-PQC 同名使用。 |
| `SSL_set_bio` | `void SSL_set_bio(SSL *s, BIO *rbio, BIO *wbio)` | `client.c` 绑定连接 BIO | TLCP-PQC 同名使用。 |
| `SSL_set_fd` | `int SSL_set_fd(SSL *s, int fd)` | demo server 绑定 socket fd | TLCP-PQC 主要使用 socket BIO，也有不同封装。 |
| `SSL_set_connect_state` | `void SSL_set_connect_state(SSL *s)` | client 进入 client handshake state | TLCP-PQC 同名使用。 |
| `SSL_set_accept_state` | `void SSL_set_accept_state(SSL *s)` | server 进入 accept state | TLCP-PQC 同名使用。 |
| `SSL_do_handshake` | `int SSL_do_handshake(SSL *s)` | 手动循环握手，处理 `SSL_ERROR_WANT_HSM_RESULT` | TLCP-PQC 同名使用，也处理 async/X509 lookup 等更多状态。 |
| `SSL_get_error` | `int SSL_get_error(const SSL *s, int ret_code)` | 判断握手错误 | TLCP-PQC 同名使用。 |
| `SSL_read` | `int SSL_read(SSL *ssl, void *buf, int num)` | 读取应用数据 | TLCP-PQC 同名使用。 |
| `SSL_write` | `int SSL_write(SSL *ssl, const void *buf, int num)` | 写应用数据 | TLCP-PQC 同名使用。 |
| `SSL_shutdown` | `int SSL_shutdown(SSL *s)` | 关闭 TLS 连接 | TLCP-PQC 通过 helper 和部分路径调用。 |
| `SSL_get_cipher` | 宏，按当前 cipher 返回名称 | 打印连接 cipher | TLCP-PQC 更多使用 `SSL_get_current_cipher()` + `SSL_CIPHER_get_name()` 等。 |
| `SSL_CTX_use_certificate_file` | `int SSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type)` | demo 从文件加载服务端证书 | TLCP-PQC 主路径先加载为 `X509 *`，再用 `SSL_CTX_use_certificate()`；参数差异最大。 |
| `SSL_CTX_use_PrivateKey_file` | `int SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type)` | demo 从 PEM 或 SDF engine URI 加载私钥 | TLCP-PQC 主路径先加载为 `EVP_PKEY *`，再用 `SSL_CTX_use_PrivateKey()`。 |
| `SSL_CTX_load_verify_locations` | `int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile, const char *CApath)` | server 加载 CA 文件 | TLCP-PQC 通过 `ctx_set_verify_locations()` 和 `ssl_load_stores()` 支持 CAfile/CApath/CAstore。 |
| `SSL_CTX_set_verify` | `void SSL_CTX_set_verify(SSL_CTX *ctx, int mode, SSL_verify_cb callback)` | server 设置对端证书验证模式 | TLCP-PQC 同名使用，mode 来自命令行。 |
| `SSL_CTX_set_verify_depth` | `void SSL_CTX_set_verify_depth(SSL_CTX *ctx, int depth)` | demo 固定深度 1 | TLCP-PQC 深度通过 verify 参数体系配置，不直接同名出现在主路径。 |
| `SSL_get_peer_certificate` | `X509 *SSL_get_peer_certificate(const SSL *s)` | demo 获取单张对端证书，调用者释放 | TLCP-PQC 倾向 `SSL_get0_peer_certificate()` 和 `SSL_get_peer_cert_chain()`，输出更完整。 |
| `X509_get_subject_name` | `X509_NAME *X509_get_subject_name(const X509 *a)` | 取 subject | TLCP-PQC 同名使用。 |
| `X509_get_issuer_name` | `X509_NAME *X509_get_issuer_name(const X509 *a)` | 取 issuer | TLCP-PQC 同名使用。 |
| `X509_NAME_oneline` | `char *X509_NAME_oneline(const X509_NAME *a, char *buf, int size)` | demo 打印 subject/issuer | TLCP-PQC 多用 `X509_NAME_print_ex()`，格式控制更好。 |
| `X509_free` | `void X509_free(X509 *a)` | 释放 `SSL_get_peer_certificate()` 返回值 | TLCP-PQC 同名使用。 |
| `ERR_print_errors_fp` | `void ERR_print_errors_fp(FILE *fp)` | demo 打印错误 | TLCP-PQC 多用 `ERR_print_errors(BIO *)`，适配 app BIO 输出。 |
| `ERR_clear_error` | `void ERR_clear_error(void)` | engine fallback 前清理错误 | TLCP-PQC 同名使用。 |
| `OBJ_txt2nid` | `int OBJ_txt2nid(const char *s)` | 注册/查找 SM2WITHSM3 OID | TLCP-PQC 不需要在 `s_client/s_server` 主路径重复注册该 OID。 |
| `OBJ_create` | `int OBJ_create(const char *oid, const char *sn, const char *ln)` | demo 创建 `1.2.156.10197.1.501` | TLCP-PQC 未直接同名使用。 |
| `OBJ_find_sigid_by_algs` | `int OBJ_find_sigid_by_algs(int *psignid, int dig_nid, int pkey_nid)` | 查找 SM3 + SM2 签名算法映射 | TLCP-PQC 未直接同名使用。 |
| `OBJ_add_sigid` | `int OBJ_add_sigid(int signid, int dig_id, int pkey_id)` | 添加 SM2WITHSM3 签名算法映射 | TLCP-PQC 未直接同名使用。 |
| `OBJ_cleanup` | `void OBJ_cleanup(void)` | OID 创建失败清理 | TLCP-PQC 未直接同名使用。 |
| `ENGINE_by_id` | `ENGINE *ENGINE_by_id(const char *id)` | `server_P.c` 获取 `sdf_cipher` engine | TLCP-PQC engine 由 app helper/命令行参数管理。 |
| `ENGINE_load_builtin_engines` | `void ENGINE_load_builtin_engines(void)` | engine fallback | TLCP-PQC 不在 `s_client/s_server` 主路径直接同名。 |
| `ENGINE_set_default` | `int ENGINE_set_default(ENGINE *e, unsigned int flags)` | 设置 SDF engine 默认实现 | TLCP-PQC 通过 `setup_engine()`/`load_key()` 等 helper 处理。 |
| `ENGINE_free` | `int ENGINE_free(ENGINE *e)` | engine 失败路径释放 | TLCP-PQC helper 内部管理。 |
| `ENGINE_load_gm` | `int ENGINE_load_gm(const GM_CALLBACK_METHOD *cb, void *args)` | `server_PP.c` 加载 gm_cipher engine | TLCP-PQC 未使用 demo 的 gm_cipher 专用接口。 |
| `CRYPTO_num_locks` | `int CRYPTO_num_locks(void)` | `server_PP.c` OpenSSL 1.0.x 线程锁初始化 | TLCP-PQC 未直接使用旧式锁 API。 |
| `CRYPTO_set_id_callback` | `void CRYPTO_set_id_callback(unsigned long (*func)(void))` | 设置线程 ID 回调 | TLCP-PQC 未直接使用旧式锁 API。 |
| `CRYPTO_set_locking_callback` | `void CRYPTO_set_locking_callback(void (*func)(int, int, const char *, int))` | 设置锁回调 | TLCP-PQC 未直接使用旧式锁 API。 |
| `OPENSSL_malloc` | `void *OPENSSL_malloc(size_t num)` | 分配 lock 数组 | TLCP-PQC 同名使用，但不用于 demo 这类线程锁场景。 |

## TLCP-PQC 证书/国密/PQC 侧额外接口

| 接口或 helper | 参数/原型要点 | 出现位置 | demo 是否启用 | 差异说明 |
|---|---|---|---|---|
| `set_cert_key_stuff` | `SSL_CTX *ctx, X509 *cert, EVP_PKEY *key, STACK_OF(X509) *chain, int build_chain` | `s_client.c`、`s_server.c`，实现在 `apps/lib/s_cb.c` | 无 | TLCP-PQC app 层统一设置证书、私钥、链和 build-chain。 |
| `SSL_CTX_use_certificate` | `SSL_CTX *ctx, X509 *x` | `set_cert_key_stuff()` 内部 | demo 用文件接口 | 参数从文件路径变成已解析的 `X509 *`。 |
| `SSL_CTX_use_PrivateKey` | `SSL_CTX *ctx, EVP_PKEY *pkey` | `set_cert_key_stuff()` 内部 | demo 用文件接口 | 参数从文件路径变成已解析的 `EVP_PKEY *`。 |
| `SSL_CTX_check_private_key` | `const SSL_CTX *ctx` | `set_cert_key_stuff()` 内部 | demo 中相关代码未启用 | TLCP-PQC 主路径实际校验。 |
| `SSL_CTX_set1_chain` | `SSL_CTX *ctx, STACK_OF(X509) *chain` | helper 内部 | 无 | 支持显式证书链。 |
| `SSL_CTX_build_cert_chain` | `SSL_CTX *ctx, int flags` | helper 内部 | 无 | 支持自动构建证书链。 |
| `set_sign_cert_key_stuff` | `SSL_CTX *ctx, X509 *cert, EVP_PKEY *key, STACK_OF(X509) *chain, int build_chain` | NTLS 路径 | 无 | 国密签名证书专用 helper。 |
| `SSL_CTX_use_sign_certificate` | `SSL_CTX *ctx, X509 *x` | `set_sign_cert_key_stuff()` 内部 | 无 | Tongsuo/NTLS 专用签名证书对象接口。 |
| `SSL_CTX_use_sign_PrivateKey` | `SSL_CTX *ctx, EVP_PKEY *pkey` | `set_sign_cert_key_stuff()` 内部 | 无 | Tongsuo/NTLS 专用签名私钥对象接口。 |
| `set_enc_cert_key_stuff` | `SSL_CTX *ctx, X509 *cert, EVP_PKEY *key, STACK_OF(X509) *chain, int build_chain` | NTLS 路径 | demo 有未启用文件接口 | 国密加密证书专用 helper。 |
| `SSL_CTX_use_enc_certificate` | `SSL_CTX *ctx, X509 *x` | `set_enc_cert_key_stuff()` 内部 | 无实际启用 | Tongsuo/NTLS 专用加密证书对象接口。 |
| `SSL_CTX_use_enc_PrivateKey` | `SSL_CTX *ctx, EVP_PKEY *pkey` | `set_enc_cert_key_stuff()` 内部 | demo 未启用的是 `SSL_CTX_use_enc_PrivateKey_file()` | TLCP-PQC 使用对象接口，demo 备用代码使用文件接口。 |
| `SSL_CTX_enable_ntls` | `SSL_CTX *ctx` | `s_client.c`、`s_server.c` | 无 | 显式开启 NTLS/TLCP 能力。 |
| `SSL_CTX_enable_force_ntls` | `SSL_CTX *ctx` | `s_server.c` | 无 | server 可强制 NTLS。 |
| `SSL_CTX_set_gm_tls13_dual_cert` | `SSL_CTX *ctx, int enable` | `s_client.c`、`s_server.c` | 无 | TLS 1.3 国密双证书开关。 |
| `SSL_CTX_enable_sm_tls13_strict` | `SSL_CTX *ctx` | `s_client.c`、`s_server.c` | 无 | SM TLS 1.3 strict 模式。 |
| `SSL_CTX_enable_sm2_kyber768_tls13` | `SSL_CTX *ctx` | `s_client.c`、`s_server.c` | 无 | SM2 + Kyber768 混合密钥交换开关，是 TLCP-PQC 的核心差异之一。 |
| `SSL_CTX_enable_verify_peer_by_dc` | `SSL_CTX *ctx` | `s_client.c`、`s_server.c` | 无 | 委托凭证验证扩展。 |
| `SSL_CTX_use_dc` / `SSL_CTX_use_dc_PrivateKey` | `SSL_CTX *ctx, DELEGATED_CREDENTIAL *dc` / `SSL_CTX *ctx, EVP_PKEY *pkey` | `set_dc_cert_key_stuff()` 内部 | 无 | 委托凭证签名路径，demo 没有。 |

## 结论

1. demo 的 `client.c` 和 `server.c` 已从“最小单证书流程”改为 Tongsuo NTLS/TLCP 双证书流程；其它 demo 变体仍保留原有示例实现。
2. TLCP-PQC 的 `s_client/s_server` 是“完整测试/诊断工具”：证书输入、证书链、验证源、验证参数、NTLS 双证书、PQC 开关、SNI/ALPN/OCSP/DANE/PSK/会话等接口覆盖面远大于 demo。
3. 证书方面最关键的接口差异是参数形态：demo 多用 `*_file(ctx, const char *file, int type)`；TLCP-PQC 主路径多用 `X509 *`、`EVP_PKEY *`、`STACK_OF(X509) *` 对象参数，并在 helper 中统一做私钥匹配和链处理。
4. demo 当前主路径已经实际使用国密双证书文件接口；不过它仍不同于 TLCP-PQC app helper 的对象接口，后者会先解析为 `X509 *` / `EVP_PKEY *` 再设置到 `SSL_CTX`。
