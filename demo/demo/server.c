#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "openssl/err.h"
#include "openssl/ssl.h"
#include "openssl/x509.h"

#define MAX_BUF_LEN 4096
#define TLCP_CERT_DIR        "../../certs/loose"
#define SM2_SERVER_SIGN_CERT TLCP_CERT_DIR "/sign_sm2.crt"
#define SM2_SERVER_SIGN_KEY  TLCP_CERT_DIR "/sign_sm2.key"
#define SM2_SERVER_ENC_CERT  TLCP_CERT_DIR "/enc_sm2.crt"
#define SM2_SERVER_ENC_KEY   TLCP_CERT_DIR "/enc_sm2.key"
#define SM2_SERVER_CA_CERT   TLCP_CERT_DIR "/ca_sm2.crt"
#define TLCP_CIPHER          "ECC-KYBER-SM4-GCM-SM3"
#define TLCP_PORT            4433

#define SSL_ERROR_WANT_HSM_RESULT 10

static void show_peer_cert(SSL *ssl)
{
    X509 *cert = SSL_get_peer_certificate(ssl);
    char *line = NULL;

    if (cert == NULL) {
        printf("No peer certificate.\n");
        return;
    }

    printf("Peer certificate information:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
    printf("Subject: %s\n", line);
    OPENSSL_free(line);

    line = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
    printf("Issuer: %s\n", line);
    OPENSSL_free(line);

    X509_free(cert);
}

static int create_listen_socket(void)
{
    int listen_sock = -1;
    int opt = 1;
    struct sockaddr_in addr;

    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listen_sock);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(TLCP_PORT);

    if (bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_sock);
        return -1;
    }

    if (listen(listen_sock, 5) < 0) {
        perror("listen");
        close(listen_sock);
        return -1;
    }

    return listen_sock;
}

static int do_ntls_handshake(SSL *ssl)
{
    int ret;

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
}

int main(void)
{
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    const SSL_METHOD *method = NULL;
    int listen_sock = -1;
    int sock = -1;
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    char buf[MAX_BUF_LEN];
    int n;
    int ret = 1;

    setvbuf(stdout, NULL, _IONBF, 0);

    SSL_library_init();
    SSL_load_error_strings();

    method = NTLS_server_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        goto err;
    }

    SSL_CTX_enable_ntls(ctx);

    if (SSL_CTX_use_sign_certificate_file(ctx, SM2_SERVER_SIGN_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    if (SSL_CTX_use_sign_PrivateKey_file(ctx, SM2_SERVER_SIGN_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Signing private key does not match the signing certificate public key\n");
        goto err;
    }
    printf("sign cert/key set ok\n");

    if (SSL_CTX_use_enc_certificate_file(ctx, SM2_SERVER_ENC_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    if (SSL_CTX_use_enc_PrivateKey_file(ctx, SM2_SERVER_ENC_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Encryption private key does not match the encryption certificate public key\n");
        goto err;
    }
    printf("enc cert/key set ok\n");

    if (!SSL_CTX_set_cipher_list(ctx, TLCP_CIPHER)) {
        ERR_print_errors_fp(stderr);
        fprintf(stderr, "set cipher list %s fail\n", TLCP_CIPHER);
        goto err;
    }
    printf("cipher set ok: %s\n", TLCP_CIPHER);

    if (!SSL_CTX_load_verify_locations(ctx, SM2_SERVER_CA_CERT, NULL)) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    SSL_CTX_set_verify_depth(ctx, 1);

    listen_sock = create_listen_socket();
    if (listen_sock < 0) {
        goto err;
    }
    printf("NTLS server listening on 0.0.0.0:%d\n", TLCP_PORT);

    memset(&peer_addr, 0, sizeof(peer_addr));
    sock = accept(listen_sock, (struct sockaddr *)&peer_addr, &peer_len);
    if (sock < 0) {
        perror("accept");
        goto err;
    }
    printf("NTLS client connected from %s:%d\n",
           inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        ERR_print_errors_fp(stderr);
        goto err;
    }

    if (!SSL_set_fd(ssl, sock)) {
        ERR_print_errors_fp(stderr);
        goto err;
    }

    if (!do_ntls_handshake(ssl)) {
        goto err;
    }

    printf("server handshake ok\n");
    printf("SSL connection using %s\n", SSL_get_cipher(ssl));
    show_peer_cert(ssl);

    memset(buf, 0, sizeof(buf));
    n = SSL_read(ssl, buf, sizeof(buf) - 1);
    if (n <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    buf[n] = '\0';
    printf("Received %d chars:'%s'\n", n, buf);

    if (SSL_write(ssl,
                  "-----This message is from the SSL server-----",
                  strlen("-----This message is from the SSL server-----")) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    printf("SSL write over\n");

    ret = 0;

err:
    if (ssl != NULL) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (sock >= 0) {
        close(sock);
    }
    if (listen_sock >= 0) {
        close(listen_sock);
    }
    if (ctx != NULL) {
        SSL_CTX_free(ctx);
    }

    return ret;
}
