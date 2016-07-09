#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#ifndef JSI_AMALGAMATION
#include "jsiUtf8.h"
#include "jsiInt.h"
#endif

#if 0
/* BEGIN: Code extracted from sqlite */
static const unsigned char Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#define WRITE_UTF8(zOut, c) {                          \
  if( c<0x00080 ){                                     \
    *zOut++ = (u8)(c&0xFF);                            \
  }                                                    \
  else if( c<0x00800 ){                                \
    *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  else if( c<0x10000 ){                                \
    *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }else{                                               \
    *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#define WRITE_UTF16LE(zOut, c) {                                    \
  if( c<=0xFFFF ){                                                  \
    *zOut++ = (u8)(c&0x00FF);                                       \
    *zOut++ = (u8)((c>>8)&0x00FF);                                  \
  }else{                                                            \
    *zOut++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *zOut++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *zOut++ = (u8)(c&0x00FF);                                       \
    *zOut++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#define WRITE_UTF16BE(zOut, c) {                                    \
  if( c<=0xFFFF ){                                                  \
    *zOut++ = (u8)((c>>8)&0x00FF);                                  \
    *zOut++ = (u8)(c&0x00FF);                                       \
  }else{                                                            \
    *zOut++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *zOut++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *zOut++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *zOut++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

#define READ_UTF16LE(zIn, TERM, c){                                   \
  c = (*zIn++);                                                       \
  c += ((*zIn++)<<8);                                                 \
  if( c>=0xD800 && c<0xE000 && TERM ){                                \
    int c2 = (*zIn++);                                                \
    c2 += ((*zIn++)<<8);                                              \
    c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);   \
  }                                                                   \
}

#define READ_UTF16BE(zIn, TERM, c){                                   \
  c = ((*zIn++)<<8);                                                  \
  c += (*zIn++);                                                      \
  if( c>=0xD800 && c<0xE000 && TERM ){                                \
    int c2 = ((*zIn++)<<8);                                           \
    c2 += (*zIn++);                                                   \
    c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);   \
  }                                                                   \
}

#define READ_UTF8(zIn, zTerm, c)                           \
  c = *(zIn++);                                            \
  if( c>=0xc0 ){                                           \
    c = Utf8Trans1[c-0xc0];                         \
    while( zIn!=zTerm && (*zIn & 0xc0)==0x80 ){            \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
    if( c<0x80                                             \
        || (c&0xFFFFF800)==0xD800                          \
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }        \
  }
  
static u32 Utf8Read(
  const unsigned char **pz    /* Pointer to string from which to read char */
){
  unsigned int c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  if( c>=0xc0 ){
    c = Utf8Trans1[c-0xc0];
    while( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    if( c<0x80
        || (c&0xFFFFF800)==0xD800
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }
  }
  return c;
}
/* END */
#endif

int jsi_utf8_fromunicode(char *p, unsigned uc)
{
    if (uc <= 0x7f) {
        *p = uc;
        return 1;
    }
    else if (uc <= 0x7ff) {
        *p++ = 0xc0 | ((uc & 0x7c0) >> 6);
        *p = 0x80 | (uc & 0x3f);
        return 2;
    }
    else if (uc <= 0xffff) {
        *p++ = 0xe0 | ((uc & 0xf000) >> 12);
        *p++ = 0x80 | ((uc & 0xfc0) >> 6);
        *p = 0x80 | (uc & 0x3f);
        return 3;
    }
    else {
        *p++ = 0xf0 | ((uc & 0x1c0000) >> 18);
        *p++ = 0x80 | ((uc & 0x3f000) >> 12);
        *p++ = 0x80 | ((uc & 0xfc0) >> 6);
        *p = 0x80 | (uc & 0x3f);
        return 4;
    }
}

int jsi_utf8_charlen(int c)
{
#ifndef JSI_UTF8
    return 1;
#else
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xe0) == 0xc0) return 2;
    if ((c & 0xf0) == 0xe0) return 3;
    if ((c & 0xf8) == 0xf0) return 4;
    return -1;
#endif
}

int jsi_utf8_strlen(const char *s) {
#ifndef JSI_UTF8
    return strlen(s);
#else
    int i = 0;
    while (*s)
        if ((*s++ & 0xc0) != 0x80)
            i++;
    return i;
#endif
}

int jsi_utf8_index(const char *str, int index)
{
#ifndef JSI_UTF8
    return str[index];
#else
    const char *s = str;
    while (index--) {
        int c;
        s += jsi_utf8_tounicode(s, &c);
    }
    return s - str;
#endif
}

int jsi_utf8_charequal(const char *s1, const char *s2)
{
#ifndef JSI_UTF8
    return (*s1 == *s2);
#else
    int c1, c2;
    jsi_utf8_tounicode(s1, &c1);
    jsi_utf8_tounicode(s2, &c2);
    return c1 == c2;
#endif
}

int jsi_utf8_prev_len(const char *str, int len)
{
#ifndef JSI_UTF8
    return 1;
#else
    int n = 1;
    if(len <= 0) return -1;
    while (--len) {
        if ((str[-n] & 0x80) == 0)
            break;
        if ((str[-n] & 0xc0) == 0xc0)
            break;
        n++;
    }
    return n;
#endif
}

int jsi_utf8_tounicode_case(const char *s, int *uc, int upper)
{
#ifndef JSI_UTF8
    if (upper)
        *uc = (int)toupper(*s);
    else
        *uc = (int)(*s);
    return 1;
#else
    int l = jsi_utf8_tounicode(s, uc);
    if (upper)
        *uc = jsi_utf8_upper(*uc);
    return l;
#endif
}

int jsi_utf8_tounicode(const char *str, int *uc)
{
#ifndef JSI_UTF8
    *uc = (char)*str;
    return 1;
#else
    unsigned const char *s = (unsigned const char *)str;

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
        if (((str[1] & 0xc0) == 0x80) && ((str[2] & 0xc0) == 0x80)) {
            *uc = ((s[0] & ~0xe0) << 12) | ((s[1] & ~0x80) << 6) | (s[2] & ~0x80);
            return 3;
        }
    }
    else if (s[0] < 0xf8) {
        if (((str[1] & 0xc0) == 0x80) && ((str[2] & 0xc0) == 0x80) && ((str[3] & 0xc0) == 0x80)) {
            *uc = ((s[0] & ~0xf0) << 18) | ((s[1] & ~0x80) << 12) | ((s[2] & ~0x80) << 6) | (s[3] & ~0x80);
            return 4;
        }
    }

    /* Invalid sequence, so just return the byte */
    *uc = *s;
    return 1;
#endif
}

#ifdef JSI_UTF8
struct casemap {
    unsigned short code;
    unsigned short altcode;
};

#include "jsiUniMap.c"

#define ARRAYSIZE(A) sizeof(A) / sizeof(*(A))

static int cmp_casemap(const void *key, const void *cm)
{
    return *(int *)key - (int)((const struct casemap *)cm)->code;
}

static int map_case(const struct casemap *mapping, int num, int ch)
{
    /* 16 bit case mapping only */
    if (ch <= 0xffff) {
        const struct casemap *cm =
            bsearch(&ch, mapping, num, sizeof(*mapping), cmp_casemap);

        if (cm) {
            return cm->altcode;
        }
    }
    return ch;
}
#endif

int jsi_utf8_upper(int ch)
{
#ifndef JSI_UTF8
    return toupper(ch);
#else
    if (isascii(ch)) {
        return toupper(ch);
    }
    return map_case(unicode_case_mapping_upper, ARRAYSIZE(unicode_case_mapping_upper), ch);
#endif
}

int jsi_utf8_lower(int ch)
{
#ifndef JSI_UTF8
    return tolower(ch);
#else
    if (isascii(ch)) {
        return tolower(ch);
    }
    return map_case(unicode_case_mapping_lower, ARRAYSIZE(unicode_case_mapping_lower), ch);
#endif
}

int jsi_utf8_title(int ch)
{
#ifndef JSI_UTF8
    return toupper(ch);
#else
    int newch = map_case(unicode_case_mapping_title, ARRAYSIZE(unicode_case_mapping_title), ch);
    if (newch != ch) {
        return newch ? newch : ch;
    }
    return jsi_utf8_upper(ch);
#endif
}

