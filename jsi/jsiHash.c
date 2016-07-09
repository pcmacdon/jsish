#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#define REBUILD_MULTIPLIER  3

#if JSI_IS64BIT
#define RANDOM_INDEX        HashOneWord
#define DOWNSHIFT_START     62
#else 
#define RANDOM_INDEX(tablePtr, i) \
    (((((long) (i))*1103515245) >> (tablePtr)->downShift) & (tablePtr)->mask)
#define DOWNSHIFT_START 28
#endif

static jsi_Hash HashArray (const void *key, size_t length);
static Jsi_HashEntry *HashArrayFind (Jsi_Hash *tablePtr, const void 
*key);
static Jsi_HashEntry *HashArrayCreate (Jsi_Hash *tablePtr,
    const void *key, int *newPtr);
static jsi_Hash HashString (const char *string);
static void RebuildTable (Jsi_Hash *tablePtr);
static Jsi_HashEntry *HashStringFind (Jsi_Hash *tablePtr,
    const void *key);
static Jsi_HashEntry *HashStringCreate (Jsi_Hash *tablePtr,
    const void *key, int *newPtr);
static Jsi_HashEntry *HashOneWordFind (Jsi_Hash *tablePtr,
    const void *key);
static Jsi_HashEntry *HashOneWordCreate (Jsi_Hash *tablePtr,
    const void *key, int *newPtr);

#if JSI_IS64BIT
static jsi_Hash HashOneWord (Jsi_Hash *tablePtr,
    const void *key));

#endif /* JSI_IS64BIT */

static jsi_Hash
HashString( const char *string)
{
     jsi_Hash result;
     jsi_Hash c;

    result = 0;
    while ((c = *string++) != 0) {
    result += (result << 3) + c;
    }
    return (jsi_Hash)result;
}
static Jsi_HashEntry *
HashStringFind( Jsi_Hash *tablePtr, const void *key)
{
    jsi_Hash hval;
    Jsi_HashEntry *hPtr;
    size_t hindex;

    hval = HashString((char *)key);
    hindex = hval & tablePtr->mask;

    /*
     * Search all of the entries in the appropriate bucket.
     */

    for (hPtr = tablePtr->buckets[hindex]; hPtr != NULL;
            hPtr = hPtr->nextPtr) {
        if (hPtr->hval == hval) {
            const char *p1, *p2;

            for (p1 = (char*)key, p2 = hPtr->key.string; ; p1++, p2++) {
                if (*p1 != *p2) {
                    break;
                }
                if (*p1 == '\0') {
                    return hPtr;
                }
            }
        }
    }
    return NULL;
}

static Jsi_HashEntry *
HashStringCreate( Jsi_Hash *tablePtr, const void *key, int *newPtr)
{
    jsi_Hash hval;
    Jsi_HashEntry **bucketPtr;
    Jsi_HashEntry *hPtr;
    size_t size, hindex;

    hval = HashString((const char*)key);
    hindex = hval & tablePtr->mask;

    for (hPtr = tablePtr->buckets[hindex]; hPtr != NULL;
            hPtr = hPtr->nextPtr) {
        if (hPtr->hval == hval) {
            const char *p1, *p2;

            for (p1 = (const char*)key, p2 = hPtr->key.string; ; p1++, p2++) {
                if (*p1 != *p2) {
                    break;
                }
                if (*p1 == '\0') {
                    if (newPtr)
                        *newPtr = 0;
                    return hPtr;
                }
            }
        }
    }

    if (newPtr)
        *newPtr = 1;
    size = sizeof(Jsi_HashEntry) + strlen((char*)key) - sizeof(jsi_HashKey) + 1;
    hPtr = (Jsi_HashEntry*)Jsi_Malloc(size);
    bucketPtr = tablePtr->buckets + hindex;
    hPtr->tablePtr = tablePtr;
    hPtr->nextPtr = *bucketPtr;
    hPtr->hval = hval;
    hPtr->clientData = 0;
    Jsi_Strcpy(hPtr->key.string, (char*)key);
    *bucketPtr = hPtr;
    tablePtr->numEntries++;


    if (tablePtr->numEntries >= tablePtr->rebuildSize) {
        RebuildTable(tablePtr);
    }
    return hPtr;
}

#if JSI_IS64BIT
/*
 *----------------------------------------------------------------------
 *
 * HashOneWord --
 *
 *  Compute a one-word hash value of a 64-bit word, which then can
 *  be used to generate a hash index.
 *
 *  From Knuth, it's a multiplicative hash.  Multiplies an unsigned
 *  64-bit value with the golden ratio (sqrt(5) - 1) / 2.  The
 *  downshift value is 64 - n, when n is the log2 of the size of
 *  the hash table.
 *      
 * Results:
 *  The return value is a one-word summary of the information in
 *  64 bit word.
 *
 * Side effects:
 *  None.
 *
 *----------------------------------------------------------------------
 */
static jsi_Hash
HashOneWord(
    Jsi_Hash *tablePtr,
    const void *key)
{
    uint64_t a0, a1;
    uint64_t y0, y1;
    uint64_t y2, y3;
    uint64_t p1, p2;
    uint64_t result;
    /* Compute key * GOLDEN_RATIO in 128-bit arithmetic */
    a0 = (uint64_t)key & 0x00000000FFFFFFFF;
    a1 = (uint64_t)key >> 32;

    y0 = a0 * 0x000000007f4a7c13;
    y1 = a0 * 0x000000009e3779b9;
    y2 = a1 * 0x000000007f4a7c13;
    y3 = a1 * 0x000000009e3779b9;
    y1 += y0 >> 32;     /* Can't carry */
    y1 += y2;           /* Might carry */
    if (y1 < y2) {
        y3 += (1LL << 32);  /* Propagate */
    }

    /* 128-bit product: p1 = loword, p2 = hiword */
    p1 = ((y1 & 0x00000000FFFFFFFF) << 32) + (y0 & 0x00000000FFFFFFFF);
    p2 = y3 + (y1 >> 32);

    /* Left shift the value downward by the size of the table */
    if (tablePtr->downShift > 0) {
        if (tablePtr->downShift < 64) {
            result = ((p2 << (64 - tablePtr->downShift)) |
                      (p1 >> (tablePtr->downShift & 63)));
        } else {
            result = p2 >> (tablePtr->downShift & 63);
        }
    } else {
        result = p1;
    }
    /* Finally mask off the high bits */
    return (jsi_Hash)(result & tablePtr->mask);
}

#endif /* JSI_IS64BIT */

static Jsi_HashEntry *
HashOneWordFind( Jsi_Hash *tablePtr,  const void *key)
{
     Jsi_HashEntry *hPtr;
    size_t hindex;

    hindex = RANDOM_INDEX(tablePtr, key);

    /*
     * Search all of the entries in the appropriate bucket.
     */
    for (hPtr = tablePtr->buckets[hindex]; hPtr != NULL;
        hPtr = hPtr->nextPtr) {
    if (hPtr->key.oneWordValue == key) {
        return hPtr;
    }
    }
    return NULL;
}

static Jsi_HashEntry *
HashOneWordCreate( Jsi_Hash *tablePtr, const void *key, int *newPtr)
{
    Jsi_HashEntry **bucketPtr;
     Jsi_HashEntry *hPtr;
    size_t hindex;

    hindex = RANDOM_INDEX(tablePtr, key);


    for (hPtr = tablePtr->buckets[hindex]; hPtr != NULL;
        hPtr = hPtr->nextPtr) {
        if (hPtr->key.oneWordValue == key) {
            if (newPtr)
                *newPtr = 0;
            return hPtr;
        }
    }

    if (newPtr)
        *newPtr = 1;
    hPtr = (Jsi_HashEntry*)Jsi_Calloc(1,sizeof(*hPtr));
    hPtr->typ = JSI_MAP_HASH;
    bucketPtr = tablePtr->buckets + hindex;
    hPtr->tablePtr = tablePtr;
    hPtr->nextPtr = *bucketPtr;
    hPtr->hval = (jsi_Hash)key;
    hPtr->clientData = 0;
    hPtr->key.oneWordValue = (void *)key; 
    *bucketPtr = hPtr;
    tablePtr->numEntries++;


    if (tablePtr->numEntries >= tablePtr->rebuildSize) {
        RebuildTable(tablePtr);
    }
    return hPtr;
}

static jsi_Hash
HashArray(const void *key, size_t length )
{
    const char *string = (const char *) key;
    unsigned int result;
    int c, i;

    result = 0;

    for (i=0, c=*string++ ; i<(int)length; i++, c=*string++) {
        result += (result<<3) + c;
    }
    return (jsi_Hash)result;
}



static Jsi_HashEntry *
HashArrayFind( Jsi_Hash *tablePtr, const void *key)
{
    jsi_Hash hval;
     Jsi_HashEntry *hPtr;
    size_t hindex;

    hval = HashArray(key, tablePtr->keyType);
    hindex = hval & tablePtr->mask;

    for (hPtr = tablePtr->buckets[hindex]; hPtr != NULL;
        hPtr = hPtr->nextPtr) {
        if (hPtr->hval == hval) {
            if (memcmp(hPtr->key.words, key, tablePtr->keyType))
                break;
        }
    }
    return NULL;
}

static Jsi_HashEntry *
HashArrayCreate( Jsi_Hash *tablePtr,  const void *key, int *newPtr)
{
    jsi_Hash hval;
    Jsi_HashEntry **bucketPtr;
    int count;
     Jsi_HashEntry *hPtr;
    size_t size, hindex;

    hval = HashArray(key, tablePtr->keyType);
    hindex = hval & tablePtr->mask;

    /*
     * Search all of the entries in the appropriate bucket.
     */
    for (hPtr = tablePtr->buckets[hindex]; hPtr != NULL;
            hPtr = hPtr->nextPtr) {
        if (hPtr->hval == hval) {
            if (memcmp(hPtr->key.words, key, tablePtr->keyType))
                break;
        }
    }

    /* Entry not found.  Add a new one to the bucket. */
    if (newPtr)
        *newPtr = 1;
    /* We assume here that the size of the key is at least 2 words */
    size = sizeof(Jsi_HashEntry) + tablePtr->keyType -
           sizeof(jsi_HashKey);
    hPtr = (Jsi_HashEntry*)Jsi_Malloc(size);
    bucketPtr = tablePtr->buckets + hindex;
    hPtr->tablePtr = tablePtr;
    hPtr->nextPtr = *bucketPtr;
    hPtr->hval = hval;
    hPtr->clientData = 0;
    count = tablePtr->keyType;
    memcpy(hPtr->key.words, key, count);
    *bucketPtr = hPtr;
    tablePtr->numEntries++;

    /*
     * If the table has exceeded a decent size, rebuild it with many
     * more buckets.
     */
    if (tablePtr->numEntries >= tablePtr->rebuildSize) {
        RebuildTable(tablePtr);
    }
    return hPtr;
}


Jsi_HashEntry *
Jsi_HashEntryFind( Jsi_Hash *tablePtr, const void *key)
{
    if (tablePtr->lockProc && (*tablePtr->lockProc)(tablePtr, 1) != JSI_OK)
        return NULL;
    Jsi_HashEntry *hPtr = (*((tablePtr)->findProc))(tablePtr, key);
    if (tablePtr->lockProc)
        (*tablePtr->lockProc)(tablePtr, 0);
    return hPtr;
}

Jsi_HashEntry *
Jsi_HashEntryNew(Jsi_Hash *tablePtr, const void *key, int *newPtr)
{
    if (tablePtr->lockProc && (*tablePtr->lockProc)(tablePtr, 1) != JSI_OK)
        return NULL;
    Jsi_HashEntry *hPtr =  (*((tablePtr)->createProc))(tablePtr, key, newPtr);
#ifdef JSI_HAS_SIG_HASHENTRY
    SIGINIT(hPtr, HASHENTRY);
#endif
    if (tablePtr->lockProc)
        (*tablePtr->lockProc)(tablePtr, 0);
    return hPtr;
}

static void
RebuildTable(Jsi_Hash *tablePtr)
{
    Jsi_HashEntry **bucketPtr, **oldBuckets;
    Jsi_HashEntry **oldChainPtr, **endPtr;
    Jsi_HashEntry *hPtr, *nextPtr;
    size_t hindex;

    oldBuckets = tablePtr->buckets;
    endPtr = tablePtr->buckets + tablePtr->numBuckets;
    tablePtr->numBuckets <<= 2;
    tablePtr->buckets = (Jsi_HashEntry**)Jsi_Calloc(tablePtr->numBuckets, 
                   sizeof(Jsi_HashEntry *));
    tablePtr->rebuildSize <<= 2;
    tablePtr->downShift -= 2;
    tablePtr->mask = tablePtr->numBuckets - 1;

 
    if (tablePtr->keyType == JSI_KEYS_ONEWORD) {

    for (oldChainPtr = oldBuckets; oldChainPtr < endPtr; oldChainPtr++) {
        for (hPtr = *oldChainPtr; hPtr != NULL; hPtr = nextPtr) {
        nextPtr = hPtr->nextPtr;
        hindex = RANDOM_INDEX(tablePtr, hPtr->key.oneWordValue);
        bucketPtr = tablePtr->buckets + hindex;
        hPtr->nextPtr = *bucketPtr;
        *bucketPtr = hPtr;
        }
    }
    } else {
        for (oldChainPtr = oldBuckets; oldChainPtr < endPtr; oldChainPtr++) {
            for (hPtr = *oldChainPtr; hPtr != NULL; hPtr = nextPtr) {
            nextPtr = hPtr->nextPtr;
            hindex = hPtr->hval & tablePtr->mask;
            bucketPtr = tablePtr->buckets + hindex;
            hPtr->nextPtr = *bucketPtr;
            *bucketPtr = hPtr;
            }
        }
    }

    if (oldBuckets != tablePtr->staticBuckets) {
        Jsi_Free(oldBuckets);
    }
}

Jsi_Hash *
Jsi_HashNew(Jsi_Interp *interp, unsigned int keyType, Jsi_HashDeleteProc freeProc)
{
    Jsi_Hash *tablePtr = (Jsi_Hash*)Jsi_Calloc(1,sizeof(*tablePtr));
    SIGINIT(tablePtr, HASH);
    tablePtr->typ = JSI_MAP_HASH;
    tablePtr->interp = interp;
    tablePtr->buckets = tablePtr->staticBuckets;
    tablePtr->numBuckets = JSI_SMALL_HASH_TABLE;
    tablePtr->rebuildSize = JSI_SMALL_HASH_TABLE * REBUILD_MULTIPLIER;
    tablePtr->downShift = DOWNSHIFT_START;
    tablePtr->freeProc = freeProc;

    tablePtr->mask = (jsi_Hash)(tablePtr->numBuckets - 1);
    tablePtr->keyType = keyType;

    switch (keyType) {
    case JSI_KEYS_STRING:   /* NUL terminated string keys. */
        tablePtr->findProc = HashStringFind;
        tablePtr->createProc = HashStringCreate;
        break;

    case JSI_KEYS_STRINGKEY: /* Lookup from another String hash, eg. Jsi_KeyAdd() */
    case JSI_KEYS_ONEWORD: /* A pointer. */
        tablePtr->findProc = HashOneWordFind;
        tablePtr->createProc = HashOneWordCreate;
        break;

    default:            /* Structs. */
        if (keyType < JSI_KEYS_STRUCT_MINSIZE) {
            Jsi_LogError("Jsi_HashNew: Key size can't be %d, must be >= %d\n", keyType, JSI_KEYS_STRUCT_MINSIZE);
            Jsi_Free(tablePtr);
            return NULL;
        }
        tablePtr->findProc = HashArrayFind;
        tablePtr->createProc = HashArrayCreate;
        break;
    }
    return tablePtr;
}

int
Jsi_HashEntryDelete(Jsi_HashEntry *entryPtr)
{
    Jsi_HashEntry *prevPtr;
    Jsi_HashEntry **bucketPtr;
    size_t hindex;
    Jsi_Hash *tablePtr = entryPtr->tablePtr;
    Jsi_Interp *interp = tablePtr->interp;
    JSI_NOTUSED(interp);
    int cnt = 0;
#ifdef JSI_HAS_SIG_HASHENTRY
    SIGASSERT(entryPtr, HASHENTRY);
#endif
    if (tablePtr->lockProc && (*tablePtr->lockProc)(tablePtr, 1) != JSI_OK)
        return 0;
    if (tablePtr->keyType == JSI_KEYS_ONEWORD) {
        hindex = RANDOM_INDEX(tablePtr, (const void *)entryPtr->hval);
    } else {
        hindex = (entryPtr->hval & tablePtr->mask);
    }
    bucketPtr = tablePtr->buckets + hindex;
    if (*bucketPtr == entryPtr) {
        *bucketPtr = entryPtr->nextPtr;
        cnt++;
    } else {
        for (prevPtr = *bucketPtr; /*empty*/; prevPtr = prevPtr->nextPtr) {
            if (prevPtr == NULL) {
                Jsi_LogFatal("malformed bucket chain in Jsi_HashEntryDelete");
            }
            if (prevPtr->nextPtr == entryPtr) {
                prevPtr->nextPtr = entryPtr->nextPtr;
                cnt++;
                break;
            }
        }
    }
    if (tablePtr->lockProc)
        (*tablePtr->lockProc)(tablePtr, 0);
    tablePtr->numEntries--;
    Jsi_Free(entryPtr);
    return cnt;
}

void
Jsi_HashDelete(Jsi_Hash *tablePtr)
{
    Jsi_HashEntry *hPtr, *nextPtr;
    size_t i;
    if (!tablePtr)
        return;
    if (tablePtr->lockProc && (*tablePtr->lockProc)(tablePtr, 1) != JSI_OK)
        return;
    for (i = 0; i < (size_t)tablePtr->numBuckets; i++) {
        hPtr = tablePtr->buckets[i];
        while (hPtr != NULL) {
            nextPtr = hPtr->nextPtr;
            if (tablePtr->freeProc && hPtr->clientData)
                (tablePtr->freeProc)(tablePtr->interp, hPtr, hPtr->clientData);
            Jsi_Free(hPtr);
            hPtr = nextPtr;
        }
    }
    
    if (tablePtr->buckets != tablePtr->staticBuckets) {
        Jsi_Free(tablePtr->buckets);
    }
    if (tablePtr->lockProc)
        (*tablePtr->lockProc)(tablePtr, 0);
    Jsi_Free(tablePtr);
}

Jsi_HashEntry *
Jsi_HashEntryFirst(Jsi_Hash *tablePtr, Jsi_HashSearch *searchPtr) 
{
    searchPtr->tablePtr = tablePtr;
    searchPtr->nextIndex = 0;
    searchPtr->nextEntryPtr = NULL;
    return Jsi_HashEntryNext(searchPtr);
}


void *Jsi_HashValueGet(Jsi_HashEntry *h)
{
    return h->clientData;
}

void Jsi_HashValueSet(Jsi_HashEntry *h, void *value)
{
    h->clientData = value;
}

void *Jsi_HashKeyGet(Jsi_HashEntry *h)
{
    Jsi_Hash *tablePtr = h->tablePtr;
    if (tablePtr->keyType == JSI_KEYS_ONEWORD)
        return h->key.oneWordValue;
    else
        return h->key.string;
}

Jsi_HashEntry *
Jsi_HashEntryNext(Jsi_HashSearch *searchPtr)
{
    Jsi_HashEntry *hPtr;
    Jsi_Hash *tablePtr = searchPtr->tablePtr;
    int locked = 0;
    
    while (searchPtr->nextEntryPtr == NULL) {
        if (searchPtr->nextIndex >= (size_t)tablePtr->numBuckets) {
            if (tablePtr->lockProc && locked)
                (*tablePtr->lockProc)(tablePtr, 0);
            return NULL;
        }
        if (tablePtr->lockProc && locked == 0 && (*tablePtr->lockProc)(tablePtr, locked++) != JSI_OK)
            return NULL;
        searchPtr->nextEntryPtr =
            tablePtr->buckets[searchPtr->nextIndex];
        searchPtr->nextIndex++;
    }
    if (tablePtr->lockProc && locked)
        (*tablePtr->lockProc)(tablePtr, 0);
    hPtr = searchPtr->nextEntryPtr;
    searchPtr->nextEntryPtr = hPtr->nextPtr;
    return hPtr;
}


Jsi_HashEntry* Jsi_HashSet(Jsi_Hash *tbl, void *key, void *value) {
    Jsi_HashEntry *hPtr;
    int isNew;
    hPtr = Jsi_HashEntryNew(tbl, key, &isNew);
    if (!hPtr) return hPtr;
    Jsi_HashValueSet(hPtr, value);
    return hPtr;
}

void *Jsi_HashGet(Jsi_Hash *tbl, void *key) {
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryFind(tbl, key);
    if (!hPtr)
        return NULL;
    return Jsi_HashValueGet(hPtr);
}

#ifndef JSI_LITE_ONLY
int Jsi_HashKeysDump(Jsi_Interp *interp, Jsi_Hash *tablePtr, Jsi_Value **ret, int flags) {
    char *key;
    int n = 0;
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch search;
    Jsi_Obj *nobj;
    
    if (tablePtr->keyType != JSI_KEYS_STRING && tablePtr->keyType != JSI_KEYS_STRINGKEY)
        return JSI_ERROR;
    nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    for (hPtr = Jsi_HashEntryFirst(tablePtr, &search);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
        key = (char*)Jsi_HashKeyGet(hPtr);
        Jsi_ObjArraySet(interp, nobj, Jsi_ValueNewStringKey(interp, key), n++);
    }
    return JSI_OK;
}
#endif

int Jsi_HashSize(Jsi_Hash *hashPtr) { return hashPtr->numEntries; }
