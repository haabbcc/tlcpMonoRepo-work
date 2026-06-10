#ifndef _GM_CIPHER_H_
#define _GM_CIPHER_H_


#ifdef  __cplusplus
extern "C" {
#endif     /* __cplusplus */


//调用时将openssl/engine.h头文件需放在gm_cipher.h前，否则会有宏重定义
#ifndef ENGINE_CMD_BASE
#define ENGINE_CMD_BASE   200
#endif

//设置算法优先级，优先回调 or 优先软算法库
#define GM_CMD_SET_ALGOR_PRIORITY       (ENGINE_CMD_BASE + 1)
//设置软算法类型  1--openssl算法库 其它--引擎自带软算法库
#define GM_CMD_SET_SOFT_ALGOR           (ENGINE_CMD_BASE + 2)

//传递给回调库
//应用名称 
#define GM_CMD_SET_APP_NAME             (ENGINE_CMD_BASE + 3)
#define GM_CMD_GET_APP_NAME             (ENGINE_CMD_BASE + 4)
//容器名称
#define GM_CMD_SET_CONTAINER            (ENGINE_CMD_BASE + 5)
#define GM_CMD_GET_CONTAINER            (ENGINE_CMD_BASE + 6)
//私钥访问控制码
#define GM_CMD_SET_USER_PIN             (ENGINE_CMD_BASE + 7)
#define GM_CMD_GET_USER_PIN             (ENGINE_CMD_BASE + 8)
//SM2密钥类型，加密/签名密钥
#define GM_CMD_SET_KEY_TYPE             (ENGINE_CMD_BASE + 9)
#define GM_CMD_GET_KEY_TYPE             (ENGINE_CMD_BASE + 10)
//密钥索引
#define GM_CMD_SET_KEY_INDEX            (ENGINE_CMD_BASE + 11)
#define GM_CMD_GET_KEY_INDEX            (ENGINE_CMD_BASE + 12)

//回调库路径
#define GM_CMD_SET_SO_PATH              (ENGINE_CMD_BASE + 13)
//删除回调
#define GM_CMD_DEL_CALLBACK             (ENGINE_CMD_BASE + 15)


#define GM_ECC_MAX_BITS                 512 
#define GM_ECC_MAX_LEN                  ((GM_ECC_MAX_BITS+7) / 8)
#define GM_ECC_MAX_CIPHER_LEN           136

//ECC 公钥结构
typedef struct gm_ecc_publickey_st {
	unsigned int  bits;                      //公钥位数
	unsigned char x[GM_ECC_MAX_LEN];         //公钥x分量
	unsigned char y[GM_ECC_MAX_LEN];         //公钥y分量
}GM_ECCrefPublicKey;
//ECC 密文结构
typedef struct gm_ecc_cipher_st {
	unsigned char x[GM_ECC_MAX_LEN];         //公钥x分量
	unsigned char y[GM_ECC_MAX_LEN];         //公钥y分量
	unsigned char M[32];                     //明文的杂凑值
	unsigned int  L;                         //密文数据长度
	unsigned char C[GM_ECC_MAX_CIPHER_LEN];  //密文数据
}GM_ECCCipher;
/*ECC 签名结构*/
typedef struct gm_ecc_signature_st
{
	unsigned char r[GM_ECC_MAX_LEN];         //签名的r部分
	unsigned char s[GM_ECC_MAX_LEN];         //签名的s部分
} GM_ECCSignature;

//回调接口。args参数为应用层传给回调接口参数，具体结构由应用层与回调库协商
//返回值：0--成功，非0--失败
typedef struct gm_callback_method_st {
	const char *id;                          //回调库id
	int(*create)(void *args);                //引擎初始化，加载引擎时(ENGINE_load_gm)调用
	int(*destory)(void);                     //引擎销毁
	int(*init)(void);                        //引擎初始化，与finish配套使用
	int(*finish)(void);                      //引擎使用完毕
	int(*sign)(GM_ECCSignature *signautre, const unsigned char *msg, 
		unsigned int msglen, void *args);    //SM2签名
	int(*decrypt)(unsigned char *plain, unsigned int *plainlen, 
		GM_ECCCipher *cipher, void *args);   //SM2私钥解密
	int(*random)(unsigned char *buf, unsigned int num);                 //生成随机数
	int(*load_pubkey)(GM_ECCrefPublicKey *pubkey, void *args);          //导出公钥
	int(*ctrl)(int cmd, long i, void *p, void(*f) (void));              //控制接口，用于传参给回调库
}GM_CALLBACK_METHOD;


//加载引擎接口，加载后EC、SM2算法默认走引擎
//返回值：1--成功， -2--引擎库已加载， 其它--失败
int ENGINE_load_gm(const GM_CALLBACK_METHOD *cb, void *args);


void ERR_load_GM_strings(void);
void ERR_unload_GM_strings(void);

//库版本信息
const char *ENGINE_GM_version(void);
unsigned long ENGINE_GM_version_num(void);


#ifdef  __cplusplus
}
#endif      /* __cplusplus */
#endif     /* !_GM_CIPHER_H_ */
