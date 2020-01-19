#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>


#ifndef JSI_OMIT_MD5
// Source from Sqlite
struct jsi_MD5_Context {
    uint buf[4];
    uint bits[2];
    unsigned char in[64];
};
typedef char jsi_MD5Context[88];

/*
 * Note: this code is harmless on little-endian machines.
 */
static void jsi_MD5_byteReverse (unsigned char *buf, unsigned longs) {
    uint t;
    do {
        t = (uint)((unsigned)buf[3]<<8 | buf[2]) << 16 |
            ((unsigned)buf[1]<<8 | buf[0]);
        *(uint *)buf = t;
        buf += 4;
    } while (--longs);
}
/* The four core functions - _JSI_F1 is optimized somewhat */

/* #define _JSI_F1(x, y, z) (x & y | ~x & z) */
#define _JSI_F1(x, y, z) (z ^ (x & (y ^ z)))
#define _JSI_F2(x, y, z) _JSI_F1(z, x, y)
#define _JSI_F3(x, y, z) (x ^ y ^ z)
#define _JSI_F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define _JSI_MD5STEP(f, w, x, y, z, data, s) \
        ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  jsi_MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void jsi_MD5Transform(uint buf[4], const uint in[16]) {
    register uint a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    _JSI_MD5STEP(_JSI_F1, a, b, c, d, in[ 0]+0xd76aa478,  7);
    _JSI_MD5STEP(_JSI_F1, d, a, b, c, in[ 1]+0xe8c7b756, 12);
    _JSI_MD5STEP(_JSI_F1, c, d, a, b, in[ 2]+0x242070db, 17);
    _JSI_MD5STEP(_JSI_F1, b, c, d, a, in[ 3]+0xc1bdceee, 22);
    _JSI_MD5STEP(_JSI_F1, a, b, c, d, in[ 4]+0xf57c0faf,  7);
    _JSI_MD5STEP(_JSI_F1, d, a, b, c, in[ 5]+0x4787c62a, 12);
    _JSI_MD5STEP(_JSI_F1, c, d, a, b, in[ 6]+0xa8304613, 17);
    _JSI_MD5STEP(_JSI_F1, b, c, d, a, in[ 7]+0xfd469501, 22);
    _JSI_MD5STEP(_JSI_F1, a, b, c, d, in[ 8]+0x698098d8,  7);
    _JSI_MD5STEP(_JSI_F1, d, a, b, c, in[ 9]+0x8b44f7af, 12);
    _JSI_MD5STEP(_JSI_F1, c, d, a, b, in[10]+0xffff5bb1, 17);
    _JSI_MD5STEP(_JSI_F1, b, c, d, a, in[11]+0x895cd7be, 22);
    _JSI_MD5STEP(_JSI_F1, a, b, c, d, in[12]+0x6b901122,  7);
    _JSI_MD5STEP(_JSI_F1, d, a, b, c, in[13]+0xfd987193, 12);
    _JSI_MD5STEP(_JSI_F1, c, d, a, b, in[14]+0xa679438e, 17);
    _JSI_MD5STEP(_JSI_F1, b, c, d, a, in[15]+0x49b40821, 22);

    _JSI_MD5STEP(_JSI_F2, a, b, c, d, in[ 1]+0xf61e2562,  5);
    _JSI_MD5STEP(_JSI_F2, d, a, b, c, in[ 6]+0xc040b340,  9);
    _JSI_MD5STEP(_JSI_F2, c, d, a, b, in[11]+0x265e5a51, 14);
    _JSI_MD5STEP(_JSI_F2, b, c, d, a, in[ 0]+0xe9b6c7aa, 20);
    _JSI_MD5STEP(_JSI_F2, a, b, c, d, in[ 5]+0xd62f105d,  5);
    _JSI_MD5STEP(_JSI_F2, d, a, b, c, in[10]+0x02441453,  9);
    _JSI_MD5STEP(_JSI_F2, c, d, a, b, in[15]+0xd8a1e681, 14);
    _JSI_MD5STEP(_JSI_F2, b, c, d, a, in[ 4]+0xe7d3fbc8, 20);
    _JSI_MD5STEP(_JSI_F2, a, b, c, d, in[ 9]+0x21e1cde6,  5);
    _JSI_MD5STEP(_JSI_F2, d, a, b, c, in[14]+0xc33707d6,  9);
    _JSI_MD5STEP(_JSI_F2, c, d, a, b, in[ 3]+0xf4d50d87, 14);
    _JSI_MD5STEP(_JSI_F2, b, c, d, a, in[ 8]+0x455a14ed, 20);
    _JSI_MD5STEP(_JSI_F2, a, b, c, d, in[13]+0xa9e3e905,  5);
    _JSI_MD5STEP(_JSI_F2, d, a, b, c, in[ 2]+0xfcefa3f8,  9);
    _JSI_MD5STEP(_JSI_F2, c, d, a, b, in[ 7]+0x676f02d9, 14);
    _JSI_MD5STEP(_JSI_F2, b, c, d, a, in[12]+0x8d2a4c8a, 20);

    _JSI_MD5STEP(_JSI_F3, a, b, c, d, in[ 5]+0xfffa3942,  4);
    _JSI_MD5STEP(_JSI_F3, d, a, b, c, in[ 8]+0x8771f681, 11);
    _JSI_MD5STEP(_JSI_F3, c, d, a, b, in[11]+0x6d9d6122, 16);
    _JSI_MD5STEP(_JSI_F3, b, c, d, a, in[14]+0xfde5380c, 23);
    _JSI_MD5STEP(_JSI_F3, a, b, c, d, in[ 1]+0xa4beea44,  4);
    _JSI_MD5STEP(_JSI_F3, d, a, b, c, in[ 4]+0x4bdecfa9, 11);
    _JSI_MD5STEP(_JSI_F3, c, d, a, b, in[ 7]+0xf6bb4b60, 16);
    _JSI_MD5STEP(_JSI_F3, b, c, d, a, in[10]+0xbebfbc70, 23);
    _JSI_MD5STEP(_JSI_F3, a, b, c, d, in[13]+0x289b7ec6,  4);
    _JSI_MD5STEP(_JSI_F3, d, a, b, c, in[ 0]+0xeaa127fa, 11);
    _JSI_MD5STEP(_JSI_F3, c, d, a, b, in[ 3]+0xd4ef3085, 16);
    _JSI_MD5STEP(_JSI_F3, b, c, d, a, in[ 6]+0x04881d05, 23);
    _JSI_MD5STEP(_JSI_F3, a, b, c, d, in[ 9]+0xd9d4d039,  4);
    _JSI_MD5STEP(_JSI_F3, d, a, b, c, in[12]+0xe6db99e5, 11);
    _JSI_MD5STEP(_JSI_F3, c, d, a, b, in[15]+0x1fa27cf8, 16);
    _JSI_MD5STEP(_JSI_F3, b, c, d, a, in[ 2]+0xc4ac5665, 23);

    _JSI_MD5STEP(_JSI_F4, a, b, c, d, in[ 0]+0xf4292244,  6);
    _JSI_MD5STEP(_JSI_F4, d, a, b, c, in[ 7]+0x432aff97, 10);
    _JSI_MD5STEP(_JSI_F4, c, d, a, b, in[14]+0xab9423a7, 15);
    _JSI_MD5STEP(_JSI_F4, b, c, d, a, in[ 5]+0xfc93a039, 21);
    _JSI_MD5STEP(_JSI_F4, a, b, c, d, in[12]+0x655b59c3,  6);
    _JSI_MD5STEP(_JSI_F4, d, a, b, c, in[ 3]+0x8f0ccc92, 10);
    _JSI_MD5STEP(_JSI_F4, c, d, a, b, in[10]+0xffeff47d, 15);
    _JSI_MD5STEP(_JSI_F4, b, c, d, a, in[ 1]+0x85845dd1, 21);
    _JSI_MD5STEP(_JSI_F4, a, b, c, d, in[ 8]+0x6fa87e4f,  6);
    _JSI_MD5STEP(_JSI_F4, d, a, b, c, in[15]+0xfe2ce6e0, 10);
    _JSI_MD5STEP(_JSI_F4, c, d, a, b, in[ 6]+0xa3014314, 15);
    _JSI_MD5STEP(_JSI_F4, b, c, d, a, in[13]+0x4e0811a1, 21);
    _JSI_MD5STEP(_JSI_F4, a, b, c, d, in[ 4]+0xf7537e82,  6);
    _JSI_MD5STEP(_JSI_F4, d, a, b, c, in[11]+0xbd3af235, 10);
    _JSI_MD5STEP(_JSI_F4, c, d, a, b, in[ 2]+0x2ad7d2bb, 15);
    _JSI_MD5STEP(_JSI_F4, b, c, d, a, in[ 9]+0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void jsi_MD5Init(jsi_MD5Context *pCtx) {
    struct jsi_MD5_Context *ctx = (struct jsi_MD5_Context *)pCtx;
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;
    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static
void jsi_MD5Update(jsi_MD5Context *pCtx, const unsigned char *buf, unsigned int len) {
    struct jsi_MD5_Context *ctx = (struct jsi_MD5_Context *)pCtx;
    uint t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((uint)len << 3)) < t)
        ctx->bits[1]++; /* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if ( t ) {
        unsigned char *p = (unsigned char *)ctx->in + t;

        t = 64-t;
        if (len < t) {
            memcpy(p, buf, len);
            return;
        }
        memcpy(p, buf, t);
        jsi_MD5_byteReverse(ctx->in, 16);
        jsi_MD5Transform(ctx->buf, (uint *)ctx->in);
        buf += t;
        len -= t;
    }

    /* Process data in 64-byte chunks */

    while (len >= 64) {
        memcpy(ctx->in, buf, 64);
        jsi_MD5_byteReverse(ctx->in, 16);
        jsi_MD5Transform(ctx->buf, (uint *)ctx->in);
        buf += 64;
        len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void MD5Final(unsigned char digest[16], jsi_MD5Context *pCtx) {
    struct jsi_MD5_Context *ctx = (struct jsi_MD5_Context *)pCtx;
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
        /* Two lots of padding:  Pad the first block to 64 bytes */
        memset(p, 0, count);
        jsi_MD5_byteReverse(ctx->in, 16);
        jsi_MD5Transform(ctx->buf, (uint *)ctx->in);

        /* Now fill the next block with 56 bytes */
        memset(ctx->in, 0, 56);
    } else {
        /* Pad block to 56 bytes */
        memset(p, 0, count-8);
    }
    jsi_MD5_byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((uint *)ctx->in)[ 14 ] = ctx->bits[0];
    ((uint *)ctx->in)[ 15 ] = ctx->bits[1];

    jsi_MD5Transform(ctx->buf, (uint *)ctx->in);
    jsi_MD5_byteReverse((unsigned char *)ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset(ctx, 0, sizeof(*ctx));    /* In case it's sensitive */
}

int jsi_Md5Digest(unsigned char digest[16], const char *str, int len)
{
    jsi_MD5Context ctx;
    jsi_MD5Init(&ctx);
    if (len<0)
        len = Jsi_Strlen(str);
    jsi_MD5Update(&ctx, (unsigned char*)str, len);
    MD5Final(digest, &ctx);
    return 16;
}
#endif

#ifndef JSI_OMIT_SHA1

/*********************************************************************
* Author:     Brad Conte (brad AT bradconte.com) (copyright AS-IS)
* Source:     https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c
*********************************************************************/

typedef struct {
    uchar data[64];
    uint datalen;
    unsigned long long bitlen;
    uint state[5];
    uint k[4];
} jsi_SHA1_CTX;


/****************************** MACROS ******************************/
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

/*********************** FUNCTION DEFINITIONS ***********************/
void sha1_transform(jsi_SHA1_CTX *ctx, const uchar data[])
{
    uint a, b, c, d, e, i, j, t, m[80];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) + (data[j + 1] << 16) + (data[j + 2] << 8) + (data[j + 3]);
    for ( ; i < 80; ++i) {
        m[i] = (m[i - 3] ^ m[i - 8] ^ m[i - 14] ^ m[i - 16]);
        m[i] = (m[i] << 1) | (m[i] >> 31);
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];

    for (i = 0; i < 20; ++i) {
        t = ROTLEFT(a, 5) + ((b & c) ^ (~b & d)) + e + ctx->k[0] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 40; ++i) {
        t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + ctx->k[1] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 60; ++i) {
        t = ROTLEFT(a, 5) + ((b & c) ^ (b & d) ^ (c & d))  + e + ctx->k[2] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 80; ++i) {
        t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + ctx->k[3] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
}

void sha1_init(jsi_SHA1_CTX *ctx)
{
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;
    ctx->k[0] = 0x5a827999;
    ctx->k[1] = 0x6ed9eba1;
    ctx->k[2] = 0x8f1bbcdc;
    ctx->k[3] = 0xca62c1d6;
}

void sha1_update(jsi_SHA1_CTX *ctx, const uchar data[], size_t len)
{
    size_t i;

    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha1_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void sha1_final(jsi_SHA1_CTX *ctx, uchar hash[])
{
    uint i;

    i = ctx->datalen;

    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    }
    else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha1_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha1_transform(ctx, ctx->data);

    // Since this implementation uses little endian byte ordering and MD uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
    }
}
#endif

#ifndef JSI_OMIT_SHA1
static int jsi_Sha1Digest(unsigned char *digest, const char *str, int len)
{
    jsi_SHA1_CTX ctx;
    sha1_init(&ctx);
    if (len<0)
        len = Jsi_Strlen(str);
    sha1_update(&ctx, (unsigned char*)str, len);
    sha1_final(&ctx, digest);
    return 20;
}
#endif


#ifndef JSI_OMIT_SHA256

typedef struct {
    uchar data[64];
    uint datalen;
    unsigned long long bitlen;
    uint state[8];
} jsi_SHA256_CTX;

/****************************** MACROS ******************************/
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

/**************************** VARIABLES *****************************/
static const uint k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

/*********************** FUNCTION DEFINITIONS ***********************/
static void sha256_transform(jsi_SHA256_CTX *ctx, const uchar data[])
{
    uint a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init(jsi_SHA256_CTX *ctx)
{
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

static void sha256_update(jsi_SHA256_CTX *ctx, const uchar data[], size_t len)
{
    uint i;

    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(jsi_SHA256_CTX *ctx, uchar hash[])
{
    uint i;

    i = ctx->datalen;

    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    }
    else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}
#undef ROTLEFT
#undef ROTRIGHT
#undef CH
#undef MAJ
#undef EP0
#undef EP1
#undef SIG0
#undef SIG1

// https://github.com/mpancorbo/sha3

#define SHA3_ROUNDS       24
#define SHA3_STATE_LEN    25

#define SHA3_224                 0
#define SHA3_224_DIGEST_LENGTH  28
#define SHA3_224_CBLOCK        144

#define SHA3_256                 1
#define SHA3_256_DIGEST_LENGTH  32
#define SHA3_256_CBLOCK        136

#define SHA3_384                 2
#define SHA3_384_DIGEST_LENGTH  48
#define SHA3_384_CBLOCK        104

#define SHA3_512                3
#define SHA3_512_DIGEST_LENGTH 64
#define SHA3_512_CBLOCK        72

//#pragma pack(push, 1)
typedef struct {
  uint64_t state[SHA3_STATE_LEN];
  uint32_t index;
  size_t   dgstlen;
  uint32_t rounds;
  size_t   blklen;
  uint8_t  blk[256];
} SHA3_CTX;
//#pragma pack(pop)


static void SHA3_Init (SHA3_CTX *ctx, int type)
{
  memset (ctx, 0, sizeof (SHA3_CTX));
  ctx->rounds = SHA3_ROUNDS;
  
  switch (type)
  {
  case SHA3_224:
    ctx->blklen  = SHA3_224_CBLOCK;
    ctx->dgstlen = SHA3_224_DIGEST_LENGTH;
    break;
  case SHA3_384:
    ctx->blklen  = SHA3_384_CBLOCK;
    ctx->dgstlen = SHA3_384_DIGEST_LENGTH;
    break;
  case SHA3_512:
    ctx->blklen  = SHA3_512_CBLOCK;
    ctx->dgstlen = SHA3_512_DIGEST_LENGTH;
    break;
  default:
    ctx->blklen  = SHA3_256_CBLOCK;
    ctx->dgstlen = SHA3_256_DIGEST_LENGTH;
    break;
  }   
}
static const uint64_t keccakf_rndc[24] = 
{
  0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
  0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
  0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
  0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
  0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
  0x8000000000008003, 0x8000000000008002, 0x8000000000000080, 
  0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
  0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};

static const int keccakf_rotc[24] = 
{
  1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14, 
  27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
};

static const int keccakf_piln[24] = 
{
  10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4, 
  15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1 
};

#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))

static void SHA3_Transform (SHA3_CTX *ctx)
{
  uint32_t i, j, round;
  uint64_t t, bc[5];
  uint64_t *st=ctx->state;
  uint64_t *p=(uint64_t*)ctx->blk;
  
  // xor block with state
  for (i=0; i<ctx->blklen/8; i++) st[i] ^= p[i];
  
  for (round = 0; round < ctx->rounds; round++) 
  {
    // Theta
    for (i = 0; i < 5; i++)     
    bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

    for (i = 0; i < 5; i++) {
      t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
      for (j = 0; j < 25; j += 5)
      st[j + i] ^= t;
    }

    // Rho Pi
    t = st[1];
    for (i = 0; i < 24; i++) {
      j = keccakf_piln[i];
      bc[0] = st[j];
      st[j] = ROTL64(t, keccakf_rotc[i]);
      t = bc[0];
    }

    //  Chi
    for (j = 0; j < 25; j += 5) {
      for (i = 0; i < 5; i++)
      bc[i] = st[j + i];
      for (i = 0; i < 5; i++)
      st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
    }
    
    //  Iota
    st[0] ^= keccakf_rndc[round];
  }
}

static void SHA3_Update (SHA3_CTX* ctx, void *in, size_t len) {
  uint8_t *p;
  size_t  r, idx;

  p   = (uint8_t*)in;
  idx = ctx->index;
  
  do {
    r = ctx->blklen - idx;
    if (len<r)
        r = len;
    memcpy ((void*)&ctx->blk[idx], p, r);
    if ((idx + r) < ctx->blklen) {
      idx += r;
      break;
    }
    SHA3_Transform (ctx);
    len -= r;
    idx = 0;
    p += r;
  } while (1);
  ctx->index=idx;
}

static void SHA3_Final (void* dgst, SHA3_CTX* ctx)
{
  // add 3 bits, Keccak uses 1
  // a lot of online implementations are using 1 instead of 6
  // since the NIST specifications haven't been finalized.
  ctx->blk[ctx->index] = 6;
  // or the end bit
  ctx->blk[ctx->blklen-1] |= 0x80;
  // update context
  SHA3_Transform (ctx);
  // copy digest to buffer
  memcpy (dgst, (uint8_t*)ctx->state, ctx->dgstlen);
}

#endif

#ifndef JSI_OMIT_SHA256
static int jsi_Sha256Digest(unsigned char *digest, const char *str, int len)
{
    jsi_SHA256_CTX ctx;
    sha256_init(&ctx);
    if (len<0)
        len = Jsi_Strlen(str);
    sha256_update(&ctx, (unsigned char*)str, len);
    sha256_final(&ctx, digest);
    return 32;
}

static int jsi_Sha3Digest(unsigned char *digest, const char *str, int len, int type)
{
    SHA3_CTX ctx;
    SHA3_Init(&ctx, type);
    if (len<0)
        len = Jsi_Strlen(str);
    SHA3_Update(&ctx, (unsigned char*)str, len);
    SHA3_Final(digest, &ctx);
    return 64;
}
#endif


static int jsi_CryptoHash(unsigned char *digest, const char *str, int len, Jsi_CryptoHashType type)
{
    switch (type) {
        case Jsi_CHash_SHA1:
    #ifndef JSI_OMIT_SHA1
            return jsi_Sha1Digest(digest, str, len);
    #endif
            break;
        case Jsi_CHash_SHA2_256:
    #ifndef JSI_OMIT_SHA256
            return jsi_Sha256Digest(digest, str, len);
    #endif
            break;
        case Jsi_CHash_MD5:
    #ifndef JSI_OMIT_MD5
            return jsi_Md5Digest(digest, str, len);
    #endif
            break;
        case Jsi_CHash_SHA3_224:
        case Jsi_CHash_SHA3_384:
        case Jsi_CHash_SHA3_512:
        case Jsi_CHash_SHA3_256:
    #ifndef JSI_OMIT_SHA1
            return jsi_Sha3Digest(digest, str, len, type-Jsi_CHash_SHA3_224);
    #endif
            break;
    }
    return -1;
}

Jsi_RC Jsi_CryptoHash(char *outbuf, const char *str, int len, Jsi_CryptoHashType type, uint strength, bool noHex, int *sizPtr)
{
    int nonce = 0, dsiz = 0;
    unsigned char digest[1024];
    if (len<0)
        len = Jsi_Strlen(str);
    if (strength<=0)
        dsiz = jsi_CryptoHash(digest, str, len, type);
    else {
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        Jsi_DSAppendLen(&dStr, str, len);
        uint i, slen = Jsi_DSLength(&dStr);
        slen += 1+8;
        Jsi_DSSetLength(&dStr, slen);
        char *astr = Jsi_DSValue(&dStr);
        char *estr = astr + len;
        uint smask = 0, sbytes = (strength/8);
        uint sbits = (strength - sbytes*8);
        for (i=0; i<sbits; i++)
            smask = (smask>>1)|0x80;
        while (1) {
            snprintf(estr, 10, ":%08x", nonce);
            dsiz = jsi_CryptoHash(digest, astr, slen, type);
            i = 0;
            if (sbytes) {
                for (i=0; i<sbytes; i++)
                    if (digest[i]) break;
                if (i<sbytes) { nonce++; continue; }
            }
            if (!smask || !(digest[i]&smask))
                break;
            nonce++;
        }
        Jsi_DSFree(&dStr);
        if (dsiz<0)
            return JSI_ERROR;
    }

    if (noHex) {
        memcpy(outbuf, digest, dsiz);
        outbuf[dsiz] = 0;
    }
    else {
        jsi_ToHexStr(digest, dsiz, outbuf);
        dsiz = Jsi_Strlen(outbuf);
    }
    if (sizPtr)
        *sizPtr = dsiz;
    return JSI_OK;
}

#ifndef JSI_OMIT_ENCRYPT
/* XXTEA encryption. Source the Wikipedia page. Key modified to use 32 byte SHA256 instead of 16 byte MD5*/
#define _JSI_XXTEA_MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&7)^e] ^ z)))

static void jsi_btea(uint32_t *v, int n, uint32_t const key[8]) {
    const uint32_t DELTA = 0x9e3779b9;
    uint32_t y, z, sum;
    uint p, rounds, e;
    if (n > 1) {          /* Coding Part */
        rounds = 6 + 52/n;
        sum = 0;
        z = v[n-1];
        do {
            sum += DELTA;
            e = (sum >> 2) & 7;
            for (p=0; p<(uint)(n-1); p++) {
                y = v[p+1];
                z = v[p] += _JSI_XXTEA_MX;
            }
            y = v[0];
            z = v[n-1] += _JSI_XXTEA_MX;
        } while (--rounds);
    } else if (n < -1) {  /* Decoding Part */
        n = -n;
        rounds = 6 + 52/n;
        sum = rounds*DELTA;
        y = v[0];
        do {
            e = (sum >> 2) & 7;
            for (p=n-1; p>0; p--) {
                z = v[p-1];
                y = v[p] -= _JSI_XXTEA_MX;
            }
            z = v[n-1];
            y = v[0] -= _JSI_XXTEA_MX;
            sum -= DELTA;
        } while (--rounds);
    }
}

/* Encrypt/decrypt string using key. */
Jsi_RC Jsi_Encrypt(Jsi_Interp *interp, Jsi_DString *inout, const char *key, uint keyLen, bool decrypt)
{
    uchar digest[32];
    char *buf = Jsi_DSValue(inout);
    uint pad = 0, nlen = Jsi_DSLength(inout);
    if (nlen<2)
        goto badlen;
    if (decrypt) {
        pad = (uint)buf[--nlen];
        Jsi_DSSetLength(inout, nlen);
    } else {
        while (nlen%4) {
            Jsi_DSAppendLen(inout, "\0", 1);
            nlen++;
            pad++;
        }
    }

    if (nlen%4) {
badlen:
        return Jsi_LogError("length must be a multiple of 4: %d, %d", nlen, nlen%4);
    }
    nlen /= 4;
    if (keyLen==sizeof(digest))
        memcpy(digest, key, sizeof(digest));
    else
        jsi_Sha256Digest(digest, key, keyLen);
    buf = Jsi_DSValue(inout);
    jsi_btea((uint32_t*)buf, (decrypt ? -nlen : nlen), (uint32_t*)digest);
    if (decrypt) {
        buf = Jsi_DSValue(inout);
        nlen = Jsi_DSLength(inout);
        int olen = (nlen-pad);
        if (olen<0)
            return JSI_OK; // quietly fail
        Jsi_DSSetLength(inout, olen);
    } else {
        char spad = pad;
        Jsi_DSAppendLen(inout, &spad, 1);
    }
    return JSI_OK;
}

#else
Jsi_RC Jsi_Encrypt(Jsi_Interp *interp, Jsi_DString *inout, const char *key, uint keyLen, bool decrypt)
{
    return Jsi_LogError("Jsi_Encrypt unsupported");
}
#endif

#endif
