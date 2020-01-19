#ifndef __DEMO_H__
#define __DEMO_H__
#include "jsi.h"
#define DEMO_VERSION 1.0
JSI_EXTERN int Demo_Incr(int n); /*STUB = 1*/
JSI_EXTERN int Demo_Decr(int n); /*STUB = 2*/

typedef struct {
    int a; int b;
} DemoStruct;

#define DEMO_STUBS_STRUCTSIZES (sizeof(DemoStruct))
#ifdef DEMO_USE_STUBS
#include "demoStubs.h"
#else
#define DEMO_EXTENSION_INI
#define Demo_StubsInit(i,f) JSI_OK
#endif

#endif

