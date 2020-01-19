#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

// Returns length.
uint Jsi_UniCharToUtf(Jsi_UniChar uc, char *dest)
{
    if (uc <= 0x7f) {
        *dest = uc;
        return 1;
    }
    if (uc <= 0x7ff) {
        *dest++ = 0xc0 | ((uc & 0x7c0) >> 6);
        *dest = 0x80 | (uc & 0x3f);
        return 2;
    }
    if (uc <= 0xffff) {
        *dest++ = 0xe0 | ((uc & 0xf000) >> 12);
        *dest++ = 0x80 | ((uc & 0xfc0) >> 6);
        *dest = 0x80 | (uc & 0x3f);
        return 3;
    }
    *dest++ = 0xf0 | ((uc & 0x1c0000) >> 18);
    *dest++ = 0x80 | ((uc & 0x3f000) >> 12);
    *dest++ = 0x80 | ((uc & 0xfc0) >> 6);
    *dest = 0x80 | (uc & 0x3f);
    return 4;
}

uint Jsi_NumUtfBytes(char c)
{
#if !JSI__UTF8
    return 1;
#else
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xe0) == 0xc0) return 2;
    if ((c & 0xf0) == 0xe0) return 3;
    if ((c & 0xf8) == 0xf0) return 4;
    return -1;
#endif
}

uint Jsi_NumUtfChars(const char *s, int length) {
    if (!s || !length) return 0;
#if !JSI__UTF8
    uint len = Jsi_Strlen(s);
    if (length<0) return len;
    if (len>length) return length;
    return len;
#else
    uint i = 0;
    if (length<0) {
        while (*s)
            if ((*s++ & 0xc0) != 0x80)
                i++;
        return i;
    }
    const char *so = s;
    while (1) {
        if ((*s++ & 0xc0) != 0x80)
            i++;
        if ((s-so)>=length)
            break;
    }
    return i;
#endif
}

uint Jsi_UtfGetIndex(const char *str, int index, char cbuf[5]) {
#if !JSI__UTF8
    cbuf[0] = str[index];
    cbuf[1] = 0;
    return 1;
#else
    const char *bp = Jsi_UtfAtIndex(str, index);
    int l = 0;
    if (!bp)
        cbuf[0] = 0;
    else {
        l = Jsi_NumUtfBytes(*bp);
        if (l<0) l = 0;
        else Jsi_Strncpy(cbuf, bp, l+1);
        cbuf[l] = 0;
    }
    return l;
#endif
}

char* Jsi_UtfSubstr(const char *str, int n, int len, Jsi_DString *dStr) {
    int ulen, ustart;
    if (n<0) {
        int lenofa = Jsi_NumUtfChars(str, -1);
        if (lenofa<=0)
            return Jsi_DSValue(dStr);
        while (n < 0) n += lenofa;
    }
#if !JSI__UTF8
    ustart = n;
    ulen = len;
#else
    int m, e, l;
    ulen = ustart = 0;
    m = Jsi_UtfIndexToOffset(str, n);
    if (m>=0) {
        if (len<0) {
            ustart = m;
            ulen = Jsi_Strlen(str+m);
        } else {
            e = Jsi_UtfIndexToOffset(str+m, len);
            l = Jsi_NumUtfBytes(str[m]);
            if (l<0) l = 1;
            if (e>=0 && l>0) {
                ustart = m;
                ulen = e+l-1;
            }
        }
    }
#endif
    if (ulen>len) {
#if JSI__MEMDEBUG
        fprintf(stderr, "TODO: fix utf substr\n");
#endif
        ulen = len;
    }
    Jsi_DSAppendLen(dStr, str+ustart, ulen);
    return Jsi_DSValue(dStr);
}

int Jsi_UtfIndexToOffset(const char *str, int index)
{
#if !JSI__UTF8
    return index;
#else
    const char *s = str;
    while (index-- && *s) {
        int c;
        if (*s&0x80)
            s += Jsi_UtfToUniChar(s, &c);
        else
            s++;
    }
    return s - str;
#endif
}

const char* Jsi_UtfAtIndex(const char *str, int index)
{
#if !JSI__UTF8
    int slen = Jsi_Strlen(str);
    return (index<slen?str+index:NULL);
#else
    const char *s = str;
    while (index-- && *s) {
        int c;
        if (*s&0x80)
            s += Jsi_UtfToUniChar(s, &c);
        else
            s++;
    }
    return (index<0 ? s : NULL);
#endif
}

// Convert utf to unicode with optional upcase. Returns UTF size in bytes
uint Jsi_UtfToUniCharCase(const char *utf, Jsi_UniChar *uc, int upper)
{
#if !JSI__UTF8
    *uc = (Jsi_UniChar)(upper?toupper(*utf):*utf);
    return 1;
#else
    int l = Jsi_UtfToUniChar(utf, uc);
    if (upper && *uc<0x80)
        *uc = toupper(*uc);
    return l;
#endif
}

// Convert utf to unicode.  Returns UTF size in bytes.
uint Jsi_UtfToUniChar(const char *utf, Jsi_UniChar *uc)
{
#if !JSI__UTF8
    *uc = (char)*utf;
    return 1;
#else
    unsigned const char *s = (unsigned const char *)utf;

    if (s[0] < 0xc0) {
        *uc = s[0];
        return 1;
    }
    if (s[0] < 0xe0) {
        if ((s[1] & 0xc0) == 0x80) {
            *uc = ((s[0] & ~0xc0) << 6) | (s[1] & ~0x80);
            return 2;
        }
    }
    else if (s[0] < 0xf0) {
        if (((utf[1] & 0xc0) == 0x80) && ((utf[2] & 0xc0) == 0x80)) {
            *uc = ((s[0] & ~0xe0) << 12) | ((s[1] & ~0x80) << 6) | (s[2] & ~0x80);
            return 3;
        }
    }
    else if (s[0] < 0xf8) {
        if (((utf[1] & 0xc0) == 0x80) && ((utf[2] & 0xc0) == 0x80) && ((utf[3] & 0xc0) == 0x80)) {
            *uc = ((s[0] & ~0xf0) << 18) | ((s[1] & ~0x80) << 12) | ((s[2] & ~0x80) << 6) | (s[3] & ~0x80);
            return 4;
        }
    }

    /* Invalid sequence, so just return the byte */
    *uc = *s;
    return 1;
#endif
}

// Encode UTF number to \uNNNN form.  Returns size.
uint Jsi_UtfEncode(const char *innum, char *outstr) {
    int32_t ival;
    int ilen;
    outstr[0] = '\\';
    outstr[1] = 'u';
    outstr[6] = 0;
    ilen = Jsi_UtfToUniChar((char*)innum, &ival);
    outstr[2] = jsi_toHexChar((ival&0xF000)>>12);
    outstr[3] = jsi_toHexChar((ival&0xF00)>>8);
    outstr[4] = jsi_toHexChar((ival&0xF0)>>4);
    outstr[5] = jsi_toHexChar((ival&0x0F));
    return ilen;
}

// Decode \uNNNN string to a UTF number.
uint Jsi_UtfDecode(const char *str, char* uo) {
    char c;
    uint uc = 0, len = 0;
    int pos = 0;

    while(pos<4) {
        c = str[pos];
        if(!isxdigit(c)) {
            return 0;
        }
        uc += ((uint)jsi_fromHexChar(c) << ((3-pos++)*4));
    }
    if (uc < 0x80) {
        uo[0] = uc;
        len = 1;
    } else if (uc < 0x800) {
        uo[0] = 0xc0 | (uc >> 6);
        uo[1] = 0x80 | (uc & 0x3f);
        len = 2;
    } else {
        uo[0] = 0xe0 | (uc >> 12);
        uo[1] = 0x80 | ((uc >> 6) & 0x3f);
        uo[2] = 0x80 | (uc & 0x3f);
        len = 3;
    }
    return len;
}
