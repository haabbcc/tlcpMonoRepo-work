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
#include "openssl/crypto.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/engine.h"
#include "e_sdf.h"
#include "public.h"
#include "e_sdf_err.h"

#define NUM 1 
#define MAX_BUF_LEN 4096
#define SM2_SERVER_CERT     "../certs/SS.crt"
#define SM2_SERVER_KEY      "../certs/SS.key"

#define TEST_SERVER_KEY      "../certs/gm_server.pem"

#define SM2_SERVER_ENC_CERT     "../certs/SE.crt"
#define SM2_SERVER_ENC_KEY      "../certs/SE.key"

#define SM2_SERVER_CA_CERT  "../certs/CA.crt"
#define SDF_SERVER_CERT "../certs/signature.cer"


#define SM2_SERVER_CA_PATH  "."
#define SSL_ERROR_WANT_HSM_RESULT 10
#define ON   1
#define OFF  0

#define RETURN_NULL(x) if ((x)==NULL) exit(1)
#define RETURN_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define RETURN_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }
int opt = 1000;


int pthread_i = 0;


void *thread_main(void *arg)
{
    int     err;

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

        short int       s_port = 5533;

        int ret = 0;
        int hsm_tag = 1;
        int aio_tag = 1;
        int error;
        int nid = 0;
        ENGINE *e;

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

	SSL_library_init();

	SSL_load_error_strings();

//	ENGINE_load_sdf();

	printf("ENGINE_by_id start\n");
	
#if 1 
	e = ENGINE_by_id("sdf_cipher");
	printf("ENGINE_by_id over\n");
        if (e == NULL)
        {
	     printf("e is null!!!!!!!!!!!!!\n");
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
	meth = SSLv23_server_method();

	ctx = SSL_CTX_new(meth);

	if (!ctx)
        {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
printf("out ctx:%p\n",ctx);
//printf("e1:%p\n",ENGINE_by_id("sdf_cipher"));
	if (SSL_CTX_use_certificate_file(ctx, SDF_SERVER_CERT, SSL_FILETYPE_PEM) <= 0)
	{
                ERR_print_errors_fp(stderr);
                exit(1);
        }
printf("cert set ok\n");
//printf("e2:%p\n",ENGINE_by_id("sdf_cipher"));
	if (SSL_CTX_use_PrivateKey_file(ctx, "engine:sdf_cipher:2:12345678", SSL_FILETYPE_SDF) <= 0)
	{
                ERR_print_errors_fp(stderr);
                exit(1);
        }

printf("key set ok\n");
//printf("e3:%p\n",ENGINE_by_id("sdf_cipher"));
	if (!SSL_CTX_set_cipher_list(ctx, "ECC-SM4-SM3")) {
                ERR_print_errors_fp(stderr);
                printf("set cipher list fail!\n");
        	exit(1);
	}
printf("set cipher ok\n");
	listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        ret = setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
        if (ret == -1)
        {
                printf("set socket erro\n");
                exit(1);

        }

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

	err = listen(listen_sock, 5);

        RETURN_ERR(err, "listen");
        client_len = sizeof(sa_cli);

	sock = accept(listen_sock, (struct sockaddr *)&sa_cli, (socklen_t *)&client_len);

        RETURN_ERR(sock, "accept");

	ssl = SSL_new(ctx);

        RETURN_NULL(ssl);

	SSL_set_fd(ssl, sock);

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
                        }
                }
                else
                        break;
        }
printf("server handshake ok\n");
        RETURN_SSL(err);

	printf("SSL connection using %s\n", SSL_get_cipher(ssl));

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

printf("SSL write over\n");
        RETURN_SSL(err);

	err = SSL_shutdown(ssl);

        RETURN_SSL(err);

}

int main()
{
    int i = 0;
    pthread_t thread[NUM];
    int ret =0;

    for(i=0;i<NUM;i++)    
    {         
	ret=pthread_create(&thread[i],NULL,&thread_main,NULL); 
	if(ret!=0)             
	{                     
		printf("pthread_create err[%d]\n",ret);    
		continue;     
	}  
	usleep(250);   
    }

    for(i=0;i<NUM;i++)
    {
	ret = pthread_join(thread[i],NULL);
        if(ret == 0)
        {
        	printf("thread join[%d]\n",i);
        }
        else
        {
                printf("join fail/n");
                return -1;
        }
    }
 
    return 0;
}

