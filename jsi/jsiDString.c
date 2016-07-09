/* Support for Jsi_DString: Dynamic Strings in C */
/* Can be used either within Jsi or standalone by including jsi.h */

#ifndef JSI_AMALGAMATION
#ifdef JSI_STANDALONE
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

#define DSNotInit(dsPtr) (dsPtr->spaceAvl<=0 || dsPtr->str == NULL)

/* Initialize an uninitialized string. */
static void InitStr(Jsi_DString *dsPtr)
{
    char *str = dsPtr->str;
    dsPtr->len = 0;
    if (dsPtr->staticSize<=0)
        dsPtr->staticSize = JSI_DSTRING_STATIC_SIZE;
    dsPtr->spaceAvl = dsPtr->staticSize;
    dsPtr->staticSpace[0] = 0;
    dsPtr->str = dsPtr->staticSpace;
    if (str)
        Jsi_DSAppendLen(dsPtr, str, -1);
}

/* RETURNS: string value. */
char* Jsi_DSValue(Jsi_DString *dsPtr)  {
    if (DSNotInit(dsPtr))
        InitStr(dsPtr);
    return dsPtr->str;
}

/* RETURNS: string length. */
int Jsi_DSLength(Jsi_DString *dsPtr) {
    return dsPtr->len;
}

/* Initialize string. */
void Jsi_DSInit(Jsi_DString *dsPtr)
{
    dsPtr->staticSize = 0;
    dsPtr->str = NULL;
    InitStr(dsPtr);
}

/* 
 * Frees any allocated space and sets the DString back to empty such that it is safe to exit the scope.
 * Alternatively, the DString may be immediately reused. However in that case a call to Jsi_DSReset might
 * be more appropriate. 
 */
void
Jsi_DSFree(Jsi_DString *dsPtr)
{
    if (!dsPtr->str)
        return;
    if (dsPtr->str != dsPtr->staticSpace && dsPtr->spaceAvl>0)
        Jsi_Free(dsPtr->str);
    dsPtr->str = NULL;
    dsPtr->staticSpace[0] = 0;
    dsPtr->spaceAvl = dsPtr->staticSize;
}

/* 
 * Append length bytes to the string. If length is less than 0,
 * the value of strlen is used.  If required, the DString is realloced to
 * be large enough to contain bytes, plus an extra null byte that is added to the end.
 * RETURNS: The string starting at the first appended character.
*/
char *Jsi_DSAppendLen(Jsi_DString *dsPtr, const char *string, int length)
{
    if (DSNotInit(dsPtr))
        InitStr(dsPtr);        
    int len = dsPtr->len;
    if (string) {
        if (length < 0)
            length = strlen(string);
        int nsiz = length + len+1;
    
        if (nsiz >= dsPtr->spaceAvl) {
            if (nsiz < dsPtr->spaceAvl*2)
                nsiz = dsPtr->spaceAvl*2;
            if (Jsi_DSSetLength(dsPtr, nsiz) < nsiz)
                return Jsi_DSValue(dsPtr);
        }
        char * dst = dsPtr->str + dsPtr->len;
        memcpy(dst, string, length);
        dst[length] = 0;
        dsPtr->len += length;
    }
    return dsPtr->str+len;
}

/* 
 * Calls Jsi_DSAppendLen for each string value argument, passing in -1 for the length.
 * Each string is assumed to be null terminated and the final argument must be a NULL.
 * RETURNS: The string starting at the first appended character.
*/
char *
Jsi_DSAppend(Jsi_DString *dsPtr, const char *str, ...)
{
    va_list argList;
    char *elem;
    if (DSNotInit(dsPtr))
        InitStr(dsPtr);
    int len = dsPtr->len;
    if (!str)
        return dsPtr->str;
    Jsi_DSAppendLen(dsPtr, str, -1);
    va_start(argList, str);
    while ((elem = va_arg(argList, char *)) != NULL) {
        Jsi_DSAppendLen(dsPtr, elem, -1);
    }
    va_end(argList);
    return dsPtr->str+len;
}

/*
 * Format output and append to the end of dsPtr.
 * RETURNS: The string starting at the first appended character.
 */
char *
Jsi_DSPrintf(Jsi_DString *dsPtr, const char *fmt, ...)
{
    va_list argList;
#ifndef JSI_PRINTF_BUFSIZ
#define JSI_PRINTF_BUFSIZ BUFSIZ
#endif
    char buf[JSI_PRINTF_BUFSIZ], *bPtr = buf;
    int n, bsiz = sizeof(buf), needAppend = 1;
    if (DSNotInit(dsPtr))
        InitStr(dsPtr);
    int len = dsPtr->len;
    char *send = dsPtr->str + len;
    uint avail = (dsPtr->spaceAvl - len);
    if (avail >= sizeof(buf)) { /* Format directly into string. */
        bPtr = dsPtr->str+len;
        bsiz = avail;
        needAppend = 0;
    }
    va_start(argList, fmt);
    n = vsnprintf(bPtr, bsiz, fmt, argList);
    if (n<0 || (n+len)>JSI_MAX_ALLOC_BUF) {
        LogError("vsnprintf error: rc=%d, len=%d", n, len);
        va_end(argList);
        return send;
    }
    if (n >= bsiz) {
        int m = len+n+1;
        if (Jsi_DSSetLength(dsPtr, m) < m) {
            va_start(argList, fmt);
            return send;
        }
        m = vsnprintf(dsPtr->str+len, len+1, fmt, argList);
        if (m != n) {
            LogError("len mismatch: %d != %d",  m, n);
            va_end(argList);
            return send;
        }
    } else if (needAppend) {
        Jsi_DSAppendLen(dsPtr, buf, n);
    }
    va_end(argList);
    return send;
}

/* 
 * Set the minimum allocated space and/or the maximum string length. 
 * If length < current dsPtr->len truncates string, else sets minimum allocated space.
 * RETURNS: currently allocated space. 
 */
int
Jsi_DSSetLength(Jsi_DString *dsPtr, int length)
{
    if (DSNotInit(dsPtr))
        InitStr(dsPtr);
    if (length < 0)
        return dsPtr->spaceAvl;
    if (length >= JSI_MAX_ALLOC_BUF) {
        LogError("max alloc exceeded %d", length);
        length = JSI_MAX_ALLOC_BUF-1;
    }

    if (length >= dsPtr->spaceAvl) {
        int isStatic = (dsPtr->staticSpace == dsPtr->str);

        dsPtr->spaceAvl = length;
        if (isStatic == 0 || length >= dsPtr->staticSize) {
            char *newString = (char *) Jsi_Realloc((isStatic?NULL:dsPtr->str), (unsigned) (dsPtr->spaceAvl+1));
            if (!newString) {
                LogError("malloc failed %d", dsPtr->spaceAvl+1);
                return -1;
            }
            dsPtr->str = newString;
            if (isStatic && dsPtr->len>0)
                memcpy(dsPtr->str, dsPtr->staticSpace, (size_t) (dsPtr->len+1));
        }
    }
    if (length < dsPtr->len) {
        dsPtr->str[length] = 0;
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
    return Jsi_DSValue(dsPtr);
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
    if (DSNotInit(dsPtr))
        InitStr(dsPtr);
    if (dsPtr->staticSpace == dsPtr->str) {
        cp = Jsi_Strdup(dsPtr->str);
        Jsi_DSSetLength(dsPtr, 0);
        return cp;
    }
    cp = dsPtr->str;
    dsPtr->str = dsPtr->staticSpace;
    dsPtr->staticSpace[0] = 0;
    dsPtr->spaceAvl = dsPtr->staticSize;
    dsPtr->len = 0;
    dsPtr->str = NULL;
    return cp;
}

#ifdef JSI_DSTRING_SELFTEST

void Jsi_DString_SelfTest() {
    {
        Jsi_DString d1 = {}, d2 = {"Here is"};
        Jsi_DSAppend(&d2 ," your score: ", NULL);
        Jsi_DSPrintf(&d2, " -> %d/%d", 99, 100);
        char *cp = Jsi_DSValue(&d2);
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
