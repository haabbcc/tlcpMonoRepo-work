/*
 * ++
 * FACILITY:
 *
 *      Simplest SM2 TLSv1.1 Server
 *
 * ABSTRACT:
 *
 *   This is an example of a SSL server with minimum functionality.
 *    The socket APIs are used to handle TCP/IP operations. This SSL
 *    server loads its own certificate and key, but it does not verify
 *  the certificate of the SSL client.
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <winsock.h>
//#include <WinSock2.h>
//#pragma comment(lib,"Ws2_32.lib ")
//#include <ws2tcpip.h>
#include "openssl/crypto.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/engine.h"


#define MAX_BUF_LEN 4096
#define TLCP_CERT_DIR        "../../certs/loose"
#define SM2_SERVER_SIGN_CERT TLCP_CERT_DIR "/sign_sm2.crt"
#define SM2_SERVER_SIGN_KEY  TLCP_CERT_DIR "/sign_sm2.key"
#define SM2_SERVER_ENC_CERT  TLCP_CERT_DIR "/enc_sm2.crt"
#define SM2_SERVER_ENC_KEY   TLCP_CERT_DIR "/enc_sm2.key"
#define SM2_SERVER_CA_CERT   TLCP_CERT_DIR "/ca_sm2.crt"
#define TLCP_CIPHER          "ECC-KYBER-SM4-GCM-SM3"


//#define SDF_SERVER_CERT "../certs/107_2s.crt"
#define SDF_SERVER_CERT "../certs/signature.cer"


#define SM2_SERVER_CA_PATH  "."
#define SSL_ERROR_WANT_HSM_RESULT 10
#define ON   1
#define OFF  0

#define RETURN_NULL(x) if ((x)==NULL) exit(1)
#define RETURN_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define RETURN_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }
int opt = 1000;

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


int verify_callback(int ok, X509_STORE_CTX *ctx)
{
	if (!ok) {
		ok = 1;
	}

	return (ok);
}


int main( )
{
	int     err;
	int     verify_client = ON; /* To verify a client certificate, set ON */

	int     listen_sock;
	int     sock;
	struct sockaddr_in sa_serv;
	struct sockaddr_in sa_cli;
	size_t client_len;
	char    *str;
	char    buf[MAX_BUF_LEN];

	SSL_CTX         *ctx = NULL;
	SSL             *ssl = NULL;
	const SSL_METHOD      *meth;

	short int       s_port = 4433;

	int ret = 0;
	int hsm_tag = 1;
	int aio_tag = 1;
	int error;
	int nid = 0;
	ENGINE *e;

#if 1 
	if ((OBJ_txt2nid("1.2.156.10197.1.501") == NID_undef) &&
		(OBJ_create("1.2.156.10197.1.501", "SM2WITHSM3", "sm2withsm3") == 0)) {
		OBJ_cleanup();
		return 0;
	}

	if (OBJ_find_sigid_by_algs(NULL, NID_sm3, EVP_PKEY_SM2) <= 0) {
		nid = OBJ_txt2nid("1.2.156.10197.1.501");
		if (NID_undef == nid)
			return 0;
		OBJ_add_sigid(nid, NID_sm3, EVP_PKEY_SM2);
	}

#endif
	/* Load encryption & hashing algorithms for the SSL program */
        SSL_library_init();

	/* Load the error strings for SSL & CRYPTO APIs */
        SSL_load_error_strings();


	//ENGINE_load_sdf();
#if 0 
	e = ENGINE_by_id("sdf_cipher");
        if (e == NULL)
        {
	     ERR_clear_error();
	     ENGINE_load_builtin_engines();
	     e = ENGINE_by_id("sdf_cipher");
        }

	if(e == NULL ){
		ERR_print_errors_fp(stderr);
	}

        if (ENGINE_set_default(e, ENGINE_METHOD_ALL) == 0) {
            ENGINE_free(e);
	    return 0;
        }

#endif
	/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
	meth = NTLS_server_method();

	/* Create a SSL_CTX structure */
	ctx = SSL_CTX_new(meth);
#if 0
	SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_1);
	SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_2);
	SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_3);

#endif
	if (!ctx)
	{
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	SSL_CTX_enable_ntls(ctx);

	/* Load the TLCP signing certificate and key. */
	if (SSL_CTX_use_sign_certificate_file(ctx, SM2_SERVER_SIGN_CERT, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_CTX_use_sign_PrivateKey_file(ctx, SM2_SERVER_SIGN_KEY, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (!SSL_CTX_check_private_key(ctx))
	{
		fprintf(stderr, "Signing private key does not match the signing certificate public key\n");
		exit(1);
	}
	printf("sign cert/key set ok\n");

	/* Load the TLCP encryption certificate and key. */
	if (SSL_CTX_use_enc_certificate_file(ctx, SM2_SERVER_ENC_CERT, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (SSL_CTX_use_enc_PrivateKey_file(ctx, SM2_SERVER_ENC_KEY, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	if (!SSL_CTX_check_private_key(ctx))
	{
		fprintf(stderr, "Encryption private key does not match the encryption certificate public key\n");
		exit(1);
	}
	printf("enc cert/key set ok\n");

	if (!SSL_CTX_set_cipher_list(ctx, TLCP_CIPHER)) {
		ERR_print_errors_fp(stderr);
		printf("set cipher list %s fail!\n", TLCP_CIPHER);
		exit(1);
	}
	printf("cipher set ok: %s\n", TLCP_CIPHER);
	if (verify_client == ON)
	{
		/* Load the RSA CA certificate into the SSL_CTX structure */
		if (!SSL_CTX_load_verify_locations(ctx, SM2_SERVER_CA_CERT, NULL))
		{
			ERR_print_errors_fp(stderr);
			exit(1);
		}

		/* Set to require peer (client) certificate verification */
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);
		//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
		//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);

		/* Set the verification depth to 1 */
		SSL_CTX_set_verify_depth(ctx, 1);

	}

	/* ----------------------------------------------- */
	/* Set up a TCP socket IPPROTO_TCP*/
	listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	ret = setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
	if (ret == -1)
	{
		printf("set socket erro\n");
		exit(1);

	}
	
	//RETURN_ERR(listen_sock, "socket");
	memset(&sa_serv, '\0', sizeof(sa_serv));
	sa_serv.sin_family = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port = htons(s_port);          /* Server Port number */
	ret = bind(listen_sock, (struct sockaddr*)&sa_serv, sizeof(sa_serv));
	if (ret == -1)
	{
		printf("bind err\n");
		exit(1);

	}

	/* Wait for an incoming TCP connection. */
	err = listen(listen_sock, 5);

	RETURN_ERR(err, "listen");
	client_len = sizeof(sa_cli);

	/* Socket for a TCP/IP connection is created */
	sock = accept(listen_sock, (struct sockaddr *)&sa_cli, (socklen_t *)&client_len);

	RETURN_ERR(sock, "accept");
	//close(listen_sock);

	//printf("Connection from %d, port %d\n",sa_cli.sin_addr.s_addr,sa_cli.sin_port);

	/* ----------------------------------------------- */
	/* TCP connection is ready. */
	/* A SSL structure is created */

#if 0
	if (!SSL_CTX_set_cipher_list(ctx, "ECC-SM4-SM3")) {
                ERR_print_errors_fp(stderr);
                printf("set cipher list fail!\n");
                goto err;
        }
#endif
	ssl = SSL_new(ctx);

	RETURN_NULL(ssl);

	/* Assign the socket into the SSL structure (SSL and socket without BIO) */
	SSL_set_fd(ssl, sock);

	/* Perform SSL Handshake on the SSL server */
	/*err = SSL_accept(ssl);*/
	SSL_set_accept_state(ssl);
	while (1)
	{
		err = SSL_do_handshake(ssl);
		if (err <= 0)
		{
			if (SSL_get_error(ssl, err) == SSL_ERROR_WANT_HSM_RESULT)
				continue;
			else
			{
				ERR_print_errors_fp(stderr);
				continue;
			//	goto err;
			}
		}
		else
			break;
	}
printf("server handshake ok\n");
	RETURN_SSL(err);

	/* Informational output (optional) */
	printf("SSL connection using %s\n", SSL_get_cipher(ssl));
	ShowCerts(ssl);


	/*------- DATA EXCHANGE - Receive message and send reply. -------*/
	/* Receive data from the SSL client */
	while (1) {
		memset(buf, 0x00, sizeof(buf));
		err = SSL_read(ssl, buf, sizeof(buf) - 1);
		if (err <= 0) {
			printf("ssl_read fail!\n");
			break;
		}
		break;
	}

	RETURN_SSL(err);

	buf[err] = '\0';

	printf("Received %d chars:'%s'\n", err, buf);
	/* Send data to the SSL client */
	err = SSL_write(ssl,
		"-----This message is from the SSL server-----",
		strlen("-----This message is from the SSL server-----"));

printf("SSL write over\n");
	RETURN_SSL(err);

	/*--------------- SSL closure ---------------*/
	/* Shutdown this side (server) of the connection. */

	err = SSL_shutdown(ssl);

	RETURN_SSL(err);

	/* Terminate communication on a socket */
	//close(sock);
	//close(listen_sock);

err:

	/* Free the SSL structure */
	if (ssl) SSL_free(ssl);

	/* Free the SSL_CTX structure */
	if (ctx) SSL_CTX_free(ctx);


	return 0;


}





