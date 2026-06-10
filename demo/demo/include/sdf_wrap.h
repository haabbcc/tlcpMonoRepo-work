#ifndef _SDF_WRAP_H_
#define _SDF_WRAP_H_

#include <openssl/evp.h>
#include <openssl/engine.h>
#include "public.h"

#if defined( __cplusplus )
extern "C" {
#endif

char g_devname[16];
char g_devserial[16];
unsigned int g_devsoftversion;
unsigned int g_asymalg;
unsigned int g_asymmode;
unsigned int g_symalg;
unsigned int g_hashalg;

int E_InitSDF(const char * pcPath);
int E_DestroySDF(void);

int cipher_init_wrap(CIPHER_DATA_CTX *ctx, void *phDevice,
	unsigned char *key, unsigned int keylen);
int cipher_do_wrap(CIPHER_DATA_CTX *ctx, unsigned char *out,
	size_t *outl, unsigned char *in, size_t inl);
int cipher_cleanup_wrap(CIPHER_DATA_CTX *ctx);

int E_create_ciphers(void);
void E_free_ciphers(void);
int E_ciphers(ENGINE *e, const EVP_CIPHER **cipher, const int **nids, int nid);

int E_pmeth_init(void);
int E_pkey_pmeths(ENGINE *e, EVP_PKEY_METHOD **pkey_meths,
	const int **nids, int nid);

EVP_PKEY *E_load_pubkey(ENGINE *e, const char *key_id, \
	UI_METHOD *ui_method, void *cb_data);
EVP_PKEY *E_load_prikey(ENGINE *e, const char *key_id,
	UI_METHOD *ui_method, void *cb_data);

#if defined(ENGINE_SDF_SM2)
int sm2_keygen(unsigned int index, char* password, EVP_PKEY *pkey);
int sm2_ext_verify(EVP_PKEY *pkey, \
	const unsigned char *sig, size_t siglen, unsigned char *tbs, size_t tbslen);
int sm2_pkey_init(EC_DATA_CTX *ec_ctx, void **handle);
void sm2_pkey_cleanup(EC_DATA_CTX *ec_ctx);
int sm2_sign(void *session, unsigned int index, \
	unsigned char *sig, size_t *siglen,
	const unsigned char *tbs, size_t tbslen);
int sm2_decrypt(void *session, unsigned int index, \
	unsigned char *out, size_t *outlen,
	const unsigned char *in, size_t inlen);
int sm2_ext_encrypt(EVP_PKEY *pkey, unsigned char *out, size_t *outlen,
	const unsigned char *in, size_t inlen);
int sm2_load_pubkey(const char *key_id, EVP_PKEY *pkey, int keytype);
#endif

#if defined(ENGINE_SDF_RSA)
int rsa_sign(unsigned int index, char* password, unsigned int passlen, \
	unsigned char *sig, size_t *siglen,
	const unsigned char *tbs, size_t tbslen);
int rsa_ext_verify(EVP_PKEY *pkey, \
	const unsigned char *sig, size_t siglen, unsigned char *tbs, size_t *tbslen);
int rsa_ext_encrypt(EVP_PKEY *pkey, unsigned char *out, size_t *outlen,
	const unsigned char *in, size_t inlen);
int rsa_decrypt(unsigned int index, char* password, unsigned int passlen, \
	unsigned char *out, size_t *outlen,
	const unsigned char *in, size_t inlen);
int rsa_load_pubkey(const char *key_id, EVP_PKEY *pkey, int keytype);
#endif

#if defined(ENGINE_SDF_HASH)
int E_md_init(void);
void E_md_free(void);
int E_digest_pmeths(ENGINE *e, const EVP_MD **md_meths, const int **nids, int nid);

int sm3_init_internal(void *session, EVP_PKEY *pkey);
int sm3_update_internal(void *session, const void *data, size_t);
int sm3_final_internal(void *session, unsigned char *md);
int sm3_cleanup_internal(void *session);
#endif

#if defined( __cplusplus )
}
#endif

#endif