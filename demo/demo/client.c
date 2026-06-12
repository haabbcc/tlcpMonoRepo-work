#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openssl/bio.h"
#include "openssl/err.h"
#include "openssl/ssl.h"
#include "openssl/x509.h"

#define MAX_BUF_LEN 4096
#define TLCP_CERT_DIR  "../../certs/loose"
#define CLIENT_S_CERT  TLCP_CERT_DIR "/sign_sm2.crt"
#define CLIENT_S_KEY   TLCP_CERT_DIR "/sign_sm2.key"
#define CLIENT_E_CERT  TLCP_CERT_DIR "/enc_sm2.crt"
#define CLIENT_E_KEY   TLCP_CERT_DIR "/enc_sm2.key"
#define CLIENT_CA_CERT TLCP_CERT_DIR "/ca_sm2.crt"
#define TLCP_CIPHER    "ECC-KYBER-SM4-GCM-SM3"
#define TLCP_HOST_PORT "127.0.0.1:4433"

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

static int do_ntls_handshake(SSL *ssl)
{
    int ret;

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
}

int main(void)
{
    BIO *conn = NULL;
    SSL *ssl = NULL;
    SSL_CTX *ctx = NULL;
    const SSL_METHOD *method = NULL;
    char rbuf[MAX_BUF_LEN];
    int n;
    int ret = 1;

    setvbuf(stdout, NULL, _IONBF, 0);

    SSL_library_init();
    SSL_load_error_strings();

    method = NTLS_client_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        goto err;
    }

    SSL_CTX_enable_ntls(ctx);

    if (SSL_CTX_use_sign_certificate_file(ctx, CLIENT_S_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    if (SSL_CTX_use_sign_PrivateKey_file(ctx, CLIENT_S_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Client signing private key does not match certificate\n");
        goto err;
    }

    if (SSL_CTX_use_enc_certificate_file(ctx, CLIENT_E_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    if (SSL_CTX_use_enc_PrivateKey_file(ctx, CLIENT_E_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Client encryption private key does not match certificate\n");
        goto err;
    }

    if (!SSL_CTX_load_verify_locations(ctx, CLIENT_CA_CERT, NULL)) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

    if (!SSL_CTX_set_cipher_list(ctx, TLCP_CIPHER)) {
        ERR_print_errors_fp(stderr);
        fprintf(stderr, "set cipher list %s fail\n", TLCP_CIPHER);
        goto err;
    }

    conn = BIO_new_connect(TLCP_HOST_PORT);
    if (conn == NULL) {
        ERR_print_errors_fp(stderr);
        goto err;
    }

    if (BIO_do_connect(conn) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        ERR_print_errors_fp(stderr);
        goto err;
    }

    SSL_set_bio(ssl, conn, conn);
    conn = NULL;

    if (!do_ntls_handshake(ssl)) {
        goto err;
    }

    printf("handshake ok\n");
    printf("SSL connection using %s\n", SSL_get_cipher(ssl));
    show_peer_cert(ssl);

    if (SSL_write(ssl, "hello i am from client!", strlen("hello i am from client!")) <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }

    memset(rbuf, 0, sizeof(rbuf));
    n = SSL_read(ssl, rbuf, sizeof(rbuf) - 1);
    if (n <= 0) {
        ERR_print_errors_fp(stderr);
        goto err;
    }
    rbuf[n] = '\0';
    printf("SSL recv: %s.\n", rbuf);

    ret = 0;

err:
    if (ssl != NULL) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (conn != NULL) {
        BIO_free_all(conn);
    }
    if (ctx != NULL) {
        SSL_CTX_free(ctx);
    }

    return ret;
}
