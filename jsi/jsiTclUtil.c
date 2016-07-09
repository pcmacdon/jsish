#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#include "jsiUtf8.h"
#endif
#include <errno.h>

static const char *JsiCharsetMatch(const char *pattern, int c, int flags)
{
    int inot = 0;
    int pchar;
    int match = 0;
    int nocase = 0;

    if (flags & JSI_CMP_NOCASE) {
        nocase++;
        c = jsi_utf8_upper(c);
    }

    if (flags & JSI_CMP_CHARSET_SCAN) {
        if (*pattern == '^') {
            inot++;
            pattern++;
        }

        /* Special case. If the first char is ']', it is part of the set */
        if (*pattern == ']') {
            goto first;
        }
    }

    while (*pattern && *pattern != ']') {
        /* Exact match */
        if (pattern[0] == '\\') {
first:
            pattern += jsi_utf8_tounicode_case(pattern, &pchar, nocase);
        }
        else {
            /* Is this a range? a-z */
            int start;
            int end;
            pattern += jsi_utf8_tounicode_case(pattern, &start, nocase);
            if (pattern[0] == '-' && pattern[1]) {
                /* skip '-' */
                pattern += jsi_utf8_tounicode(pattern, &pchar);
                pattern += jsi_utf8_tounicode_case(pattern, &end, nocase);

                /* Handle reversed range too */
                if ((c >= start && c <= end) || (c >= end && c <= start)) {
                    match = 1;
                }
                continue;
            }
            pchar = start;
        }

        if (pchar == c) {
            match = 1;
        }
    }
    if (inot) {
        match = !match;
    }

    return match ? pattern : NULL;
}


char *Jsi_Itoa(int n)
{
    static char buf[100];
    sprintf(buf, "%d", n);
    return buf;
}
/* Split on char, or whitespace if ch==0. */
static void SplitChar(const char *str, int *argcPtr,
              char ***argvPtr, char ch, Jsi_DString *dStr)
{
    char *cp, *ep, *p, **argv;
    int cnt = 1, len, i;

    len = strlen(str);
    cp = (char*)str;
    while (*cp) {
        if (ch)
            cp = strchr(cp,ch);
        else {
            while (*cp && !isspace(*cp))
                cp++;
        }
        if (cp == NULL || *cp == 0) break;
        cp++;
        cnt++;
    }
    //argv = (char**)Jsi_Calloc(1,(sizeof(char*)*(cnt+3) + sizeof(char)*(len+6)));
    Jsi_DSSetLength(dStr, (sizeof(char*)*(cnt+3) + sizeof(char)*(len+6)));
    argv = (char**)Jsi_DSValue(dStr);
    *argvPtr = argv;
    *argcPtr = cnt;
    p = (char*)&(argv[cnt+2]);
    argv[cnt+1] = p;
    Jsi_Strcpy(p, str);
    cp = p;
    i = 0;
    argv[i++] = p;
    while (*cp) {
        if (ch)
            ep = strchr(cp,ch);
        else {
            ep = cp;
            while (*ep && !isspace(*ep))
                ep++;
        }
        if (ep == NULL || *ep == 0) break;
        *ep = 0;
        cp = ep+1;
        argv[i++] = cp;
    }
    argv[cnt] = NULL;
}

int
Jsi_GetIndex( Jsi_Interp *interp, char *str,
    const char **tablePtr, const char *msg, int flags,
    int *indexPtr)
{
  const char *msg2 = "unknown ";
  char **cp, *c;
  int cond, index = -1, slen, i, dup = 0;
  int exact = (flags & JSI_CMP_EXACT);
  int nocase = (flags & JSI_CMP_NOCASE);
  slen = strlen(str);
 /* if (slen==0) {
    Jsi_LogError("empty option %s %s", msg, str);
    return JSI_ERROR;
  }*/
  cp = (char**)tablePtr;
  i = -1;
  while (*cp != 0) {
    i++;
    c = *cp;
    if (c[0] != str[0]) { cp++; continue; }
    if (!nocase)
        cond = (exact ? Jsi_Strcmp(c,str) : Jsi_Strncmp(c,str,slen));
    else {
        cond = (exact ? Jsi_Strncasecmp(c,str, -1) : Jsi_Strncasecmp(c,str,slen));
    }
    if (cond == 0) {
      if (index<0) {
        index = i;
      } else {
        dup = 1;
        break;
      }
    }
    cp++;
  }
  if (index >= 0 && dup == 0) {
    *indexPtr = index;
    return JSI_OK;
  }
  if (exact && (dup || index<=0)) {
    if (interp != NULL) {
      msg2 = (index>=0? "unknown ":"duplicate ");
    }
    goto err;
  }
  cp = (char**)tablePtr;
  i = -1;
  dup = 0;
  index = -1;
  while (*cp != 0) {
    i++;
    c = *cp;
    if (c[0] == str[0] && Jsi_Strncmp(c,str, slen) == 0) {
      if (index<0) {
        index = i;
        if (slen == (int)Jsi_Strlen(c))
            break;
      } else {
        if (interp != NULL) {
          msg2 = "ambiguous ";
        }
        goto err;
      }
    }
    cp++;
  }
  if (index >= 0 && dup == 0) {
    *indexPtr = index;
    return JSI_OK;
  }
err:
  if (interp != NULL) {
    Jsi_DString dStr = {};
    Jsi_DSAppend(&dStr, msg2, msg, " \"", str, "\" not one of: ", NULL);
    cp = (char**)tablePtr;
    while (*cp != 0) {
      c = *cp;
      Jsi_DSAppend(&dStr, c, NULL);
      Jsi_DSAppend(&dStr, " ", NULL);
      cp++;
    }
    Jsi_LogError("%s", Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
  }
  return JSI_ERROR;
}

int Jsi_GlobMatch(const char *pattern, const char *string, int nocase)
{
    int c;
    int pchar;
    while (*pattern) {
        switch (pattern[0]) {
            case '*':
                while (pattern[1] == '*') {
                    pattern++;
                }
                pattern++;
                if (!pattern[0]) {
                    return 1;   /* match */
                }
                while (*string) {
                    if (Jsi_GlobMatch(pattern, string, nocase))
                        return 1;       /* match */
                    string += jsi_utf8_tounicode(string, &c);
                }
                return 0;       /* no match */

            case '?':
                string += jsi_utf8_tounicode(string, &c);
                break;

            case '[': {
                    string += jsi_utf8_tounicode(string, &c);
                    pattern = JsiCharsetMatch(pattern + 1, c, nocase ? JSI_CMP_NOCASE : 0);
                    if (!pattern) {
                        return 0;
                    }
                    if (!*pattern) {
                        /* Ran out of pattern (no ']') */
                        continue;
                    }
                    break;
                }
            case '\\':
                if (pattern[1]) {
                    pattern++;
                }
                /* fall through */
            default:
                string += jsi_utf8_tounicode_case(string, &c, nocase);
                jsi_utf8_tounicode_case(pattern, &pchar, nocase);
                if (pchar != c) {
                    return 0;
                }
                break;
        }
        pattern += jsi_utf8_tounicode_case(pattern, &pchar, nocase);
        if (!*string) {
            while (*pattern == '*') {
                pattern++;
            }
            break;
        }
    }
    if (!*pattern && !*string) {
        return 1;
    }
    return 0;
}

Jsi_Stack* Jsi_StackNew(void)
{
    Jsi_Stack *stack = (Jsi_Stack*)Jsi_Calloc(1, sizeof(Jsi_Stack));
    return stack;
}

void Jsi_StackFree(Jsi_Stack *stack)
{
    Jsi_Free(stack->vector);
    Jsi_Free(stack);
}

int Jsi_StackLen(Jsi_Stack *stack)
{
    return stack->len;
}

void Jsi_StackPush(Jsi_Stack *stack, void *element)
{
    int neededLen = stack->len + 1;

    if (neededLen > stack->maxlen) {
        stack->maxlen = neededLen < 20 ? 20 : neededLen * 2;
        stack->vector = (void**)Jsi_Realloc(stack->vector, sizeof(void *) * stack->maxlen);
    }
    stack->vector[stack->len] = element;
    stack->len++;
}

void *Jsi_StackPop(Jsi_Stack *stack)
{
    if (stack->len == 0)
        return NULL;
    stack->len--;
    return stack->vector[stack->len];
}

void *Jsi_StackPeek(Jsi_Stack *stack)
{
    if (stack->len == 0)
        return NULL;
    return stack->vector[stack->len - 1];
}

void Jsi_StackFreeElements(Jsi_Interp *interp, Jsi_Stack *stack, Jsi_DeleteProc *freeProc)
{
    int i;
    for (i = 0; i < stack->len; i++)
        freeProc(interp, stack->vector[i]);
    stack->len = 0;
}

typedef struct {
    void *data;
    Jsi_DeleteProc *delProc;
} AssocData;

/* Split on string. */
void Jsi_SplitStr(const char *str, int *argcPtr,
              char ***argvPtr, const char *ch, Jsi_DString *dStr)
{
    char *cp, *ep, *p, **argv;
    int cnt = 1, len, i, clen;
    if (!ch)
        ch = "";
    clen = strlen(ch);
    if (clen<=0)
        return SplitChar(str, argcPtr, argvPtr, *ch, dStr);
    len = strlen(str);
    cp = (char*)str;
    while (*cp) {
        cp = strstr(cp,ch);
 
        if (cp == NULL || *cp == 0) break;
        cp += clen;
        cnt++;
    }
    //argv = (char**)Jsi_Calloc(1,(sizeof(char*)*(cnt+3) + sizeof(char)*(len+6)));
    Jsi_DSSetLength(dStr, (sizeof(char*)*(cnt+3) + sizeof(char)*(len+6)));
    argv = (char**)Jsi_DSValue(dStr);
    *argvPtr = argv;
    *argcPtr = cnt;
    p = (char*)&(argv[cnt+2]);
    argv[cnt+1] = p;
    Jsi_Strcpy(p, str);
    cp = p;
    i = 0;
    argv[i++] = p;
    while (*cp) {
        ep = strstr(cp,ch);
        if (ep == NULL || *ep == 0) break;
        *ep = 0;
        cp = ep+clen;
        argv[i++] = cp;
    }
    argv[cnt] = NULL;
}

static int JsiCheckConversion(const char *str, const char *endptr)
{
    if (str[0] == '\0' || str == endptr) {
        return JSI_ERROR;
    }

    if (endptr[0] != '\0') {
        while (*endptr) {
            if (!isspace(UCHAR(*endptr))) {
                return JSI_ERROR;
            }
            endptr++;
        }
    }
    return JSI_OK;
}

static int JsiNumberBase(const char *str, int *base, int *sign)
{
    int i = 0;

    *base = 10;

    while (isspace(UCHAR(str[i]))) {
        i++;
    }

    if (str[i] == '-') {
        *sign = -1;
        i++;
    }
    else {
        if (str[i] == '+') {
            i++;
        }
        *sign = 1;
    }

    if (str[i] != '0') {
        /* base 10 */
        return 0;
    }

    /* We have 0<x>, so see if we can convert it */
    switch (str[i + 1]) {
        case 'x': case 'X': *base = 16; break;
        case 'o': case 'O': *base = 8; break;
        case 'b': case 'B': *base = 2; break;
        default: return 0;
    }
    i += 2;
    /* Ensure that (e.g.) 0x-5 fails to parse */
    if (str[i] != '-' && str[i] != '+' && !isspace(UCHAR(str[i]))) {
        /* Parse according to this base */
        return i;
    }
    /* Parse as base 10 */
    return 10;
}

/* Converts a number as per strtoull(..., 0) except leading zeros do *not*
 * imply octal. Instead, decimal is assumed unless the number begins with 0x, 0o or 0b
 */
static Jsi_Wide jsi_strtoull(const char *str, char **endptr)
{
#ifdef HAVE_LONG_LONG
    int sign;
    int base;
    int i = JsiNumberBase(str, &base, &sign);

    if (base != 10) {
        Jsi_Wide value = strtoull(str + i, endptr, base);
        if (endptr == NULL || *endptr != str + i) {
            return value * sign;
        }
    }

    /* Can just do a regular base-10 conversion */
    return strtoull(str, endptr, 10);
#else
    return (unsigned long)jsi_strtol(str, endptr);
#endif
}

static Jsi_Wide jsi_strtoul(const char *str, char **endptr)
{
#ifdef HAVE_LONG_LONG
    int sign;
    int base;
    int i = JsiNumberBase(str, &base, &sign);

    if (base != 10) {
        Jsi_Wide value = strtoul(str + i, endptr, base);
        if (endptr == NULL || *endptr != str + i) {
            return value * sign;
        }
    }

    /* Can just do a regular base-10 conversion */
    return strtoul(str, endptr, 10);
#else
    return (unsigned long)jsi_strtol(str, endptr);
#endif
}


int Jsi_GetWide(Jsi_Interp* interp, const char *string, Jsi_Wide *widePtr, int base)
{
    char *endptr;

    if (base) {
        *widePtr = strtoull(string, &endptr, base);
    }
    else {
        *widePtr = jsi_strtoull(string, &endptr);
    }

    return JsiCheckConversion(string, endptr);
}

int Jsi_GetInt(Jsi_Interp* interp, const char *string, int *n, int base)
{
    char *endptr;
    if (base) {
        *n = strtoul(string, &endptr, base);
    }
    else {
        *n = (int)jsi_strtoul(string, &endptr);
    }
    return JsiCheckConversion(string, endptr);
}

int Jsi_GetDouble(Jsi_Interp* interp, const char *string, Jsi_Number *n)
{
    char *endptr;

    /* Callers can check for underflow via ERANGE */
    errno = 0;

    *n = strtod(string, &endptr);

    return JsiCheckConversion(string, endptr);
}

int Jsi_GetBool(Jsi_Interp* interp, const char *string, int *n)
{
    int len = strlen(string);
    if ((strncasecmp(string, "true", len)==0 && len<=4)) {
        *n = 1;
        return JSI_OK;
    }
    if ((strncasecmp(string, "false", len)==0 && len<=5)) {
        *n = 0;
        return JSI_OK;
    }
    return Jsi_GetInt(interp, string, n, 0);
}

#ifndef JSI_LITE_ONLY

void *Jsi_InterpGetData(Jsi_Interp *interp, const char *key, Jsi_DeleteProc **proc)
{
    Jsi_HashEntry *hPtr;
    AssocData *ptr;
    hPtr = Jsi_HashEntryFind(interp->assocTbl, key);
    if (!hPtr)
        return NULL;
    ptr = (AssocData *)Jsi_HashValueGet(hPtr);
    if (!ptr)
        return NULL;
    if (proc)
        *proc = ptr->delProc;
    return ptr->data;
}
void Jsi_InterpSetData(Jsi_Interp *interp, const char *key, Jsi_DeleteProc *proc, void *data)
{
    int isNew;
    Jsi_HashEntry *hPtr;
    AssocData *ptr;
    hPtr = Jsi_HashEntryNew(interp->assocTbl, key, &isNew);
    if (!hPtr)
        return;
    if (isNew) {
        ptr = (AssocData *)Jsi_Calloc(1,sizeof(*ptr));
        Jsi_HashValueSet(hPtr, ptr);
    } else
        ptr = (AssocData *)Jsi_HashValueGet(hPtr);
    ptr->data = data;
    ptr->delProc = proc;
}

void jsi_DelAssocData(Jsi_Interp *interp, void *data) {
    AssocData *ptr = (AssocData *)data;
    if (!ptr) return;
    if (ptr->delProc)
        ptr->delProc(interp, ptr->data);
    Jsi_Free(ptr);
}

void Jsi_InterpFreeData(Jsi_Interp *interp, const char *key)
{
    Jsi_HashEntry *hPtr;
    AssocData *ptr;
    hPtr = Jsi_HashEntryFind(interp->assocTbl, key);
    if (!hPtr)
        return;
    ptr = (AssocData *)Jsi_HashValueGet(hPtr);
    Jsi_HashEntryDelete(hPtr);
    jsi_DelAssocData(interp, ptr);
}

void Jsi_SetResultFormatted(Jsi_Interp *interp, const char *fmt, ...) {
    va_list va;
    char buf[BUFSIZ];
    va_start (va, fmt);
    vsnprintf(buf, BUFSIZ, fmt, va);
    va_end(va);
    fprintf(stderr, "<Jsi_SetResultFormatted NOTIMPL>: %s\n", buf);

}


int Jsi_GetStringFromValue(Jsi_Interp* interp, Jsi_Value *value, const char **n)
{
    if (!value)
        return JSI_ERROR;
    if (value->vt == JSI_VT_STRING)
    {
        *n = (const char*)value->d.s.str;
         return JSI_OK;
    }
    if (value->vt == JSI_VT_OBJECT && value->d.obj->ot == JSI_OT_STRING) {
        *n = value->d.obj->d.s.str;
        return JSI_OK;
    }
    Jsi_LogError("invalid string");
    return JSI_ERROR;
}

int Jsi_GetBoolFromValue(Jsi_Interp* interp, Jsi_Value *value, int *n)
{
    if (!value)
        return JSI_ERROR;

    if (value->vt == JSI_VT_BOOL) {
        *n = value->d.val;
        return JSI_OK;
    }
    if (value->vt == JSI_VT_OBJECT && value->d.obj->ot == JSI_OT_BOOL) {
        *n = value->d.obj->d.val;
        return JSI_OK;
    }
    Jsi_LogError("invalid bool");
    return JSI_ERROR;
}


int Jsi_GetNumberFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Number *n)
{
    if (!value)
        return JSI_ERROR;

    if (value->vt == JSI_VT_NUMBER) {
        *n = value->d.num;
        return JSI_OK;
    }
    if (value->vt == JSI_VT_OBJECT && value->d.obj->ot == JSI_OT_NUMBER) {
        *n = value->d.obj->d.num;
        return JSI_OK;
    }
    if (interp)
        Jsi_LogError("invalid number");
    return JSI_ERROR;
}

int Jsi_GetIntFromValueBase(Jsi_Interp* interp, Jsi_Value *value, int *n, int base, int flags)
{
    Jsi_Number d;
    int noMsg = (flags & JSI_NO_ERRMSG);
    /* TODO: inefficient to convert to double then back. */
    if (!value)
        return JSI_ERROR;
    d = Jsi_ValueToNumberInt(interp, value, 1);
    if (!jsi_num_isFinite(d))
    {
        if (!noMsg)
            Jsi_LogError("invalid number");
        return JSI_ERROR;
    }
    Jsi_ValueReset(interp,&value);
    Jsi_ValueMakeNumber(interp, &value, d);
    *n = (int)d;
    return JSI_OK;
}

int Jsi_GetIntFromValue(Jsi_Interp* interp, Jsi_Value *value, int *n)
{
    return Jsi_GetIntFromValueBase(interp, value, n, 0, 0);
}

int Jsi_GetLongFromValue(Jsi_Interp* interp, Jsi_Value *value, long *n)
{
    /* TODO: inefficient to convert to double then back. */
    if (!value)
        return JSI_ERROR;
    jsi_ValueToOInt32(interp, value);
    if (value->vt != JSI_VT_NUMBER)
    {
        Jsi_LogError("invalid number");
        return JSI_ERROR;
    }
    *n = (long)value->d.num;
    return JSI_OK;
}

int Jsi_GetWideFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Wide *n)
{
    if (!value)
        return JSI_ERROR;
    jsi_ValueToOInt32(interp, value);
    if (value->vt != JSI_VT_NUMBER)
    {
        Jsi_LogError("invalid number");
        return JSI_ERROR;
    }
    *n = (Jsi_Wide)value->d.num;
    return JSI_OK;

}

int Jsi_GetDoubleFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Number *n)
{
    if (!value)
        return JSI_ERROR;
    Jsi_ValueToNumber(interp, value);
    if (value->vt != JSI_VT_NUMBER)
    {
        Jsi_LogError("invalid number");
        return JSI_ERROR;
    }
    *n = value->d.num;
    return JSI_OK;
}

int
Jsi_ValueGetIndex( Jsi_Interp *interp, Jsi_Value *valPtr,
    const char **tablePtr, const char *msg, int flags, int *indexPtr)
{
    char *val = Jsi_ValueString(interp, valPtr, NULL);
    if (val == NULL) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    return Jsi_GetIndex(interp, val, tablePtr, msg, flags, indexPtr);
}

Jsi_Value *Jsi_InterpResult(Jsi_Interp *interp)
{
    return interp->retPtr;
}
#endif
