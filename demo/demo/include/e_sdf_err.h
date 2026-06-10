#ifndef _SDF_CIPHER_ERR_H_
#define _SDF_CIPHER_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int err;
	unsigned long reason;
} SDF_ERR_REASON;

//static void ERR_load_SDF_strings(void);
//static void ERR_unload_SDF_strings(void);
//static void ERR_SDF_error(int function, int reason, char *file, int line);
#define SDFerr(f,r) ERR_SDF_error((f),(r),__FILE__,__LINE__)
/* Error codes for the SDF functions. */


/* Function codes. */
# define SDF_F_SANSEC_DECODE_ECCCIPHER                    149
# define SDF_F_SANSEC_ENCODE_ECCCIPHER                    150
# define SDF_F_SDF_CALCULATEMAC                           100
# define SDF_F_SDF_CLOSEDEVICE                            101
# define SDF_F_SDF_CLOSESESSION                           102
# define SDF_F_SDF_CREATEFILE                             103
# define SDF_F_SDF_DECRYPT                                104
# define SDF_F_SDF_DELETEFILE                             105
# define SDF_F_SDF_DESTROYKEY                             106
# define SDF_F_SDF_ENCRYPT                                107
# define SDF_F_SDF_EXCHANGEDIGITENVELOPEBASEONECC         108
# define SDF_F_SDF_EXCHANGEDIGITENVELOPEBASEONRSA         109
# define SDF_F_SDF_EXPORTENCPUBLICKEY_ECC                 110
# define SDF_F_SDF_EXPORTENCPUBLICKEY_RSA                 111
# define SDF_F_SDF_EXPORTSIGNPUBLICKEY_ECC                112
# define SDF_F_SDF_EXPORTSIGNPUBLICKEY_RSA                113
# define SDF_F_SDF_EXTERNALENCRYPT_ECC                    114
# define SDF_F_SDF_EXTERNALPRIVATEKEYOPERATION_RSA        115
# define SDF_F_SDF_EXTERNALPUBLICKEYOPERATION_RSA         116
# define SDF_F_SDF_EXTERNALVERIFY_ECC                     117
# define SDF_F_SDF_GENERATEAGREEMENTDATAANDKEYWITHECC     118
# define SDF_F_SDF_GENERATEAGREEMENTDATAWITHECC           119
# define SDF_F_SDF_GENERATEKEYPAIR_ECC                    120
# define SDF_F_SDF_GENERATEKEYPAIR_RSA                    121
# define SDF_F_SDF_GENERATEKEYWITHECC                     122
# define SDF_F_SDF_GENERATEKEYWITHEPK_ECC                 123
# define SDF_F_SDF_GENERATEKEYWITHEPK_RSA                 124
# define SDF_F_SDF_GENERATEKEYWITHIPK_ECC                 125
# define SDF_F_SDF_GENERATEKEYWITHIPK_RSA                 126
# define SDF_F_SDF_GENERATEKEYWITHKEK                     127
# define SDF_F_SDF_GENERATERANDOM                         128
# define SDF_F_SDF_GETDEVICEINFO                          129
# define SDF_F_SDF_GETPRIVATEKEYACCESSRIGHT               130
# define SDF_F_SDF_HASHFINAL                              131
# define SDF_F_SDF_HASHINIT                               132
# define SDF_F_SDF_HASHUPDATE                             133
# define SDF_F_SDF_IMPORTKEY                              134
# define SDF_F_SDF_IMPORTKEYWITHISK_ECC                   135
# define SDF_F_SDF_IMPORTKEYWITHISK_RSA                   136
# define SDF_F_SDF_IMPORTKEYWITHKEK                       137
# define SDF_F_SDF_INTERNALDECRYPT_ECC                    151
# define SDF_F_SDF_INTERNALENCRYPT_ECC                    152
# define SDF_F_SDF_INTERNALPRIVATEKEYOPERATION_RSA        138
# define SDF_F_SDF_INTERNALPUBLICKEYOPERATION_RSA         147
# define SDF_F_SDF_INTERNALSIGN_ECC                       139
# define SDF_F_SDF_INTERNALVERIFY_ECC                     140
# define SDF_F_SDF_LOADLIBRARY                            148
# define SDF_F_SDF_METHOD_LOAD_LIBRARY                    141
# define SDF_F_SDF_NEWECCCIPHER                           153
# define SDF_F_SDF_OPENDEVICE                             142
# define SDF_F_SDF_OPENSESSION                            143
# define SDF_F_SDF_READFILE                               144
# define SDF_F_SDF_RELEASEPRIVATEKEYACCESSRIGHT           145
# define SDF_F_SDF_WRITEFILE                              146
# define SDF_F_SDF_ERROR                                  151

# define SDF_F_SDF_EXTERNALDECRYPT_ECC                    160
# define SDF_F_SDF_EXTERNALSIGN_ECC                       161

# define SDF_F_EGN_SM2_VERIFY                             200
# define SDF_F_EGN_SM2_SIGN                               201
# define SDF_F_EGN_SM2_DECRYPT                            202
# define SDF_F_EGN_SM2_ENCRYPT                            203
# define SDF_F_EGN_RANDON_BYTES                           204
# define SDF_F_EGN_CIPHER_CTRL                            205
# define SDF_F_EGN_CIPHER_INIT                            206
# define SDF_F_EGN_CIPHER_CODE                            207
# define SDF_F_EGN_DIGEST_INIT                            208
# define SDF_F_EGN_DIGEST_UPDATE                          209
# define SDF_F_EGN_DIGEST_FINAL                           210
# define SDF_F_EGN_SM2_INIT                               211
# define SDF_F_EGN_SM2_COPY                               212
//# define SDF_F_EGN_SM2_SIGN                               213

/* Reason codes. */
# define SDF_R_ALGORITHM_MODE_NOT_SUPPORTED               111
# define SDF_R_ALGORITHM_NOT_SUPPORTED                    112
# define SDF_R_BUFFER_TOO_SMALL                           113
# define SDF_R_COMMUNICATION_FAILURE                      114
# define SDF_R_DSO_LOAD_FAILURE                           110
# define SDF_R_ENCRYPT_DATA_ERROR                         115
# define SDF_R_ERROR                                      116
# define SDF_R_FILE_ALREADY_EXIST                         117
# define SDF_R_FILE_NOT_EXIST                             118
# define SDF_R_HARDWARE_ERROR                             119
# define SDF_R_INVALID_CIPHER_ALGOR                       143
# define SDF_R_INVALID_DIGEST_ALGOR                       144
# define SDF_R_INVALID_FILE_OFFSET                        120
# define SDF_R_INVALID_FILE_SIZE                          121
# define SDF_R_INVALID_INPUT_ARGUMENT                     122
# define SDF_R_INVALID_KEY                                123
# define SDF_R_INVALID_KEY_LENGTH                         100
# define SDF_R_INVALID_KEY_TYPE                           124
# define SDF_R_INVALID_OUTPUT_ARGUMENT                    125
# define SDF_R_INVALID_SANSEC_ECCCIPHER_LENGTH            207
# define SDF_R_INVALID_SDF_LIBRARY                        101
# define SDF_R_INVALID_SESSION_HANDLE                     102
# define SDF_R_INVALID_SM2_CIPHERTEXT_LENGTH              212
# define SDF_R_KEY_NOT_EXIST                              126
# define SDF_R_LOAD_LIBRARY_FAILURE                       107
# define SDF_R_MAC_ERROR                                  127
# define SDF_R_METHOD_OPERATION_FAILURE                   108
# define SDF_R_MULTI_STEP_OPERATION_ERROR                 128
# define SDF_R_NOT_INITIALIZED                            109
# define SDF_R_NOT_SUPPORTED                              103
# define SDF_R_NOT_SUPPORTED_CIPHER_ALGOR                 208
# define SDF_R_NOT_SUPPORTED_DIGEST_ALGOR                 209
# define SDF_R_NOT_SUPPORTED_ECC_ALGOR                    210
# define SDF_R_NOT_SUPPORTED_PKEY_ALGOR                   211
# define SDF_R_NO_PRIVATE_KEY_ACCESS_RIGHT                129
# define SDF_R_OPEN_DEVICE_FAILURE                        130
# define SDF_R_OPEN_SESSION_FAILURE                       131
# define SDF_R_OPERATION_FAILED                           104
# define SDF_R_OPERATION_NOT_SUPPORTED                    132
# define SDF_R_PRIVATE_KEY_OPERATION_FAILURE              133
# define SDF_R_PRKERR                                     134
# define SDF_R_PUBLIC_KEY_OPERATION_FAILURE               135
# define SDF_R_RANDOM_GENERATION_ERROR                    136

# define SDF_R_INVALID_ENCODING                           300
# define SDF_R_MALLOC_FAILURE                             301
# define SDF_R_ENCODING_FAILURE                           302
# define SDF_R_ASN1_ERROR                                 303
#  ifdef  __cplusplus
}
#  endif

#endif