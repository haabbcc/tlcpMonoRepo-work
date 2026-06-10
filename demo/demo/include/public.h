#ifndef _HEADER_PUBLIC_H_
#define _HEADER_PUBLIC_H_

#ifndef OPENSSL_NO_SDF

#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include "e_sdf.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ERR_PRINTF(format, ...)   printf("[%s]-[%05d]: "format"\n",\
 __FUNCTION__,__LINE__, ##__VA_ARGS__)

#if defined(__DEBUG__)
#define DBG_PRINTF(format, ...)   printf("[%s]-[%05d]: "format"\n",\
 __FUNCTION__,__LINE__, ##__VA_ARGS__)

#define DBG_TRACE do\
{\
	DBG_PRINTF("trace");\
}while (0);
#else
#define DBG_PRINTF(format, ...)
#define	DBG_TRACE
#endif

#define SDF_CHECK(f) do \
{\
    if((ret =(f)) !=0 )\
	{\
        ERR_PRINTF("sdf return error[0x%08x], call func[%s]",ret, #f);\
        goto end;\
	}\
    DBG_PRINTF("%s success", #f);\
}while(0)

#define SDF_CHECK_RET(f) do \
{\
    if((ret =(f)) !=0 )\
	{\
        ERR_PRINTF("sdf return error[0x%08x], call func[%s]",ret, #f);\
        return(-1);\
	}\
    DBG_PRINTF("%s success", #f);\
}while(0)

# define SDF_ENG_NELEM(x)    (sizeof(x)/sizeof((x)[0]))


/** SM4运算上下文 */
typedef struct cipher_data_ctx_st {
	/** 密钥 */
	unsigned char passwd[32];
	unsigned int passwd_len;
	/** working iv */
	unsigned char iv[EVP_MAX_IV_LENGTH];
	/**  加解密标识， 1--加密，0--解密 */
	int encrypt;
	unsigned int mode, kekidx;
	void *hsession;
	void *hkey;
	int algid;
	unsigned char remainbuf[32];
	unsigned int remainlen;
	/** 用户自定义数据 */
	void *app_data;
}CIPHER_DATA_CTX;

/** SM3运算上下文 */
typedef struct md_data_ctx_st {
	void *hsession;
	/** 用户自定义数据 */
	void *app_data;
}MD_DATA_CTX;

/** SM2运算上下文 */
typedef struct ec_data_ctx_st {
	unsigned char passwd[32];
	unsigned char mduserid[32];
	unsigned int index;
	void *hsession;
	/** 用户自定义数据 */
	void *app_data;
}EC_DATA_CTX;

#define SM2_SIGN_KEY 1
#define SM2_ENC_KEY 2
#define RSA_SIGN_KEY 3
#define RSA_ENC_KEY 4

/* EC pkey context structure */
typedef struct {
	/* Key and paramgen group */
	EC_GROUP *gen_group;
	/* message digest */
	const EVP_MD *md;
	/* Distinguishing Identifier, ISO/IEC 15946-3 */
	uint8_t *id;
	size_t id_len;
	/* id_set indicates if the 'id' field is set (1) or not (0) */
	int id_set;
#ifndef OPENSSL_NO_HKGM
	/* Duplicate key if custom cofactor needed */
	EC_KEY *co_key;
	/* Cofactor mode */
	signed char cofactor_mode;
	/* KDF (if any) to use for ECDH */
	char kdf_type;
	/* Message digest to use for key derivation */
	const EVP_MD *kdf_md;
	/* User key material */
	unsigned char *kdf_ukm;
	size_t kdf_ukmlen;
	/* KDF output length */
	size_t kdf_outlen;
	/* server tag */
	int server;
	/* peer uid */
	char *peer_id;
	/* self uid */
	char *self_id;
	/* peer uid length */
	int peerid_len;
	/* self uid length */
	int selfid_len;
	/* peer ephemeral public key */
	EC_KEY *peer_ecdhe_key;
	/* self ephemeral key */
	EC_KEY *self_ecdhe_key;
	/* sm2/ecc encrypt out format, 0 for ASN1 */
	int encdata_format;
#endif
} SM2_PKEY_CTX;

/* RSA pkey context structure */
typedef struct {
	/* Key gen parameters */
	int nbits;
	BIGNUM *pub_exp;
	int primes;
	/* Keygen callback info */
	int gentmp[2];
	/* RSA padding mode */
	int pad_mode;
	/* message digest */
	const EVP_MD *md;
	/* message digest for MGF1 */
	const EVP_MD *mgf1md;
	/* PSS salt length */
	int saltlen;
	/* Minimum salt length or -1 if no PSS parameter restriction */
	int min_saltlen;
	/* Temp buffer */
	unsigned char *tbuf;
	/* OAEP label */
	unsigned char *oaep_label;
	size_t oaep_labellen;
} RSA_PKEY_CTX;

#define ENGINE_SDF_SM2
//#define ENGINE_SDF_HASH
#define ENGINE_SDF_RSA
#define ENGINE_DSO

#define SDF_ECC_MAX_BITS                 512 
#define SDF_ECC_MAX_LEN                  ((SDF_ECC_MAX_BITS+7) / 8)
#define SDF_ECC_MAX_CIPHER_LEN           136


#define EGN_SDF_NELEM(x)    (sizeof(x)/sizeof(x[0]))

/****  中国  ****/
#define ASN1_SN_CN    "CHINA"
#define ASN1_LN_CN    "china"
#define ASN1_OBJ_CN    "1.2.156"
/****  国家密码管理局  ****/
#define ASN1_SN_SCA    "SCA"
#define ASN1_LN_SCA    "sca"
#define ASN1_OBJ_SCA    "1.2.156.197"
/****  密码行业标准化技术委员会  ****/
#define ASN1_SN_CCSTC    "CCSTC"
#define ASN1_LN_CCSTC    "ccstc"
#define ASN1_OBJ_CCSTC    "1.2.156.10197"
/****  密码算法  ****/
#define ASN1_SN_CryAlg    "CRYALG"
#define ASN1_LN_CryAlg    "cryalg"
#define ASN1_OBJ_CryAlg    "1.2.156.10197.1"
/****  分组密码算法  ****/
#define ASN1_SN_BlockCipher    "BLOCK_CIPHER"
#define ASN1_LN_BlockCipher    "block_cipher"
#define ASN1_OBJ_BlockCipher    "1.2.156.10197.1.100"
/****  SM1分组密码算法  ****/
#define ASN1_SN_SM1    "SM1"
#define ASN1_LN_SM1    "sm1"
#define ASN1_OBJ_SM1    "1.2.156.10197.1.102"
/****  SM1分组密码算法-ECB模式  ****/
#define ASN1_SN_SM1_ECB    "SM1_ECB"
#define ASN1_LN_SM1_ECB    "sm1_ecb"
#define ASN1_OBJ_SM1_ECB    "1.2.156.10197.1.102.1"
/****  SM1分组密码算法-CBC模式  ****/
#define ASN1_SN_SM1_CBC    "SM1_CBC"
#define ASN1_LN_SM1_CBC    "sm1_cbc"
#define ASN1_OBJ_SM1_CBC    "1.2.156.10197.1.102.2"
/****  SM1分组密码算法-OFB模式  ****/
#define ASN1_SN_SM1_OFB    "SM1_OFB"
#define ASN1_LN_SM1_OFB    "sm1_ofb"
#define ASN1_OBJ_SM1_OFB    "1.2.156.10197.1.102.3"
/****  SM1分组密码算法-CFB模式  ****/
#define ASN1_SN_SM1_CFB    "SM1_CFB"
#define ASN1_LN_SM1_CFB    "sm1_cfb"
#define ASN1_OBJ_SM1_CFB    "1.2.156.10197.1.102.4"
/****  SM1分组密码算法-CTR模式  ****/
#define ASN1_SN_SM1_CTR    "SM1_CTR"
#define ASN1_LN_SM1_CTR    "sm1_ctr"
#define ASN1_OBJ_SM1_CTR    "1.2.156.10197.1.102.5"
/****  SM1分组密码算法-MAC模式  ****/
#define ASN1_SN_SM1_MAC    "SM1_MAC"
#define ASN1_LN_SM1_MAC    "sm1_mac"
#define ASN1_OBJ_SM1_MAC    "1.2.156.10197.1.102.6"
/****  SSF33分组密码算法  ****/
#define ASN1_SN_SSF33    "SSF33"
#define ASN1_LN_SSF33    "ssf33"
#define ASN1_OBJ_SSF33    "1.2.156.10197.1.103"
/****  SSF33分组密码算法-ECB模式  ****/
#define ASN1_SN_SSF33_ECB    "SSF33_ECB"
#define ASN1_LN_SSF33_ECB    "ssf33_ecb"
#define ASN1_OBJ_SSF33_ECB    "1.2.156.10197.1.103.1"
/****  SSF33分组密码算法-CBC模式  ****/
#define ASN1_SN_SSF33_CBC    "SSF33_CBC"
#define ASN1_LN_SSF33_CBC    "ssf33_cbc"
#define ASN1_OBJ_SSF33_CBC    "1.2.156.10197.1.103.2"
/****  SSF33分组密码算法-OFB模式  ****/
#define ASN1_SN_SSF33_OFB    "SSF33_OFB"
#define ASN1_LN_SSF33_OFB    "ssf33_ofb"
#define ASN1_OBJ_SSF33_OFB    "1.2.156.10197.1.103.3"
/****  SSF33分组密码算法-CFB模式  ****/
#define ASN1_SN_SSF33_CFB    "SSF33_CFB"
#define ASN1_LN_SSF33_CFB    "ssf33_cfb"
#define ASN1_OBJ_SSF33_CFB    "1.2.156.10197.1.103.4"
/****  SSF33分组密码算法-CFB1模式  ****/
#define ASN1_SN_SSF33_CFB1    "SSF33_CFB1"
#define ASN1_LN_SSF33_CFB1    "ssf33_cfb1"
#define ASN1_OBJ_SSF33_CFB1    "1.2.156.10197.1.103.5"
/****  SSF33分组密码算法-CFB8模式  ****/
#define ASN1_SN_SSF33_CFB8    "SSF33_CFB8"
#define ASN1_LN_SSF33_CFB8    "ssf33_cfb8"
#define ASN1_OBJ_SSF33_CFB8    "1.2.156.10197.1.103.6"
/****  SM4分组密码算法  ****/
#define ASN1_SN_SM4    "SM4"
#define ASN1_LN_SM4    "sm4"
#define ASN1_OBJ_SM4    "1.2.156.10197.1.104"
/****  SM4分组密码算法-ECB模式  ****/
#define ASN1_SN_SM4_ECB    "SM4_ECB"
#define ASN1_LN_SM4_ECB    "sm4_ecb"
#define ASN1_OBJ_SM4_ECB    "1.2.156.10197.1.104.1"
/****  SM4分组密码算法-CBC模式  ****/
#define ASN1_SN_SM4_CBC    "SM4_CBC"
#define ASN1_LN_SM4_CBC    "sm4_cbc"
#define ASN1_OBJ_SM4_CBC    "1.2.156.10197.1.104.2"
/****  SM4分组密码算法-OFB模式  ****/
#define ASN1_SN_SM4_OFB    "SM4_OFB"
#define ASN1_LN_SM4_OFB    "sm4_ofb"
#define ASN1_OBJ_SM4_OFB    "1.2.156.10197.1.104.3"
/****  SM4分组密码算法-CFB模式  ****/
#define ASN1_SN_SM4_CFB    "SM4_CFB"
#define ASN1_LN_SM4_CFB    "sm4_cfb"
#define ASN1_OBJ_SM4_CFB    "1.2.156.10197.1.104.4"
/****  SM4分组密码算法-CFB1模式  ****/
#define ASN1_SN_SM4_CFB1    "SM4_CFB1"
#define ASN1_LN_SM4_CFB1    "sm4_cfb1"
#define ASN1_OBJ_SM4_CFB1    "1.2.156.10197.1.104.5"
/****  SM4分组密码算法-CFB8模式  ****/
#define ASN1_SN_SM4_CFB8    "SM4_CFB8"
#define ASN1_LN_SM4_CFB8    "sm4_cfb8"
#define ASN1_OBJ_SM4_CFB8    "1.2.156.10197.1.104.6"
/****  SM4分组密码算法-CTR模式  ****/
#define ASN1_SN_SM4_CTR    "SM4_CTR"
#define ASN1_LN_SM4_CTR    "sm4_ctr"
#define ASN1_OBJ_SM4_CTR    "1.2.156.10197.1.104.7"
/****  SM4分组密码算法-GCM模式  ****/
#define ASN1_SN_SM4_GCM    "SM4_GCM"
#define ASN1_LN_SM4_GCM    "sm4_gcm"
#define ASN1_OBJ_SM4_GCM    "1.2.156.10197.1.104.8"
/****  SM4分组密码算法-CCM模式  ****/
#define ASN1_SN_SM4_CCM    "SM4_CCM"
#define ASN1_LN_SM4_CCM    "sm4_ccm"
#define ASN1_OBJ_SM4_CCM    "1.2.156.10197.1.104.9"
/****  SM4分组密码算法-XTS模式  ****/
#define ASN1_SN_SM4_XTS    "SM4_XTS"
#define ASN1_LN_SM4_XTS    "sm4_xts"
#define ASN1_OBJ_SM4_XTS    "1.2.156.10197.1.104.10"
/****  SM4分组密码算法-WRAP模式  ****/
#define ASN1_SN_SM4_WRAP    "SM4_WRAP"
#define ASN1_LN_SM4_WRAP    "sm4_wrap"
#define ASN1_OBJ_SM4_WRAP    "1.2.156.10197.1.104.11"
/****  SM4分组密码算法-WRAP_PAD模式  ****/
#define ASN1_SN_SM4_WRAP_PAD    "SM4_WRAP_PAD"
#define ASN1_LN_SM4_WRAP_PAD    "sm4_wrap_pad"
#define ASN1_OBJ_SM4_WRAP_PAD    "1.2.156.10197.1.104.12"
/****  SM4分组密码算法-OCB模式  ****/
#define ASN1_SN_SM4_OCB    "SM4_OCB"
#define ASN1_LN_SM4_OCB    "sm4_ocb"
#define ASN1_OBJ_SM4_OCB    "1.2.156.10197.1.104.100"
/****  序列密码算法  ****/
#define ASN1_SN_StreamCipher    "STREAM_CIPHER"
#define ASN1_LN_StreamCipher    "stream_cipher"
#define ASN1_OBJ_StreamCipher    "1.2.156.10197.1.200"
/****  祖冲之序列密码算法  ****/
#define ASN1_SN_ZUC    "ZUC"
#define ASN1_LN_ZUC    "zuc"
#define ASN1_OBJ_ZUC    "1.2.156.10197.1.201"
/****  公钥密码算法  ****/
#define ASN1_SN_PublickeyCipher    "PUBLICKEY_CIPHER"
#define ASN1_LN_PublickeyCipher    "publickey_cipher"
#define ASN1_OBJ_PublickeyCipher    "1.2.156.10197.1.300"
/****  SM2椭圆曲线公钥密码算法  ****/
//#define ASN1_SN_SM2    "SM2"
#define ASN1_SN_SM2    "SM2"
#define ASN1_LN_SM2    "sm2"
#define ASN1_OBJ_SM2    "1.2.156.10197.1.301"
/****  SM2数字签名算法  ****/
#define ASN1_SN_SM2_Sign    "SM2_SIGN"
#define ASN1_LN_SM2_Sign    "sm2_sign"
#define ASN1_OBJ_SM2_Sign    "1.2.156.10197.1.301.1"
/****  SM2密钥交换协议  ****/
#define ASN1_SN_SM2_KeyExchange    "SM2_KEY_EXCHANGE"
#define ASN1_LN_SM2_KeyExchange    "sm2_key_exchange"
#define ASN1_OBJ_SM2_KeyExchange    "1.2.156.10197.1.301.2"
/****  SM2公钥加密算法  ****/
#define ASN1_SN_SM2_Encrypt    "SM2_ENCRYPT"
#define ASN1_LN_SM2_Encrypt    "sm2_encrypt"
#define ASN1_OBJ_SM2_Encrypt    "1.2.156.10197.1.301.3"
/****  SM2公钥加密算法推荐参数  ****/
#define ASN1_SN_SM2_Encrypt_RecomPara    "SM2_ENCRYPT_RECOMPARA"
#define ASN1_LN_SM2_Encrypt_RecomPara    "sm2_encrypt_recompara"
#define ASN1_OBJ_SM2_Encrypt_RecomPara    "1.2.156.10197.1.301.3.1"
/****  SM2公钥加密算法特殊参数  ****/
#define ASN1_SN_SM2_Encrypt_SpecPara    "SM2_ENCRYPT_SPECPARA"
#define ASN1_LN_SM2_Encrypt_SpecPara    "sm2_encrypt_specpara"
#define ASN1_OBJ_SM2_Encrypt_SpecPara    "1.2.156.10197.1.301.3.2"
/****  SM2公钥加密算法WAPIP192V1  ****/
#define ASN1_SN_SM2_Encrypt_WAPIP192V1    "SM2_ENCRYPT_WAPIP192V1"
#define ASN1_LN_SM2_Encrypt_WAPIP192V1    "sm2_encrypt_wapip192v1"
#define ASN1_OBJ_SM2_Encrypt_WAPIP192V1    "1.2.156.10197.1.301.101"
/****  杂凑算法  ****/
#define ASN1_SN_DigestAlg    "DIGEST_ALGORITHM"
#define ASN1_LN_DigestAlg    "digest_algorithm"
#define ASN1_OBJ_DigestAlg    "1.2.156.10197.1.400"
/****  SM3密码杂凑算法  ****/
#define ASN1_SN_SM3    "SM3"
#define ASN1_LN_SM3    "sm3"
#define ASN1_OBJ_SM3    "1.2.156.10197.1.401"
/****  SM3密码杂凑算法，无密钥使用  ****/
#define ASN1_SN_SM3_NoKey    "SM3_NOKEY"
#define ASN1_LN_SM3_NoKey    "sm3_nokey"
#define ASN1_OBJ_SM3_NoKey    "1.2.156.10197.1.401.1"
/****  SM3密码杂凑算法，有密钥使用  ****/
#define ASN1_SN_SM3_WithKey    "SM3_WITHKEY"
#define ASN1_LN_SM3_WithKey    "sm3_withkey"
#define ASN1_OBJ_SM3_WithKey    "1.2.156.10197.1.401.2"
/****  组合运算机制  ****/
#define ASN1_SN_CombineOperation    "COMBINE_OPERATION"
#define ASN1_LN_CombineOperation    "combine_operation"
#define ASN1_OBJ_CombineOperation    "1.2.156.10197.1.500"
/****  基于SM2算法和SM3算法的签名  ****/
#define ASN1_SN_SM2WithSM3    "SM2WITHSM3"
#define ASN1_LN_SM2WithSM3    "sm2withsm3"
#define ASN1_OBJ_SM2WithSM3    "1.2.156.10197.1.501"
/****  基于RSA算法和SM3算法的签名  ****/
#define ASN1_SN_RSAWithSM3    "RSAWITHSM3"
#define ASN1_LN_RSAWithSM3    "rsawithsm3"
#define ASN1_OBJ_RSAWithSM3    "1.2.156.10197.1.502"
/****  基于SM2算法和SHA256算法的签名  ****/
#define ASN1_SN_SM2WithSHA256    "SM2WITHSHA256"
#define ASN1_LN_SM2WithSHA256    "sm2withsha256"
#define ASN1_OBJ_SM2WithSHA256    "1.2.156.10197.1.503"
/****  基于SM2算法和SHA224算法的签名  ****/
#define ASN1_SN_SM2WithSHA224    "SM2WITHSHA224"
#define ASN1_LN_SM2WithSHA224    "sm2withsha224"
#define ASN1_OBJ_SM2WithSHA224    "1.2.156.10197.1.505"
/****  基于SM2算法和SHA384算法的签名  ****/
#define ASN1_SN_SM2WithSHA384    "SM2WITHSHA384"
#define ASN1_LN_SM2WithSHA384    "sm2withsha384"
#define ASN1_OBJ_SM2WithSHA384    "1.2.156.10197.1.506"
/****  基于SM2算法和RMD160算法的签名  ****/
#define ASN1_SN_SM2WithRMD160    "SM2WITHRMD160"
#define ASN1_LN_SM2WithRMD160    "sm2withrmd160"
#define ASN1_OBJ_SM2WithRMD160    "1.2.156.10197.1.507"
/****  CA代码  ****/
#define ASN1_SN_CA    "CA"
#define ASN1_LN_CA    "ca"
#define ASN1_OBJ_CA    "1.2.156.10197.4.3"
/****  SM2密码算法加密签名消息语法规范-数据类型dat  ****/
#define ASN1_SN_SM2_DATA    "SM2_DATA"
#define ASN1_LN_SM2_DATA    "sm2_data"
#define ASN1_OBJ_SM2_DATA    "1.2.156.10197.6.1.4.2.1"
/****  SM9标识密码算法  ****/
#define ASN1_SN_SM9    "SM9"
#define ASN1_LN_SM9    "sm9"
#define ASN1_OBJ_SM9    "1.2.156.10197.1.302"
/****  SM9数字密码算法  ****/
#define ASN1_SN_SM9_Sign    "SM9_SIGN"
#define ASN1_LN_SM9_Sign    "sm9_sign"
#define ASN1_OBJ_SM9_Sign    "1.2.156.10197.1.302.1"
/****  SM9密钥交换协议  ****/
#define ASN1_SN_SM9_KeyExchange    "SM9_KEY_EXCHANGE"
#define ASN1_LN_SM9_KeyExchange    "sm9_key_exchange"
#define ASN1_OBJ_SM9_KeyExchange    "1.2.156.10197.1.302.2"
/****  SM9公钥加密  ****/
#define ASN1_SN_SM9_Encrypt    "SM9_ENCRYPT"
#define ASN1_LN_SM9_Encrypt    "sm9_encrypt"
#define ASN1_OBJ_SM9_Encrypt    "1.2.156.10197.1.302.3"

typedef enum enum_NID {
	ASN1_NID_BEGIN = 0,
	ASN1_NID_CN = 1,
	ASN1_NID_SCA = 2,
	ASN1_NID_CCSTC = 3,
	ASN1_NID_CryAlg = 4,
	ASN1_NID_BlockCipher = 5,
	ASN1_NID_SM1 = 6,
	ASN1_NID_SM1_ECB = 7,
	ASN1_NID_SM1_CBC = 8,
	ASN1_NID_SM1_OFB = 9,
	ASN1_NID_SM1_CFB = 10,
	ASN1_NID_SSF33 = 11,
	ASN1_NID_SSF33_ECB = 12,
	ASN1_NID_SSF33_CBC = 13,
	ASN1_NID_SSF33_OFB = 14,
	ASN1_NID_SSF33_CFB = 15,
	ASN1_NID_SSF33_CFB1 = 16,
	ASN1_NID_SSF33_CFB8 = 17,
	ASN1_NID_SM4 = 18,
	ASN1_NID_SM4_ECB = 19,
	ASN1_NID_SM4_CBC = 20,
	ASN1_NID_SM4_OFB = 21,
	ASN1_NID_SM4_CFB = 22,
	ASN1_NID_SM4_CFB1 = 23,
	ASN1_NID_SM4_CFB8 = 24,
	ASN1_NID_SM4_CTR = 25,
	ASN1_NID_SM4_GCM = 26,
	ASN1_NID_SM4_CCM = 27,
	ASN1_NID_SM4_XTS = 28,
	ASN1_NID_SM4_WRAP = 29,
	ASN1_NID_SM4_WRAP_PAD = 30,
	ASN1_NID_SM4_OCB = 31,
	ASN1_NID_StreamCipher = 32,
	ASN1_NID_ZUC = 33,
	ASN1_NID_PublickeyCipher = 34,
	ASN1_NID_SM2 = 35,
	ASN1_NID_SM2_Sign = 36,
	ASN1_NID_SM2_KeyExchange = 37,
	ASN1_NID_SM2_Encrypt = 38,
	ASN1_NID_SM2_Encrypt_RecomPara = 39,
	ASN1_NID_SM2_Encrypt_SpecPara = 40,
	ASN1_NID_SM2_Encrypt_WAPIP192V1 = 41,
	ASN1_NID_DigestAlg = 42,
	ASN1_NID_SM3 = 43,
	ASN1_NID_SM3_NoKey = 44,
	ASN1_NID_SM3_WithKey = 45,
	ASN1_NID_CombineOperation = 46,
	ASN1_NID_SM2WithSM3 = 47,
	ASN1_NID_RSAWithSM3 = 48,
	ASN1_NID_SM2WithSHA256 = 49,
	ASN1_NID_SM2WithSHA224 = 50,
	ASN1_NID_SM2WithSHA384 = 51,
	ASN1_NID_SM2WithRMD160 = 52,
	ASN1_NID_CA = 53,
	ASN1_NID_SM2_DATA = 54,
	ASN1_NID_SM9 = 55,
	ASN1_NID_SM9_Sign = 56,
	ASN1_NID_SM9_KeyExchange = 57,
	ASN1_NID_SM9_Encrypt = 58,
	//ASN1_NID_SM1_MAC = 59,
	ASN1_NID_SM1_CTR = 59,
	ASN1_NID_NUMBER = 60,
	ASN1_NID_EC = 61,
}ENUM_NID;
#if defined(NID_aes_128_cfb128) && ! defined (NID_aes_128_cfb)
#define NID_aes_128_cfb NID_aes_128_cfb128
#endif
#if defined(NID_aes_128_ofb128) && ! defined (NID_aes_128_ofb)
#define NID_aes_128_ofb NID_aes_128_ofb128
#endif
#if defined(NID_aes_192_cfb128) && ! defined (NID_aes_192_cfb)
#define NID_aes_192_cfb NID_aes_192_cfb128
#endif
#if defined(NID_aes_192_ofb128) && ! defined (NID_aes_192_ofb)
#define NID_aes_192_ofb NID_aes_192_ofb128
#endif
#if defined(NID_aes_256_cfb128) && ! defined (NID_aes_256_cfb)
#define NID_aes_256_cfb NID_aes_256_cfb128
#endif
#if defined(NID_aes_256_ofb128) && ! defined (NID_aes_256_ofb)
#define NID_aes_256_ofb NID_aes_256_ofb128
#endif

#define SDF_NID_des1_ecb NID_des_ecb
#define SDF_NID_des1_cbc NID_des_cbc
#define SDF_NID_des3_ecb NID_des_ede3_ecb
#define SDF_NID_des3_cbc NID_des_ede3_cbc

#define SDF_NID_aes128_ecb NID_aes_128_ecb
#define SDF_NID_aes128_cbc NID_aes_128_cbc
#define SDF_NID_aes128_ofb NID_aes_128_ofb128
#define SDF_NID_aes128_cfb NID_aes_128_cfb128
#define SDF_NID_aes128_ctr NID_aes_128_ctr

#define SDF_NID_aes192_ecb NID_aes_192_ecb
#define SDF_NID_aes192_cbc NID_aes_192_cbc
#define SDF_NID_aes192_ofb NID_aes_192_ofb128
#define SDF_NID_aes192_cfb NID_aes_192_cfb128
#define SDF_NID_aes192_ctr NID_aes_192_ctr

#define SDF_NID_aes256_ecb NID_aes_256_ecb
#define SDF_NID_aes256_cbc NID_aes_256_cbc
#define SDF_NID_aes256_ofb NID_aes_256_ofb128
#define SDF_NID_aes256_cfb NID_aes_256_cfb128
#define SDF_NID_aes256_ctr NID_aes_256_ctr

#define SDF_NID_sm1_ecb 0
#define SDF_NID_sm1_cbc 0
#define SDF_NID_sm1_ofb 0
#define SDF_NID_sm1_cfb 0
#define SDF_NID_sm1_ctr 0
#define SDF_NID_sm1_mac 0

#define SDF_NID_sm4_ecb 0
#define SDF_NID_sm4_cbc 0
#define SDF_NID_sm4_ofb 0
#define SDF_NID_sm4_cfb 0
#define SDF_NID_sm4_ctr 0
#define SDF_NID_sm4_mac 0

/*#ifndef OPENSSL_NO_SM3

#define SM3_DIGEST_LENGTH	32
#define SM3_BLOCK_SIZE		64
#define SM3_CBLOCK		(SM3_BLOCK_SIZE)
#define SM3_HMAC_SIZE		(SM3_DIGEST_LENGTH)
#endif*/
#define SDF_SHA1_CBLOCK             (SHA_CBLOCK)
#define SDF_SHA224_CBLOCK           (SHA256_CBLOCK)
#define SDF_SHA256_CBLOCK           (SHA256_CBLOCK)
#define SDF_SHA384_CBLOCK           (SHA512_CBLOCK)
#define SDF_SHA512_CBLOCK           (SHA512_CBLOCK)
#define SDF_SM3_CBLOCK              64

#define SDF_NID_sha1                (NID_sha1)
#define SDF_NID_sha256              (NID_sha256)
#define SDF_NID_sha224              (NID_sha224)
#define SDF_NID_sha384              (NID_sha384)
#define SDF_NID_sha512              (NID_sha512)
#define SDF_NID_sm3                 0

#define SDF_NID_sha1_PKEY           (NID_sha1WithRSAEncryption) 
#define SDF_NID_sha224_PKEY         (NID_sha224WithRSAEncryption)
#define SDF_NID_sha256_PKEY         (NID_sha256WithRSAEncryption)
#define SDF_NID_sha384_PKEY         (NID_sha384WithRSAEncryption)
#define SDF_NID_sha512_PKEY         (NID_sha512WithRSAEncryption)
#define SDF_NID_sm3_PKEY            0

#define SHA1_DIGEST_LENGTH          (SHA_DIGEST_LENGTH)
#define SM3_DIGEST_LENGTH	        32

#define SDF_SM2_DEFAULT_USERID           "1234567812345678"
#define SDF_SM2_DEFAULT_ID_LENGTH	     (sizeof(SDF_SM2_DEFAULT_USERID) - 1)

#define SDF_AES_BLOCK_SIZE           16
#define SDF_DES_BLOCK_SIZE           8
#define SDF_SM1_BLOCK_SIZE		     16
#define SDF_SM4_BLOCK_SIZE		     16

#define SDF_AES128_KEY_SIZE          16
#define SDF_AES192_KEY_SIZE          24
#define SDF_AES256_KEY_SIZE          32
#define SDF_DES1_KEY_SIZE            8
#define SDF_DES3_KEY_SIZE            24
#define SDF_SM1_KEY_SIZE             16
#define SDF_SM4_KEY_SIZE             16

#define SDF_AES_BLOCK_SIZE_ECB       (SDF_AES_BLOCK_SIZE)
#define SDF_AES_BLOCK_SIZE_CBC       (SDF_AES_BLOCK_SIZE)
#define SDF_AES_BLOCK_SIZE_OFB       1
#define SDF_AES_BLOCK_SIZE_CFB       1
#define SDF_AES_BLOCK_SIZE_CTR       1
#define SDF_AES_BLOCK_SIZE_CCM       1
#define SDF_AES_BLOCK_SIZE_GCM       1
#define SDF_DES_BLOCK_SIZE_ECB       (SDF_DES_BLOCK_SIZE)
#define SDF_DES_BLOCK_SIZE_CBC       (SDF_DES_BLOCK_SIZE)
#define SDF_SM_BLOCK_SIZE_ECB		 (SDF_SM1_BLOCK_SIZE)
#define SDF_SM_BLOCK_SIZE_CBC		 (SDF_SM1_BLOCK_SIZE)
#define SDF_SM_BLOCK_SIZE_OFB		 (SDF_SM1_BLOCK_SIZE)
#define SDF_SM_BLOCK_SIZE_CFB		 1
#define SDF_SM_BLOCK_SIZE_CTR		 1
#define SDF_SM_BLOCK_SIZE_MAC		 1

#define SDF_AES_IV_LEN_ECB  0
#define SDF_AES_IV_LEN_CBC  16
#define SDF_AES_IV_LEN_OFB  16
#define SDF_AES_IV_LEN_CFB  16
#define SDF_AES_IV_LEN_CTR  16
#define SDF_AES_IV_LEN_CCM  12
#define SDF_AES_IV_LEN_GCM  12
#define SDF_DES_IV_LEN_ECB  0
#define SDF_DES_IV_LEN_CBC  8
#define SDF_DES3_IV_LEN_EBC 0
#define SDF_DES3_IV_LEN_CBC 8
#define SDF_SM_IV_LEN_ECB   0
#define SDF_SM_IV_LEN_CBC   16
#define SDF_SM_IV_LEN_OFB   16
#define SDF_SM_IV_LEN_CFB   16
#define SDF_SM_IV_LEN_MAC   16
#define SDF_SM_IV_LEN_CTR   16
#define SDF_SM_IV_LEN_CCM   12
#define SDF_SM_IV_LEN_GCM   12


#define CUSTOM_FLAGS    (EVP_CIPH_FLAG_DEFAULT_ASN1 \
                | EVP_CIPH_CUSTOM_IV | EVP_CIPH_FLAG_CUSTOM_CIPHER \
                | EVP_CIPH_ALWAYS_CALL_INIT | EVP_CIPH_CTRL_INIT \
                | EVP_CIPH_CUSTOM_COPY)


#define SDF_AES_CIPH_FLAGS_ECB  EVP_CIPH_FLAG_DEFAULT_ASN1
#define SDF_AES_CIPH_FLAGS_CBC  EVP_CIPH_FLAG_DEFAULT_ASN1
#define SDF_AES_CIPH_FLAGS_OFB  EVP_CIPH_FLAG_DEFAULT_ASN1
#define SDF_AES_CIPH_FLAGS_CFB  EVP_CIPH_FLAG_DEFAULT_ASN1
#define SDF_AES_CIPH_FLAGS_CTR  0
#define SDF_AES_CIPH_FLAGS_CCM  CUSTOM_FLAGS
#define SDF_AES_CIPH_FLAGS_GCM  EVP_CIPH_FLAG_AEAD_CIPHER | CUSTOM_FLAGS

#define SDF_SM_CIPH_FLAGS_ECB  EVP_CIPH_CUSTOM_COPY
#define SDF_SM_CIPH_FLAGS_CBC  EVP_CIPH_FLAG_CUSTOM_CIPHER | EVP_CIPH_CUSTOM_COPY
#define SDF_SM_CIPH_FLAGS_OFB  EVP_CIPH_CUSTOM_COPY
#define SDF_SM_CIPH_FLAGS_CFB  EVP_CIPH_CUSTOM_COPY
#define SDF_SM_CIPH_FLAGS_CTR  EVP_CIPH_CUSTOM_COPY
#define SDF_SM_CIPH_FLAGS_MAC  EVP_CIPH_CUSTOM_COPY

#define SDF_DES_CIPH_FLAGS_ECB  0
#define SDF_DES_CIPH_FLAGS_CBC  0

int gm_nid_Init(void);
int gm_nid_get(int nid);
int get_algid_by_nid(int nid);
int get_ans1id_by_nid(int nid);

void printHex(char *data, int len);
#if 0
int ssl2sdf_ec_cipher(ECCCipher *ecp, const unsigned char *in, int inlen);
int sdf2ssl_ec_cipher(unsigned char *out, int *outl, const ECCCipher *ecp);

int ssl2sdf_ec_pkey(ECCrefPublicKey *pub, ECCrefPrivateKey *priv, EVP_PKEY *pkey);
int sdf2ssl_ec_pkey(EVP_PKEY *pkey, const ECCrefPublicKey *pub, const ECCrefPrivateKey *priv);

int sdf2ssl_ecsign(unsigned char *sig, int *signl, const ECCSignature *ecc_sign);
int ssl2sdf_ecsign(ECCSignature *ecc_sign, const unsigned char *sig, int signl);
int ec_algor_check(EVP_PKEY_CTX *ctx);

#endif


#ifdef __cplusplus
}
#endif
#endif
#endif
