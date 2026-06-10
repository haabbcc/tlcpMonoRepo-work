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
#include <strings.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
//#include <WinSock2.h>
//#pragma comment(lib,"Ws2_32.lib ")
//#include <ws2tcpip.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "openssl/crypto.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/evp.h"
#include<sys/time.h>
#include "gm_cipher.h"

#define MAX_BUF_LEN 4096
#define SM2_SERVER_CERT     "../certs/SS.crt"
#define SM2_SERVER_KEY      "../certs/SS.key"


#define SM2_SERVER_CA_CERT  "../certs/CA.pem"

#define SM2_SERVER_CA_PATH  "."
#define SSL_ERROR_WANT_HSM_RESULT 10
#define ON   1
#define OFF  0

#define NUM 100 

#define RETURN_NULL(x) if ((x)==NULL) exit(1)
#define RETURN_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define RETURN_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }
int opt = 1000;

static pthread_mutex_t *lock_cs;
static long *lock_count;

pthread_t pthreads_thread_id(void)
{

       pthread_t ret;

       ret=pthread_self();

       return(ret);
}


void *thread_main(void *arg)
{
	int  err;
	char buf[1024 * 1024];
	int  ret = 0;

	struct timeval ts,te;
        double time_h;

	SSL     *ssl=(SSL *)arg;

	SSL_set_accept_state(ssl);
gettimeofday(&ts,NULL);
        while (1)
        {
                err = SSL_do_handshake(ssl);
                if (err <= 0)
                {
                        if (SSL_get_error(ssl, err) == SSL_ERROR_WANT_HSM_RESULT)
                                continue;
                        else
                        {
				printf("Error Of SSL do handshake\n");
                                ERR_print_errors_fp(stderr);
				return;
                        }
                }
                else
                        break;
        }

gettimeofday(&te,NULL);
time_h = te.tv_sec-ts.tv_sec + (te.tv_usec-ts.tv_usec)/1000000.0;
printf("time_h:[%lf]\n",time_h);
//	printf("handshake ok\n");	
//	printf("SSL connection using %s\n", SSL_get_cipher(ssl));
/*
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

	err = SSL_write(ssl,
                "-----This message is from the SSL server-----",
                strlen("-----This message is from the SSL server-----"));
*/
        RETURN_SSL(err);
	SSL_free(ssl);
	return;

}

void pthreads_locking_callback(int mode, int type, char *file,int line)
{
        if (mode & CRYPTO_LOCK)
        {
                pthread_mutex_lock(&(lock_cs[type]));
                lock_count[type]++;
        }
        else
        {
                pthread_mutex_unlock(&(lock_cs[type]));
        }
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
	int     verify_client = OFF; /* To verify a client certificate, set ON */

	int     listen_sock;
	int     sock;
	struct sockaddr_in sa_serv;
	struct sockaddr_in sa_cli;
	size_t client_len;
	char    *str;
	char    buf[1024 * 1024];

	SSL_CTX         *ctx = NULL;
	SSL             *ssl = NULL;
	const SSL_METHOD      *meth;

	short int       s_port = 4432;

	int ret = 0;
	int hsm_tag = 1;
	int aio_tag = 1;
	int error;

	char buff[1024 * 1024];
        int inl = 0;
        FILE *fp=NULL;
	struct timeval tss,tee;
	double time;
	int i = 0;
	pthread_t thread[NUM];

	ENGINE_load_gm(NULL, NULL);

	/* Load encryption & hashing algorithms for the SSL program */
	SSL_library_init();

	/* Load the error strings for SSL & CRYPTO APIs */
	SSL_load_error_strings();

	/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
	meth = SSLv23_server_method();

	/* Create a SSL_CTX structure */
	ctx = SSL_CTX_new(meth);

	if (!ctx)
	{
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	/* Load the server certificate into the SSL_CTX structure */
	if (SSL_CTX_use_certificate_file(ctx, SM2_SERVER_CERT, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	/* Load the private-key corresponding to the server certificate */
	if (SSL_CTX_use_PrivateKey_file(ctx, SM2_SERVER_KEY, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		exit(1);
	}


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

	lock_cs=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
        lock_count=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
        for (i=0; i<CRYPTO_num_locks(); i++)
        {
                lock_count[i]=0;
                pthread_mutex_init(&(lock_cs[i]),NULL);
        }
        CRYPTO_set_id_callback((unsigned long (*)())pthreads_thread_id);
        CRYPTO_set_locking_callback((void (*)())pthreads_locking_callback);
gettimeofday(&tss,NULL);
while(1)
{
	struct timeval tv;           
	fd_set fdset;            
	tv.tv_sec = 1;           
	tv.tv_usec = 0;          
	FD_ZERO(&fdset);          
	FD_SET(listen_sock, &fdset);
	select(listen_sock+1, &fdset, NULL, NULL, (struct timeval *)&tv);
if(FD_ISSET(listen_sock, &fdset))
{
	/* Socket for a TCP/IP connection is created */
	sock = accept(listen_sock, (struct sockaddr *)&sa_cli, (socklen_t *)&client_len);

	RETURN_ERR(sock, "accept");

	ssl = SSL_new(ctx);

	RETURN_NULL(ssl);

	SSL_set_fd(ssl, sock);

	/* Perform SSL Handshake on the SSL server */
	/*err = SSL_accept(ssl);*/
	ret=pthread_create(&thread[i],NULL,&thread_main,(void *)ssl);
	if(ret != 0 )
        {
        	printf("crear thread err[%d]\n",ret);
                return -1;
        }
	pthread_detach(thread[i]);

}
	/*------- DATA EXCHANGE - Receive message and send reply. -------*/
	/* Receive data from the SSL client */

}
gettimeofday(&tee,NULL);
time = tee.tv_sec-tss.tv_sec + (tee.tv_usec-tss.tv_usec)/1000000.0;
printf("time:[%lf]\n",time);

	err = SSL_shutdown(ssl);

	RETURN_SSL(err);

	/* Terminate communication on a socket */
	close(sock);
	close(listen_sock);

err:

	/* Free the SSL structure */
	if (ssl) SSL_free(ssl);

	/* Free the SSL_CTX structure */
	if (ctx) SSL_CTX_free(ctx);

	//WSACleanup();

	return 0;


}





