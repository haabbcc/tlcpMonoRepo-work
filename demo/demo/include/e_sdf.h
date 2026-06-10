#ifndef _SDF_CIPHER_H_
#define _SDF_CIPHER_H_

#include <stdio.h>
#ifdef  __cplusplus
extern "C" {
#endif     /* __cplusplus */


#define DCIRDLL_EXPORTS
#if defined(DCIRDLL_EXPORTS)
#if defined(__linux) || defined(__linux__) || defined(linux)
#define ENG_EXPORT __attribute__((visibility ("default")))  //Linux动态库(.so)
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) 
#define ENG_EXPORT  __declspec(dllexport)
#else
#define ENG_EXPORT
#endif
#else
#define ENG_EXPORT
#endif

ENG_EXPORT void ENGINE_load_sdf(void);//初始化

ENG_EXPORT const char* ENGINE_get_version(void);//获取版本

#ifdef  __cplusplus
}
#endif      /* __cplusplus */

#endif     /* !_SDF_CIPHER_H_ */
