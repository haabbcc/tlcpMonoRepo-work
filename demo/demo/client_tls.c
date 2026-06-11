#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openssl/bio.h"
#include "openssl/err.h"
#include "openssl/ssl.h"
#include "openssl/x509.h"

#define MAX_BUF_LEN 4096
#define TLS_CLIENT_CERT "../certs/tls_client.crt"
#define TLS_CLIENT_KEY  "../certs/tls_client.key"
#define TLS_CA_CERT     "../certs/tls_ca.crt"
#define TLS_HOST_PORT   "127.0.0.1:4443"

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

int main(void)
{
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    BIO *conn = NULL;
    const SSL_METHOD *method = NULL;
    char rbuf[MAX_BUF_LEN];
    int n;

    setvbuf(stdout, NULL, _IONBF, 0);

    SSL_library_init();
    SSL_load_error_strings();

    method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (SSL_CTX_use_certificate_file(ctx, TLS_CLIENT_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, TLS_CLIENT_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "TLS client private key does not match certificate\n");
        return 1;
    }

    if (!SSL_CTX_load_verify_locations(ctx, TLS_CA_CERT, NULL)) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

    conn = BIO_new_connect(TLS_HOST_PORT);
    if (conn == NULL) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (BIO_do_connect(conn) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    SSL_set_bio(ssl, conn, conn);
    SSL_set_connect_state(ssl);

    if (SSL_do_handshake(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    printf("TLS client handshake ok\n");
    printf("Protocol: %s\n", SSL_get_version(ssl));
    printf("Cipher: %s\n", SSL_get_cipher(ssl));
    show_peer_cert(ssl);

    if (SSL_write(ssl, "hello from tls client", strlen("hello from tls client")) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    memset(rbuf, 0, sizeof(rbuf));
    n = SSL_read(ssl, rbuf, sizeof(rbuf) - 1);
    if (n <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    printf("TLS recv: %s\n", rbuf);

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);

    return 0;
}
