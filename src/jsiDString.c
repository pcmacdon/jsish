/* Support for Jsi_DString: Dynamic Strings in C */
/* Can be used either within Jsi or standalone by including jsi.h */

#ifndef JSI_AMALGAMATION
#ifndef JSI_STANDALONE
#include "jsiInt.h"
#else
/* Not being used within JSI */
#include "jsi.h"
#ifndef JSI_MAX_ALLOC_BUF
#define JSI_MAX_ALLOC_BUF 100000000
#endif
#define Jsi_Realloc realloc
#define Jsi_Free free
#define Jsi_Strdup strdup
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#endif /* JSI_STANDALONE */
#endif /* !JSI_AMALGAMATION */

#ifndef LogError
#define LogError(fmt,...) fprintf(stderr, fmt "%s %s:%d\n", __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#endif

#define jsi_DSValue(dsPtr) (char*)(dsPtr->strA?dsPtr->strA:dsPtr->Str)

/* Initialization for a string that was declared with "= {}". */
#define jsi_DsCheckInit(dsPtr) if  (dsPtr->staticSize==0) jsi_DsDoInit(dsPtr);

static void jsi_DsDoInit(Jsi_DString *dsPtr) {
#ifdef JSI_MEM_DEBUG
    assert(dsPtr->len<JSI_MAX_ALLOC_BUF && dsPtr->spaceAvl<JSI_MAX_ALLOC_BUF );
#endif
    dsPtr->spaceAvl = dsPtr->staticSize = sizeof(dsPtr->Str);
    if (dsPtr->strA) {
        if ((dsPtr->len = Jsi_Strlen(dsPtr->strA))<dsPtr->spaceAvl) {
            Jsi_Strcpy(dsPtr->Str, dsPtr->strA);
            dsPtr->strA = NULL;
        } else
            dsPtr->strA = Jsi_Strdup(dsPtr->strA);
    } else 
        dsPtr->len = Jsi_Strlen(dsPtr->Str);
}

/* RETURNS: string value. */
char* Jsi_DSValue(Jsi_DString *dsPtr)  {
    jsi_DsCheckInit(dsPtr);
    return jsi_DSValue(dsPtr);
}

/* RETURNS: string length. */
uint Jsi_DSLength(Jsi_DString *dsPtr) {
    jsi_DsCheckInit(dsPtr);
    return dsPtr->len;
}

/* Initialize string. */
void Jsi_DSInit(Jsi_DString *dsPtr)
{
    dsPtr->spaceAvl = dsPtr->staticSize = sizeof(dsPtr->Str);
    dsPtr->Str[0] = 0;
    dsPtr->strA = NULL;
    dsPtr->len = 0;
}

/* 
 * Frees any allocated space and sets the DString back to empty such that it is safe to exit the scope
 * or the DString may be reused.
 */
void
Jsi_DSFree(Jsi_DString *dsPtr)
{
    if (dsPtr->spaceAvl == 0) return;
    jsi_DsCheckInit(dsPtr);
    if (dsPtr->strA)
        Jsi_Free((char*)dsPtr->strA);
    dsPtr->strA = NULL;
    dsPtr->Str[0] = 0;
    dsPtr->len = 0;
    if (dsPtr->staticSize<=0)
        dsPtr->staticSize = sizeof(dsPtr->Str);
    dsPtr->spaceAvl = dsPtr->staticSize;
}

/* 
 * Append length bytes to the string. If length is less than 0,
 * the value of Jsi_Strlen is used.  If required, the DString is realloced to
 * be large enough to contain bytes, plus an extra null byte that is added to the end.
 * RETURNS: The the current string.
*/
char *Jsi_DSAppendLen(Jsi_DString *dsPtr, const char *string, int length)
{
    jsi_DsCheckInit(dsPtr)
    int len = dsPtr->len;
    if (string) {
        if (length < 0)
            length = Jsi_Strlen(string);
        uint nsiz = length + len+1;
    
        if (nsiz >= dsPtr->spaceAvl) {
            if (nsiz < dsPtr->spaceAvl*2)
                nsiz = dsPtr->spaceAvl*2;
            if (Jsi_DSSetLength(dsPtr, nsiz) < nsiz)
                return jsi_DSValue(dsPtr);
        }
        char * dst = jsi_DSValue(dsPtr) + dsPtr->len;
        memcpy(dst, string, length);
        dst[length] = 0;
        dsPtr->len += length;
    }
    return jsi_DSValue(dsPtr);
}

/* 
 * Calls Jsi_DSAppendLen for each string value argument, passing in -1 for the length.
 * Each string is assumed to be null terminated and the final argument must be a NULL.
 * RETURNS: The the current string.
*/
char *
Jsi_DSAppend(Jsi_DString *dsPtr, const char *str, ...)
{
    va_list argList;
    char *elem;
    jsi_DsCheckInit(dsPtr)
    if (!str)
        return jsi_DSValue(dsPtr);
    Jsi_DSAppendLen(dsPtr, str, -1);
    va_start(argList, str);
    while ((elem = va_arg(argList, char *)) != NULL) {
        Jsi_DSAppendLen(dsPtr, elem, -1);
    }
    va_end(argList);
    return jsi_DSValue(dsPtr);
}

/*
 * Format output and append to the end of dsPtr.
 * RETURNS: The current string.
 */
char *
Jsi_DSPrintf(Jsi_DString *dsPtr, const char *fmt, ...)
{
    va_list argList;
#ifndef JSI_PRINTF_BUFSIZ
#define JSI_PRINTF_BUFSIZ JSI_BUFSIZ
#endif
    char buf[JSI_PRINTF_BUFSIZ], *bPtr = buf;
    uint n, bsiz = sizeof(buf), needAppend = 1;
    jsi_DsCheckInit(dsPtr)
    int len = dsPtr->len;
    uint avail = (dsPtr->spaceAvl - len);
    char *dstr = jsi_DSValue(dsPtr);
    if (avail >= sizeof(buf)) { /* Format directly into string. */
        bPtr = dstr+len;
        bsiz = avail;
        needAppend = 0;
    }
    va_start(argList, fmt);
    n = vsnprintf(bPtr, bsiz, fmt, argList);
    if ((n+len)>JSI_MAX_ALLOC_BUF) {
        LogError("vsnprintf error: rc=%d, len=%d", n, len);
        va_end(argList);
        return dstr;
    }
    if (n >= bsiz) {
        uint m = len+n+1;
        if (Jsi_DSSetLength(dsPtr, m) < m) {
            va_end(argList);
            return jsi_DSValue(dsPtr);
        }
        dstr = jsi_DSValue(dsPtr);
        m = vsnprintf(dstr+len, len+1, fmt, argList);
        if (m != n) {
            LogError("len mismatch: %d != %d",  m, n);
            va_end(argList);
            return dstr;
        }
    } else if (needAppend) {
        Jsi_DSAppendLen(dsPtr, buf, n);
    }
    va_end(argList);
    return jsi_DSValue(dsPtr);
}

/* 
 * Set the minimum allocated space and/or the maximum string length.
 * If length < current dsPtr->len truncates string, else sets minimum allocated space.
 * RETURNS: currently allocated space. 
 */
uint Jsi_DSSetLength(Jsi_DString *dsPtr, uint length)
{
    jsi_DsCheckInit(dsPtr)
    if (length >= JSI_MAX_ALLOC_BUF) {
        LogError("max alloc exceeded %u", length);
        length = JSI_MAX_ALLOC_BUF-1;
    }
    char *dstr = jsi_DSValue(dsPtr);
    if (length >= dsPtr->spaceAvl) {
        int isStatic = (dsPtr->strA == NULL);

        dsPtr->spaceAvl = length;
        if (isStatic == 0 || length >= dsPtr->staticSize) {
            char *newString = (char *) Jsi_Realloc((isStatic?NULL:(char*)dsPtr->strA), (unsigned) (dsPtr->spaceAvl+1));
            if (!newString) {
                LogError("malloc failed %d", dsPtr->spaceAvl+1);
                return -1;
            }
            if (isStatic && dsPtr->len>0)
                memcpy(newString, dsPtr->Str, (size_t) (dsPtr->len+1));
            dsPtr->strA = newString;
        }
    }
    if (length < dsPtr->len) {
        dstr = jsi_DSValue(dsPtr);
        dstr[length] = 0;
        dsPtr->len = length;
    }
    return dsPtr->spaceAvl;
}

char *
Jsi_DSSet(Jsi_DString *dsPtr, const char *str)
{
    Jsi_DSSetLength(dsPtr, 0);
    if (str)
        Jsi_DSAppendLen(dsPtr, str, -1);
    return jsi_DSValue(dsPtr);
}

/*
 * Returns the strdup of the string value and resets the DString in the same way as Jsi_DSFree.
 * This just avoids the user having to do an extra malloc/free when the DString was already malloced.
 * It is the responsibility of the caller to free the returned value.
 * RETURNS: previous string value malloced.
 */
char*  Jsi_DSFreeDup(Jsi_DString *dsPtr)
{
    char *cp;
    jsi_DsCheckInit(dsPtr)
    if (!dsPtr->strA) {
        cp = Jsi_StrdupLen(dsPtr->Str, dsPtr->len);
        Jsi_DSSetLength(dsPtr, 0);
        return cp;
    }
    cp = (char*)dsPtr->strA;
    dsPtr->strA = NULL;
    dsPtr->Str[0] = 0;
    dsPtr->spaceAvl = dsPtr->staticSize;
    dsPtr->len = 0;
    return cp;
}

#ifdef JSI_DSTRING_SELFTEST

void Jsi_DString_SelfTest() {
    {
        Jsi_DString d1 = {}, d2 = {"Here is"};
        Jsi_DSAppend(&d2 ," your score: ", NULL);
        Jsi_DSPrintf(&d2, " -> %d/%d", 99, 100);
        char *cp = jsi_DSValue(&d2);
        puts(cp);     // "Here is your score: -> 99/100"
        Jsi_DSAppend(&d1, cp, NULL);
        Jsi_DSFree(&d1);  Jsi_DSFree(&d2);
    }
    {
        Jsi_DString d = {};;
        Jsi_DSPrintf(&d , "%0300d", 1); // Malloc
        Jsi_DSSetLength(&d, 0);
        Jsi_DSPrintf(&d , "%0300d", 1); // No-malloc
        Jsi_DSFree(&d);
        Jsi_DSPrintf(&d , "%0300d", 1); // Malloc
        Jsi_DSFree(&d);
    }
    {
        Jsi_DString d;
        Jsi_DSInit(&d);
        Jsi_DSAppend(&d , " some stuff: ", NULL);
        Jsi_DSFree(&d);
    }
    {
        JSI_DSTRING_VAR(dPtr,301);
        Jsi_DSPrintf(dPtr , "%0300d", 1); // No-malloc
        Jsi_DSSetLength(dPtr, 0);
        Jsi_DSPrintf(dPtr , "%0300d", 1); // No-malloc
        Jsi_DSFree(dPtr);
        Jsi_DSPrintf(dPtr , "%0400d", 1); // Malloc
        Jsi_DSFree(dPtr);
    }
}
#endif
