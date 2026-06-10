#ifndef _SDF_DSO_H_
#define _SDF_DSO_H_

void *loadLibrary(const char * name);
void unloadLibrary(void * handle);
void *findSymbol(void * dll_handle, char * name);

#endif