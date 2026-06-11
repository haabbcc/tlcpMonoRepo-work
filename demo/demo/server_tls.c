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
#define TLS_SERVER_CERT "../certs/tls_server.crt"
#define TLS_SERVER_KEY  "../certs/tls_server.key"
#define TLS_CA_CERT     "../certs/tls_ca.crt"
#define TLS_PORT        4443

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
    int listen_sock;
    int opt = 1;
    struct sockaddr_in addr;

    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock < 0) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(TLS_PORT);

    if (bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(listen_sock, 5) < 0) {
        perror("listen");
        exit(1);
    }

    return listen_sock;
}

int main(void)
{
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    const SSL_METHOD *method = NULL;
    int listen_sock = -1;
    int sock = -1;
    char buf[MAX_BUF_LEN];
    int n;

    setvbuf(stdout, NULL, _IONBF, 0);

    SSL_library_init();
    SSL_load_error_strings();

    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (SSL_CTX_use_certificate_file(ctx, TLS_SERVER_CERT, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, TLS_SERVER_KEY, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "TLS private key does not match certificate\n");
        return 1;
    }

    if (!SSL_CTX_load_verify_locations(ctx, TLS_CA_CERT, NULL)) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

    listen_sock = create_listen_socket();
    printf("TLS server listening on 0.0.0.0:%d\n", TLS_PORT);

    sock = accept(listen_sock, NULL, NULL);
    if (sock < 0) {
        perror("accept");
        return 1;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    SSL_set_fd(ssl, sock);
    SSL_set_accept_state(ssl);

    if (SSL_do_handshake(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    printf("TLS server handshake ok\n");
    printf("Protocol: %s\n", SSL_get_version(ssl));
    printf("Cipher: %s\n", SSL_get_cipher(ssl));
    show_peer_cert(ssl);

    memset(buf, 0, sizeof(buf));
    n = SSL_read(ssl, buf, sizeof(buf) - 1);
    if (n <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    printf("Received %d chars: '%s'\n", n, buf);

    if (SSL_write(ssl, "hello from tls server", strlen("hello from tls server")) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    close(listen_sock);
    SSL_CTX_free(ctx);

    return 0;
}
