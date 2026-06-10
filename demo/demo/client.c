#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <strings.h>
#include <errno.h>
//#include <netdb.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/ioctl.h>
//#include <sys/select.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include "openssl/bio.h"
#include "openssl/err.h"
#include "openssl/rand.h"
#include "openssl/ssl.h"
#include "openssl/x509v3.h"

#define MAX_BUF_LEN 4096
#define TLCP_CERT_DIR  "../../certs/loose"
#define CLIENT_S_CERT  TLCP_CERT_DIR "/sign_sm2.crt"
#define CLIENT_S_KEY   TLCP_CERT_DIR "/sign_sm2.key"
#define CLIENT_E_CERT  TLCP_CERT_DIR "/enc_sm2.crt"
#define CLIENT_E_KEY   TLCP_CERT_DIR "/enc_sm2.key"
#define CLIENT_CA_CERT TLCP_CERT_DIR "/ca_sm2.crt"
#define TLCP_CIPHER    "ECC-KYBER-SM4-GCM-SM3"


#define SSL_ERROR_WANT_HSM_RESULT 10

void Init_OpenSSL()
{
	if (!SSL_library_init())
		exit(0);
	SSL_load_error_strings();
}

void ShowCerts(SSL * ssl)
{
	X509 *cert;
	char *line;

	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL) {
		printf("Certificate information:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("Certificate: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Issuer: %s\n", line);
		free(line);
		X509_free(cert);
	}
	else
		printf("No certificate information.\n");
}

int main( )
{
	setvbuf(stdout, NULL, _IONBF, 0);
	BIO *conn = NULL;
	SSL *ssl = NULL;
	SSL_CTX *ctx = NULL;
	int usecert = 1;
	int retval;
	char sendbuf[MAX_BUF_LEN];
	int i = 0;
	const SSL_METHOD      *meth;
	int ret = 0;
	int nid = 0;
#if 0 
	/*Detect arguments*/
	if ((OBJ_txt2nid("1.2.156.10197.1.501") == NID_undef) &&
		(OBJ_create("1.2.156.10197.1.501", "SM2WITHSM3", "sm2withsm3") == 0)) {
		OBJ_cleanup();
		return 0;
	}

	if (OBJ_find_sigid_by_algs(NULL, NID_sm3, EVP_PKEY_EC) <= 0) {
		nid = OBJ_txt2nid("1.2.156.10197.1.501");
		if (NID_undef == nid)
			return 0;
		OBJ_add_sigid(nid, NID_sm3, EVP_PKEY_EC);
	}
#endif

	Init_OpenSSL();

	/* Use Tongsuo NTLS/TLCP. */
	meth = NTLS_client_method();
	if(meth == NULL	)
	{
		printf("meth is null\n");
		goto err;
	}
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL)
	{
		printf("Error of Create SSL CTX!\n");
		goto err;
	}

#if 1  
	//SSL_CTX_set_min_proto_version(ctx, 0);
	//SSL_CTX_set_max_proto_version(ctx, 0);

	SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_1);
	SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_2);
	SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_3);

#endif
	SSL_CTX_enable_ntls(ctx);

	if (usecert)
	{
		if (SSL_CTX_use_sign_certificate_file(ctx, CLIENT_S_CERT, SSL_FILETYPE_PEM) <= 0)
		{
			ERR_print_errors_fp(stderr);
			goto err;
		}
		if (SSL_CTX_use_sign_PrivateKey_file(ctx, CLIENT_S_KEY, SSL_FILETYPE_PEM) <= 0)
		{
			ERR_print_errors_fp(stderr);
			goto err;
		}
		if (!SSL_CTX_check_private_key(ctx))
		{
			fprintf(stderr, "Client signing private key does not match certificate\n");
			goto err;
		}

		if (SSL_CTX_use_enc_certificate_file(ctx, CLIENT_E_CERT, SSL_FILETYPE_PEM) <= 0)
		{
			ERR_print_errors_fp(stderr);
			goto err;
		}
		if (SSL_CTX_use_enc_PrivateKey_file(ctx, CLIENT_E_KEY, SSL_FILETYPE_PEM) <= 0)
		{
			ERR_print_errors_fp(stderr);
			goto err;
		}
		if (!SSL_CTX_check_private_key(ctx))
		{
			fprintf(stderr, "Client encryption private key does not match certificate\n");
			goto err;
		}
	}

	if (!SSL_CTX_load_verify_locations(ctx, CLIENT_CA_CERT, NULL))
	{
		ERR_print_errors_fp(stderr);
		goto err;
	}
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	if (!SSL_CTX_set_cipher_list(ctx, TLCP_CIPHER)) {
		ERR_print_errors_fp(stderr);
		printf("set cipher list %s fail!\n", TLCP_CIPHER);
		goto err;
	}
	/*Now Connect host:port*/
	//conn = BIO_new_connect("10.192.76.99:9498");
	conn = BIO_new_connect("127.0.0.1:4433");
	if (!conn)
	{
		printf("Error Of Create Connection BIO\n");
		goto err;
	}

	if (BIO_do_connect(conn) <= 0)
	{
		printf("Error Connect \n");
		goto err; 
	}
#if 0 
	if (!SSL_CTX_set_cipher_list(ctx, "ECC-SM4-SM3")) {
		ERR_print_errors_fp(stderr);
		printf("set cipher list fail!\n");
		goto err;
	}
#endif
	ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
		printf("SSL New Error\n");
		goto err;
	}

	SSL_set_bio(ssl, conn, conn);

	/*if (SSL_connect(ssl) <= 0)
	{
		printf("Error Of SSL connect server\n");
		goto err;
	}*/

	SSL_set_connect_state(ssl);
	while (1)
	{
		retval = SSL_do_handshake(ssl);
		if (retval > 0)
			break;
		else
		{
			if (SSL_get_error(ssl, retval) == SSL_ERROR_WANT_HSM_RESULT)
				continue;
			else
			{
				ERR_print_errors_fp(stderr);
				printf("Error Of SSL do handshake\n");
				goto err;
			}
		}
	}

printf("handshake  ok \n");
	printf("SSL connection using %s\n", SSL_get_cipher(ssl));
	ShowCerts(ssl);
	while (1) {
		if (SSL_write(ssl, "hello i am from client!", strlen("hello i am from client!")) <= 0)
		{
			printf("ssl_write fail!\n");
			break;
		}
		break;
	}
	char rbuf[2048];
	memset(rbuf, 0x0, sizeof(rbuf));
	if (SSL_read(ssl, rbuf, 2048) > 0)
		printf("SSL recv: %s.\n", rbuf);
	else
		printf("None recv buf.\n");

/*-----------------------------------------------------------------*/	
err:
//	SSL_shutdown(ssl);
//	if (ssl) SSL_free(ssl);
	if (ctx) SSL_CTX_free(ctx);

	return 0;
}


