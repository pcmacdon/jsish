#ifndef JSI_LITE_ONLY
#ifndef JSI_OMIT_ZVFS
/*
** Copyright (c) 2000 D. Richard Hipp
** Copyright (c) 2007 Peter MacDonald
**
** This file is now released under the BSD style license
** outlined in the included file COPYING.
**
*************************************************************************
** A ZIP archive virtual filesystem for Jsi.
**
** Enables Jsish to use a Zip file (and itself) as a virtual file system
** that once mounted is visible across interpreters (TODO: needs a rewrite).
**
*/
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#ifdef __FreeBSD__
typedef int(* __compar_fn_t) (const void *, const void *);
#endif
#include <ctype.h>
#if JSI__MINIZ
#ifndef JSI_AMALGAMATION
#include "miniz/zlib.h"
#endif
#else
#include <zlib.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define SS(s) Jsi_ValueString(interp, s, 0)

/*
** Size of the decompression input buffer
*/
#define COMPR_BUF_SIZE   8192

typedef enum { ZVFS_SIG_PINFO = 0xbeefbeed, ZVFS_SIG_ARCHIVE, ZVFS_SIG_FILE, ZVFS_SIG_ZFILE } ZVFS_Sig;
#define ZVFSSIGASSERT(s,n) assert((s)->sig == ZVFS_SIG_##n)

/*
** Each ZIP archive file that is mounted is recorded as an instance
** of this structure
*/
typedef struct ZvfsArchive {
    Jsi_Value *Name;           /* Name of the archive */
    Jsi_Value *MountPoint;     /* Where this archive is mounted */
    struct ZvfsFile *pFiles;  /* List of files in that archive */
    Jsi_HashEntry *pEntry;
    Jsi_Channel chan;
} ZvfsArchive;

/*
** Particulars about each virtual file are recorded in an instance
** of the following structure.
*/
typedef struct ZvfsFile {
    char *zName;              /* The full pathname of the virtual file */
    ZvfsArchive *pArchive;    /* The ZIP archive holding this file data */
    int iOffset;              /* Offset into the ZIP archive of the data */
    int nByte;                /* Uncompressed size of the virtual file */
    int nByteCompr;           /* Compressed size of the virtual file */
    int gflags;
    uint32_t crc32;
    time_t timestamp;            /* Modification time */
    int isdir;            /* Set to 2 if directory, or 1 if mount */
    int depth;            /* Number of slashes in path. */
    int permissions;          /* File permissions. */
    Jsi_HashEntry *hPtr;
    struct ZvfsFile *pNext;      /* Next file in the same archive */
    struct ZvfsFile *pNextName;  /* A doubly-linked list of files with the same */
    struct ZvfsFile *pPrevName;  /*  name.  Only the first is in jsiIntData.zvfslocal.fileHash */
} ZvfsFile;

/*
** Information about each file within a ZIP archive is stored in
** an instance of the following structure.  A list of these structures
** forms a table of contents for the archive.
*/
typedef struct ZFile {
    char *zName;         /* Name of the file */
    int isSpecial;       /* Not really a file in the ZIP archive */
    int dosTime;         /* Modification time (DOS format) */
    int dosDate;         /* Modification date (DOS format) */
    int iOffset;         /* Offset into the ZIP archive of the data */
    int nByte;           /* Uncompressed size of the virtual file */
    int nByteCompr;      /* Compressed size of the virtual file */
    int nExtra;          /* Extra space in the TOC header */
    int iCRC;            /* Cyclic Redundancy Check of the data */
    int permissions;     /* File permissions. */
    int flags;            /* Deletion = bit 0. */
    struct ZFile *pNext;        /* Next file in the same archive */
} ZFile;

/*
** Whenever a ZVFS file is opened, an instance of this structure is
** attached to the open channel where it will be available to the
** ZVFS I/O routines below.  All state information about an open
** ZVFS file is held in this structure.
*/
typedef struct ZvfsChannelInfo {
    ZVFS_Sig sig;
    unsigned int nByte;       /* number of bytes of read uncompressed data */
    unsigned int nByteCompr;  /* number of bytes of unread compressed data */
    unsigned int nData;       /* total number of bytes of compressed data */
    int readSoFar;            /* Number of bytes read so far */
    long startOfData;         /* File position of start of data in ZIP archive */
    int isCompressed;         /* True -> data is compressed */
    int curPos;               /* Current pos. */
    Jsi_Channel chan;         /* Open to the archive file */
    unsigned char *zBuf;      /* buffer used by the decompressor */
    z_stream stream;          /* state of the decompressor */
    ZvfsFile *pFile;
    int bpos;
    int bsiz;                 /* Amount of data in buf. */
    char buf[COMPR_BUF_SIZE];
    int iCRC;            /* Cyclic Redundancy Check of the data */
    char emuGzip;
    char useGzip;
    Jsi_Interp *interp;
} ZvfsChannelInfo;


/*
** Macros to read 16-bit and 32-bit big-endian integers into the
** native format of this local processor.  B is an array of
** characters and the integer begins at the N-th character of
** the array.
*/
#define INT16(B, N) (B[N] + (B[N+1]<<8))
#define INT32(B, N) (INT16(B,N) + (B[N+2]<<16) + (B[N+3]<<24))


/*
** Make a new ZFile structure with space to hold a name of the number of
** characters given.  Return a pointer to the new structure.
*/
static ZFile *newZFile(int nName, ZFile **ppList) {
    ZFile *pNew;

    pNew = (ZFile*)Jsi_Calloc(1, sizeof(*pNew) + nName + 1 );
    pNew->zName = (char*)&pNew[1];
    pNew->pNext = *ppList;
    *ppList = pNew;
    return pNew;
}

/*
** Delete an entire list of ZFile structures
*/
static void deleteZFileList(ZFile *pList) {
    ZFile *pNext;
    while( pList ) {
        pNext = pList->pNext;
        Jsi_Free((char*)pList);
        pList = pNext;
    }
}

/* Convert DOS time to unix time. */
static void UnixTimeDate(struct tm *tm, int *dosDate, int *dosTime) {
    *dosDate = ((((tm->tm_year-80)<<9)&0xfe00) | (((tm->tm_mon+1)<<5)&0x1e0) | (tm->tm_mday&0x1f));
    *dosTime = (((tm->tm_hour<<11)&0xf800) | ((tm->tm_min<<5)&0x7e0) | (tm->tm_sec&0x1f));
}

/* Convert DOS time to unix time. */
static time_t DosTimeDate(int dosDate, int dosTime) {
    time_t now;
    struct tm *tm;
    now=time(NULL);
    tm = localtime(&now);
    tm->tm_year=(((dosDate&0xfe00)>>9) + 80);
    tm->tm_mon=((dosDate&0x1e0)>>5);
    tm->tm_mday=(dosDate & 0x1f);
    tm->tm_hour=(dosTime&0xf800)>>11;
    tm->tm_min=(dosTime&0x7e0)>>5;
    tm->tm_sec=(dosTime&0x1f);
    return mktime(tm);
}

/* Return count of char ch in str */
static int strchrcnt(char *str, char ch) {
    int cnt=0;
    char *cp=str;
    while ((cp=Jsi_Strchr(cp,ch))) {
        cp++;
        cnt++;
    }
    return cnt;
}


/* TODO: merge back into jsiFilesys.c
** Concatenate zTail onto zRoot to form a pathname.  zRoot will begin
** with "/".  After concatenation, simplify the pathname be removing
** unnecessary ".." and "." directories.  Under windows, make all
** characters lower case.
**
** Resulting pathname is returned.  Space to hold the returned path is
** obtained form Jsi_Alloc() and should be freed by the calling function.
*/
static char *CanonicalPath(const char *zRoot, const char *zTail) {
    char *zPath;
    int i, j, c;

#ifdef __WIN32__
    if( isalpha(zTail[0]) && zTail[1]==':' ) {
        zTail += 2;
    }
    if( zTail[0]=='\\' ) {
        zRoot = "";
        zTail++;
    }
#endif
    if( zTail[0]=='/' ) {
        zRoot = "";
        zTail++;
    }
    zPath = (char*)Jsi_Malloc( Jsi_Strlen(zRoot) + Jsi_Strlen(zTail) + 2 );
    if( zPath==0 ) return 0;
    if (zTail[0]) {
        sprintf(zPath, "%s/%s", zRoot, zTail);
    } else {
        Jsi_Strcpy(zPath, zRoot);
    }
    for(i=j=0; (c = zPath[i])!=0; i++) {
        if(jsiIntData.tolowerZvfs && isupper(c))
            c = tolower(c);
#ifdef __WIN32__
        if( c=='\\' ) c = '/';
#endif
        if( c=='/' ) {
            int c2 = zPath[i+1];
            if( c2=='/' ) continue;
            if( c2=='.' ) {
                int c3 = zPath[i+2];
                if( c3=='/' || c3==0 ) {
                    i++;
                    continue;
                }
                if( c3=='.' && (zPath[i+3]=='.' || zPath[i+3]==0) ) {
                    i += 2;
                    while( j>0 && zPath[j-1]!='/' ) {
                        j--;
                    }
                    continue;
                }
            }
        }
        zPath[j++] = c;
    }
    if( j==0 ) {
        zPath[j++] = '/';
    }

    zPath[j] = 0;
    return zPath;
}

static Jsi_RC ZvfsReadTOCStart(
    Jsi_Interp *interp,    /* Leave error messages in this interpreter */
    Jsi_Channel chan,
    ZFile **pList,
    int *iStart
) {
// char *zArchiveName = 0;    /* A copy of zArchive */
    int nFile;                 /* Number of files in the archive */
    int iPos;                  /* Current position in the archive file */
    //ZvfsArchive *pArchive;     /* The ZIP archive being mounted */
    //Jsi_HashEntry *pEntry;     /* Hash table entry */
    //bool isNew;                 /* Flag to tell use when a hash entry is new */
    unsigned char zBuf[100];   /* Space into which to read from the ZIP archive */
    //Jsi_HashSearch zSearch;   /* Search all mount points */
    ZFile *p;
    int zipStart;

    if (!chan) {
        return JSI_ERROR;
    }
    /* TODO: if we ever support utf8 properly, these might do something. */
    if (Jsi_SetChannelOption(interp, chan, "-translation", "binary") != JSI_OK) {
        return JSI_ERROR;
    }
    if (Jsi_SetChannelOption(interp, chan, "-encoding", "binary") != JSI_OK) {
        return JSI_ERROR;
    }

    /* Read the "End Of Central Directory" record from the end of the
    ** ZIP archive.
    */
    Jsi_Seek(interp, chan, -22, SEEK_END);
    iPos = Jsi_Tell(interp, chan);
    Jsi_Read(interp, chan, (char*)zBuf, 22);
    if (memcmp(zBuf, "\120\113\05\06", 4)) {
        /* Jsi_LogError("not a ZIP archive"); */
        return JSI_BREAK;
    }

    /* Compute the starting location of the directory for the ZIP archive
    ** in iPos then seek to that location.
    */
    zipStart = iPos;
    nFile = INT16(zBuf,8);
    iPos -= INT32(zBuf,12);
    Jsi_Seek(interp, chan, iPos, SEEK_SET);

    while(1) {
        int lenName;            /* Length of the next filename */
        int lenExtra;           /* Length of "extra" data for next file */
        int iData;              /* Offset to start of file data */
        // int dosTime;
        //int dosDate;
        //int isdir;
        //ZvfsFile *pZvfs;        /* A new virtual file */
        // char *zFullPath;        /* Full pathname of the virtual file */
        //char zName[1024];       /* Space to hold the filename */

        if (nFile-- <= 0 ) {
            break;
        }
        /* Read the next directory entry.  Extract the size of the filename,
        ** the size of the "extra" information, and the offset into the archive
        ** file of the file data.
        */
        Jsi_Read(interp, chan, (char*)zBuf, 46);
        if (memcmp(zBuf, "\120\113\01\02", 4)) 
            return Jsi_LogError("ill-formed central directory entry");
        lenName = INT16(zBuf,28);
        lenExtra = INT16(zBuf,30) + INT16(zBuf,32);
        iData = INT32(zBuf,42);
        if (iData<zipStart) {
            zipStart = iData;
        }

        p = newZFile(lenName, pList);
        if (!p) break;

        Jsi_Read(interp, chan, p->zName, lenName);
        p->zName[lenName] = 0;
        if (lenName>0 && p->zName[lenName-1] == '/') {
            p->isSpecial = 1;
        }
        p->dosDate = INT16(zBuf, 14);
        p->dosTime = INT16(zBuf, 12);
        p->nByteCompr = INT32(zBuf, 20);
        p->nByte = INT32(zBuf, 24);
        p->nExtra = INT32(zBuf, 28);
        p->iCRC = INT32(zBuf, 32);

        if (nFile < 0)
            break;

        /* Skip over the extra information so that the next read will be from
        ** the beginning of the next directory entry.
        */
        Jsi_Seek(interp, chan, lenExtra, SEEK_CUR);
    }
    *iStart = zipStart;
    return JSI_OK;
}

static Jsi_RC ZvfsReadTOC(
    Jsi_Interp *interp,    /* Leave error messages in this interpreter */
    Jsi_Channel chan,
    ZFile **pList
) {
    int iStart;
    return ZvfsReadTOCStart( interp, chan, pList, &iStart);
}


static ZvfsArchive* ZvfsLookupMount(Jsi_Interp *interp, const char *path) {
    Jsi_HashEntry *pEntry;     /* Hash table entry */
    Jsi_HashSearch zSearch;   /* Search all mount points */
    ZvfsArchive *pArchive;     /* The ZIP archive being mounted */

    ZvfsArchive* match=0;
    if( jsiIntData.zvfslocal.isInit==0 || path==0) return 0;
    pEntry=Jsi_HashSearchFirst(jsiIntData.zvfslocal.archiveHash,&zSearch);
    while (pEntry) {
        if ((pArchive = (ZvfsArchive*)Jsi_HashValueGet(pEntry))) {
            const char *zpath = Jsi_ValueToString(interp, pArchive->MountPoint, NULL);
            if (!Jsi_Strcmp(zpath, path)) {
                match=(ZvfsArchive*)Jsi_HashValueGet(pEntry);
                break;
            }
        }
        pEntry=Jsi_HashSearchNext(&zSearch);
    }

    return match;
}

#define FN_info JSI_INFO("\
Given an mount point argument, returns the archive for it. \
Otherwise, returns an array of mount points")

static Jsi_RC ZvfsNamesCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                       Jsi_Value **ret, Jsi_Func *funcPtr)
{
    ZvfsArchive *pArchive;     /* The ZIP archive being mounted */
    Jsi_HashEntry *pEntry;     /* Hash table entry */
    Jsi_HashSearch zSearch;   /* Search all mount points */
    Jsi_DString pStr, mStr;
    Jsi_Value *mount = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_RC rc = JSI_OK;
    
    Jsi_DSInit(&pStr);
    Jsi_DSInit(&mStr);
    if( !jsiIntData.zvfslocal.isInit ) return JSI_ERROR;
    /* If null archive name, return all current mounts. */
    if (!mount) {
        if (!ret)
            return JSI_OK;
        Jsi_Obj* obj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        pEntry=Jsi_HashSearchFirst(jsiIntData.zvfslocal.archiveHash,&zSearch);
        while (pEntry) {
            if ((pArchive = (ZvfsArchive*)Jsi_HashValueGet(pEntry))) {
                Jsi_ObjArrayAdd(interp, obj, pArchive->MountPoint);
            }
            pEntry=Jsi_HashSearchNext(&zSearch);
        }
        Jsi_ValueMakeObject(interp, ret, obj);
        return JSI_OK;
    }
    const char *zMountPoint = Jsi_ValueToString(interp, mount, NULL);

    pArchive = ZvfsLookupMount(interp, zMountPoint);
    if (pArchive) {
        if (ret)
            Jsi_ValueDup2(interp, ret, pArchive->Name);
        else
            rc = JSI_ERROR;
    }
    Jsi_DSFree(&pStr);
    return rc;
}

#define FN_mount JSI_INFO("\
Read a ZIP archive and make entries in the virutal file hash table for all \
files contained therein.")
Jsi_RC Zvfs_Mount( Jsi_Interp *interp, Jsi_Value *archive, Jsi_Value *mount, Jsi_Value **ret)
{
    Jsi_Channel chan;          /* Used for reading the ZIP archive file */
    char *zArchiveName = 0;    /* A copy of zArchive */
    int nFile;                 /* Number of files in the archive */
    int iPos;                  /* Current position in the archive file */
    ZvfsArchive *pArchive;     /* The ZIP archive being mounted */
    Jsi_HashEntry *pEntry;     /* Hash table entry */
    bool isNew;                 /* Flag to tell use when a hash entry is new */
    unsigned char zBuf[100];   /* Space into which to read from the ZIP archive */
    unsigned int startZip;
    Jsi_DString pStr, mStr;
    const char *zMountPoint;
    char mbuf[100];

    Jsi_DSInit(&pStr);
    Jsi_DSInit(&mStr);
    if( !jsiIntData.zvfslocal.isInit ) return JSI_ERROR;
    /* If NULL mount, generate a mount point. */
    // TODO: cleanup allocations of Absolute() path.
    if (mount == NULL || Jsi_ValueIsNull(interp, mount)) {
        int n = 0;
        while (n++ < 1000) {
            snprintf(mbuf, sizeof(mbuf), "%s%d", JSI_ZVFS_DIR, n);
            if (!ZvfsLookupMount(interp, mbuf))
                break;
        }
        zMountPoint = mbuf;
    } else {
        zMountPoint = Jsi_ValueToString(interp, mount, NULL);
    }
    chan = Jsi_Open(interp, archive, "rb");
    if (!chan) {
        return JSI_ERROR;
    }
    /* Read the "End Of Central Directory" record from the end of the
    ** ZIP archive.
    */
    if (Jsi_Seek(interp, chan, -22, SEEK_END) != 0) {
        if (ret && *ret)
            Jsi_LogError("not a ZIP archive");
        goto bail;
    }
    iPos = Jsi_Tell(interp, chan);
    Jsi_Read(interp, chan, (char*)zBuf, 22);
    if (memcmp(zBuf, "\120\113\05\06", 4)) {
        if (ret && *ret)
            Jsi_LogError("not a ZIP archive");
        goto bail;
    }

    /* Construct the archive record
    */
    zArchiveName = Jsi_ValueNormalPath(interp, archive, &pStr);
    pEntry = Jsi_HashEntryNew(jsiIntData.zvfslocal.archiveHash, zArchiveName, &isNew);
    if( !isNew ) {
        pArchive = (ZvfsArchive*)Jsi_HashValueGet(pEntry);
        Jsi_LogError("already mounted at %s", SS(pArchive->MountPoint));
        Jsi_DSFree(&pStr);
        goto bail;
    }
    if (!*zMountPoint) {
        /* Empty string is the special case of mounting on itself. */
        zMountPoint = Jsi_ValueNormalPath(interp, archive, &mStr);
    } else if (zMountPoint != mbuf) {
        zMountPoint = Jsi_ValueNormalPath(interp, mount, &mStr);
    }
    pArchive = (ZvfsArchive*)Jsi_Calloc(1, sizeof(*pArchive));
    pArchive->Name = Jsi_ValueNewStringDup(interp, zArchiveName);
    Jsi_IncrRefCount(interp, pArchive->Name);
    Jsi_DSFree(&pStr);
    pArchive->MountPoint = Jsi_ValueNewStringDup(interp, zMountPoint);
    Jsi_IncrRefCount(interp, pArchive->MountPoint);
    pArchive->pFiles = 0;
    Jsi_HashValueSet(pEntry, pArchive);
    pArchive->pEntry = pEntry;

    /* Compute the starting location of the directory for the ZIP archive
    ** in iPos then seek to that location.
    */
    nFile = INT16(zBuf,8);
    iPos -= INT32(zBuf,12);
    Jsi_Seek(interp, chan, iPos, SEEK_SET);
    startZip = iPos;

    while(1) {
        int lenName;            /* Length of the next filename */
        int lenExtra = 0;           /* Length of "extra" data for next file */
        int iData;              /* Offset to start of file data */
        int dosTime;
        int dosDate;
        int isdir;
        int rb;
        ZvfsFile *pZvfs;        /* A new virtual file */
        char *zFullPath;        /* Full pathname of the virtual file */
        char zName[1024];       /* Space to hold the filename */

        if (nFile-- <= 0 ) {
            isdir = 1;
            zFullPath = CanonicalPath(zMountPoint, "");
            iData = startZip;
            goto addentry;
        }
        /* Read the next directory entry.  Extract the size of the filename,
        ** the size of the "extra" information, and the offset into the archive
        ** file of the file data.
        */
        Jsi_Read(interp, chan, (char*)zBuf, 46);
        if (memcmp(zBuf, "\120\113\01\02", 4)) {
            Jsi_LogError("ill-formed central directory entry");
            //TODO: cleanup.
            goto bail;
        }
        lenName = INT16(zBuf,28);
        lenExtra = INT16(zBuf,30) + INT16(zBuf,32);
        iData = INT32(zBuf,42);


        /* If the virtual filename is too big to fit in zName[], then skip
        ** this file
        */
        if( lenName >= (int)sizeof(zName) ) {
            Jsi_Seek(interp, chan, lenName + lenExtra, SEEK_CUR);
            continue;
        }

        /* Construct an entry in jsiIntData.zvfslocal.fileHash for this virtual file.
        */
        rb = Jsi_Read(interp, chan, zName, lenName);
        if (rb != lenName)
            goto bail;
        isdir=0;
        if (lenName>0 && zName[lenName-1] == '/') {
            lenName--;
            isdir=2;
        }
        zName[lenName] = 0;
        zFullPath = CanonicalPath(zMountPoint, zName);
addentry:
        pEntry = Jsi_HashEntryNew(jsiIntData.zvfslocal.fileHash, zFullPath, &isNew);
        if (!isNew) {
            Jsi_Free(zFullPath);
            goto skip;
        }
        pZvfs = (ZvfsFile*)Jsi_Calloc(1, sizeof(*pZvfs) );
        pZvfs->zName = zFullPath;
        pZvfs->pArchive = pArchive;
        pZvfs->isdir = isdir;
        pZvfs->depth=strchrcnt(zFullPath,'/');
        pZvfs->iOffset = iData;
        if (iData<(int)startZip) {
            startZip = iData;
        }
        dosDate = INT16(zBuf, 14);
        dosTime = INT16(zBuf, 12);
        pZvfs->timestamp = DosTimeDate(dosDate, dosTime);
        pZvfs->nByte = INT32(zBuf, 24);
        pZvfs->nByteCompr = INT32(zBuf, 20);
        pZvfs->pNext = pArchive->pFiles;
        pZvfs->permissions = (0xffff&(INT32(zBuf, 38) >> 16));
        pZvfs->gflags = INT16(zBuf, 8);
        pZvfs->crc32 = INT32(zBuf, 16);
        pArchive->pFiles = pZvfs;
        //pEntry = Jsi_HashEntryNew(jsiIntData.zvfslocal.fileHash, zFullPath, &isNew);
        /*printf("NEW(%d): %s\n", isNew, zFullPath);
        if( isNew ) {
            pZvfs->pNextName = 0;
        } else {
            goto skip;
            ZvfsFile *pOld = (ZvfsFile*)Jsi_HashValueGet(pEntry);
            pOld->pPrevName = pZvfs;
            pZvfs->pNextName = pOld;
        }*/
        pZvfs->pPrevName = 0;
        pZvfs->hPtr = pEntry;
        Jsi_HashValueSet(pEntry, (void*) pZvfs);

        if (nFile < 0)
            break;

        /* Skip over the extra information so that the next read will be from
        ** the beginning of the next directory entry.
        */
skip:
        Jsi_Seek(interp, chan, lenExtra, SEEK_CUR);
    }
    pArchive->chan = chan;

    if (ret && *ret)
        Jsi_ValueMakeStringDup(interp, ret, zMountPoint);
    Jsi_DSFree(&mStr);
    Jsi_DSFree(&pStr);
    interp->mountCnt++;
//done:
    return JSI_OK;

bail:
    if (chan)
        Jsi_Close(interp, chan);
    return JSI_ERROR;
}

/*
** Locate the ZvfsFile structure that corresponds to the file named.
** Return NULL if there is no such ZvfsFile.
*/
static ZvfsFile *ZvfsLookup(Jsi_Interp *interp, Jsi_Value *path) {
    char *zt;
    Jsi_HashEntry *pEntry;
    ZvfsFile *pFile;
    Jsi_DString dStr = {};
    int len, isdir = 0;
    if( jsiIntData.zvfslocal.isInit==0 ) return 0;
    zt = Jsi_ValueNormalPath(interp, path, &dStr);
    if (!zt)
        return NULL;
    len = dStr.len;
    if (len && zt[len-1] == '/') {
        isdir = 1;
        zt[len-1] = 0;
    }
    pEntry = Jsi_HashEntryFind(jsiIntData.zvfslocal.fileHash, zt);
    pFile = (ZvfsFile*)(pEntry ? Jsi_HashValueGet(pEntry) : 0);
    Jsi_DSFree(&dStr);
    if (isdir && pFile && !pFile->isdir)
        return NULL;
    return pFile;
}

/*
** Unmount zip given its mount point.
*/
static Jsi_RC Zvfs_Unmount(Jsi_Interp *interp, Jsi_Value *path) {
    char *zPath;
    ZvfsArchive *pArchive;
    ZvfsFile *pZvfs, *pNextFile, *pDel = NULL;
    Jsi_HashEntry *pEntry;
    Jsi_DString pStr = {};
    Jsi_RC rc = JSI_OK;
    // TODO: use jsiMain interp for mount/unmount.
    // TODO: if pwd in mount, change it.
    zPath = Jsi_ValueNormalPath(interp, path, &pStr);
    pArchive = ZvfsLookupMount(interp, zPath);
    Jsi_DSFree(&pStr);
    if (!pArchive)
        return JSI_ERROR;
    if (pArchive->pEntry) {
        pArchive->pEntry->clientData = NULL;
        Jsi_HashEntryDelete(pArchive->pEntry);
    }
    Jsi_Close(interp, pArchive->chan);
    Jsi_DecrRefCount(interp, pArchive->Name);
    Jsi_DecrRefCount(interp, pArchive->MountPoint);
    for(pZvfs=pArchive->pFiles; pZvfs; pZvfs=pNextFile) {
        pNextFile = pZvfs->pNext;
        if( pZvfs->pNextName ) {
            pZvfs->pNextName->pPrevName = pZvfs->pPrevName;
        }
        char *zName = pZvfs->zName;
        if( pZvfs->pPrevName ) {
            pZvfs->pPrevName->pNextName = pZvfs->pNextName;
        } else {
            pEntry = Jsi_HashEntryFind(jsiIntData.zvfslocal.fileHash, pZvfs->zName);
            assert(pEntry);
            if( pZvfs->pNextName ) {
                pDel = (typeof(pDel))Jsi_HashValueGet(pEntry); // TODO: Needs rework.
                Jsi_HashValueSet(pEntry, pZvfs->pNextName);
            } else {
                Jsi_HashEntryDelete(pEntry);
            }
        }
        Jsi_Free(zName);
    }
    if (pDel)
        Jsi_Free(pDel);
    Jsi_Free(pArchive);
    interp->mountCnt--;
    return rc;
}

/*
** zvfs::mount  Zip-archive-name  mount-point
**
** Create a new mount point on the given ZIP archive.  After this
** command executes, files contained in the ZIP archive will appear
** to Jsi to be regular files at the mount point.
**
** With no mount-point, return mount point for archive.
** With no archive, return all archive/mount pairs.
** If mount-point is specified as an empty string, mount on file path.
**
*/
static Jsi_RC ZvfsMountCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr) {
    Jsi_Value* File = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value* Mount = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_RC rc = Zvfs_Mount(interp, File, Mount, ret);
    if (rc != JSI_OK) {
        Jsi_LogError("mount failed for %s on %s", SS(File), SS(Mount));
    }
    return rc;
}

static Jsi_RC ZvfsUnmountCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                          Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value* Mount = Jsi_ValueArrayIndex(interp, args, 0);
    ZvfsArchive *pArchive;     /* The ZIP archive being mounted */
    Jsi_HashEntry *pEntry;     /* Hash table entry */
    Jsi_HashSearch zSearch;   /* Search all mount points */

    if (Zvfs_Unmount(interp, Mount) == JSI_OK) {
        return JSI_OK;
    }

    if( !jsiIntData.zvfslocal.isInit ) return JSI_ERROR;
    char *zMountPoint;
    char *zMount = SS(Mount);
    if (!zMount)
        return JSI_ERROR;
    pEntry=Jsi_HashSearchFirst(jsiIntData.zvfslocal.archiveHash,&zSearch);
    while (pEntry) {
        if (((pArchive = (ZvfsArchive*)Jsi_HashValueGet(pEntry)))
                && ((zMountPoint = SS(pArchive->MountPoint)))
                && zMountPoint[0]
                && (Jsi_Strcmp(zMountPoint, zMount) == 0)) {
            if (Zvfs_Unmount(interp, pArchive->Name) == JSI_OK) {
                return JSI_OK;
            }
            break;
        }
        pEntry=Jsi_HashSearchNext(&zSearch);
    }

    Jsi_LogError("unknown zvfs mount point or file: %s", zMount);
    return JSI_ERROR;
}

#define FN_stat JSI_INFO("\
Return details about the given file in the ZVFS.  The information \
consists of (1) the name of the ZIP archive that contains the file, \
(2) the size of the file after decompressions, (3) the compressed \
size of the file, and (4) the offset of the compressed data in the archive.")
static Jsi_RC ZvfsStatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                       Jsi_Value **ret, Jsi_Func *funcPtr)
{
    ZvfsFile *pFile;

    Jsi_Value *Filename = Jsi_ValueArrayIndex(interp, args, 0);
    pFile = ZvfsLookup(interp, Filename);
    if (pFile) 
    {
        Jsi_Obj *sobj = Jsi_ObjNew(interp);
        Jsi_ValueMakeObject(interp, ret, sobj);
        
        Jsi_ObjInsert(interp, sobj, "archive", pFile->pArchive->Name, 0);
        Jsi_ObjInsert(interp, sobj, "offset", Jsi_ValueNewNumber(interp, (Jsi_Number)pFile->iOffset), 0);
        Jsi_ObjInsert(interp, sobj, "size", Jsi_ValueNewNumber(interp, (Jsi_Number)pFile->nByte), 0);
        Jsi_ObjInsert(interp, sobj, "compressedSize", Jsi_ValueNewNumber(interp, (Jsi_Number)pFile->nByteCompr), 0);

    }
    return JSI_OK;
}

/*
** The JSI I/O system calls this function to actually read information
** from a ZVFS file.
*/
static int vfsInput (
    ZvfsChannelInfo* pInfo,  /* The channel to read from */
    char *buf,               /* Buffer to fill */
    int toRead,              /* Requested number of bytes */
    int *pErrorCode          /* Location of error flag */
) {
    Jsi_Interp *interp = pInfo->interp;
    if( toRead > (int)pInfo->nByte ) {
        toRead = pInfo->nByte;
    }
    if( toRead == 0 ) {
        return 0;
    }
    Jsi_Seek(interp, pInfo->chan, pInfo->curPos, SEEK_SET);
    if (!pInfo->isCompressed ) {
        toRead = Jsi_Read(interp, pInfo->chan, buf, toRead);
    } else {
        int err = Z_OK;
        z_stream *stream = &pInfo->stream;
        stream->next_out = (unsigned char*)buf;
        stream->avail_out = toRead;
        while (stream->avail_out) {
            if (!stream->avail_in && pInfo->nByteCompr>0)
            {
                int len = pInfo->nByteCompr;
                if (len > COMPR_BUF_SIZE) {
                    len = COMPR_BUF_SIZE;
                }
                len = Jsi_Read(interp, pInfo->chan, (char*)pInfo->zBuf, len);
                if (len==0) break;
                if (len<0) { err = Z_STREAM_END+1; break; }
                pInfo->nByteCompr -= len;
                stream->next_in = pInfo->zBuf;
                stream->avail_in = len;
            }
            err = inflate(stream, Z_NO_FLUSH);
            if (err) break;
        }
        if (err == Z_STREAM_END) {
            if ((stream->avail_out != 0)) {
                *pErrorCode = err; /* premature end */
                return -1;
            }
        } else if( err ) {
            *pErrorCode = err; /* some other zlib error */
            return -1;
        }
    }
    pInfo->curPos = Jsi_Tell(interp, pInfo->chan);

    if (toRead<0)
        *pErrorCode = Z_STREAM_END;
    else {
        pInfo->nByte -= toRead;
        pInfo->readSoFar += toRead;
        *pErrorCode = 0;
    }
    return toRead;
}

/*
** Move the file pointer so that the next byte read will be "offset".
*/
static int vfsSeek(
    ZvfsChannelInfo* pInfo,    /* The file structure */
    long offset,                /* Offset to seek to */
    int mode,                   /* One of SEEK_CUR, SEEK_SET or SEEK_END */
    int *pErrorCode             /* Write the error code here */
) {
    Jsi_Interp *interp = pInfo->interp;
    Jsi_Seek(interp, pInfo->chan, pInfo->curPos, SEEK_SET);
    switch( mode ) {
    case SEEK_CUR: {
        offset += pInfo->readSoFar;
        break;
    }
    case SEEK_END: {
        offset += pInfo->readSoFar + pInfo->nByte;
        break;
    }
    default: {
        /* Do nothing */
        break;
    }
    }
    if (offset < 0) offset = 0;
    if( !pInfo->isCompressed ) {
        Jsi_Seek(interp, pInfo->chan, offset + pInfo->startOfData, SEEK_SET);
        pInfo->nByte = pInfo->nData;
        pInfo->readSoFar = offset;
    } else {
        if( offset<pInfo->readSoFar ) {
            z_stream *stream = &pInfo->stream;
            inflateEnd(stream);
            stream->zalloc = (alloc_func)0;
            stream->zfree = (free_func)0;
            stream->opaque = (voidpf)0;
            stream->avail_in = 2;
            stream->next_in = pInfo->zBuf;
            pInfo->zBuf[0] = 0x78;
            pInfo->zBuf[1] = 0x01;
            inflateInit(&pInfo->stream);
            Jsi_Seek(interp, pInfo->chan, pInfo->startOfData, SEEK_SET);
            pInfo->nByte += pInfo->readSoFar;
            pInfo->nByteCompr = pInfo->nData;
            pInfo->readSoFar = 0;
        }
        while( pInfo->readSoFar < offset ) {
            int toRead, errCode;
            char zDiscard[100];
            toRead = offset - pInfo->readSoFar;
            if( toRead>(int)sizeof(zDiscard) ) toRead = (int)sizeof(zDiscard);
            vfsInput(pInfo, zDiscard, toRead, &errCode);
        }
    }
    pInfo->curPos = Jsi_Tell(interp, pInfo->chan);
    return pInfo->readSoFar;
}

static void put32(char *z, int v);

/***************** Compressed files ******************/

static Jsi_Channel Jfz_FSOpenProc (Jsi_Interp *interp, Jsi_Value *path, const char *modes)
{
    ZvfsChannelInfo *pInfo;
    Jsi_Channel chan;
    int mode = 0, rc, i, n, zLevel = Z_DEFAULT_COMPRESSION, useGzip = 1;
    char Mode[JSI_FSMODESIZE];
    const char *s = (modes ? modes : "r");
    
    for (i=0, n = 0; s[i]; i++) {
        switch (s[i]) {
            case '+': Jsi_LogError("+ is unsupported with z: %s", s); return NULL; break;
            case 'b': break;
            case 'r': mode |= JSI_FS_READONLY; break;
            case 'a': mode |= (JSI_FS_APPEND|JSI_FS_WRITEONLY); break;
            case 'w': mode |= JSI_FS_WRITEONLY; break;
            case 'Z': useGzip = 0; 
            case 'z': mode |= JSI_FS_COMPRESS; continue; break;
            case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                zLevel = s[i]-'1'+1; continue; break;
            default: Jsi_LogError("unknown mode char: %c", s[i]); return NULL;
        }
        Mode[n++] = s[i];
    }
    Mode[n] = 0;
    if (mode&JSI_FS_READONLY && mode&JSI_FS_WRITEONLY) {
        Jsi_LogError("simultaneous read and write is unsupported with z mode");
        return NULL; 
    }
    Jsi_Channel achan = Jsi_Open(interp, path, Mode);
    if (!achan) {
        return NULL;
    }
    pInfo = (ZvfsChannelInfo*)Jsi_Calloc(1, sizeof(*pInfo) );
    pInfo->interp = interp;
    pInfo->isCompressed = 1;
    pInfo->sig = ZVFS_SIG_PINFO;
    pInfo->pFile = NULL;
    pInfo->chan = achan;
    pInfo->useGzip = useGzip;
    //TODO: Jsi_CreateExitHandler(vfsExit, pInfo);
#ifdef MZ_DEFAULT_WINDOW_BITS
    int winBits = (useGzip?-MZ_DEFAULT_WINDOW_BITS:0);
    pInfo->emuGzip = useGzip;
#else
    int winBits = (useGzip?15+16:0);;
#endif
    z_stream *stream = &pInfo->stream;
    if (mode & JSI_FS_READONLY) {
        stream->next_in = (unsigned char *)pInfo->buf;
        Jsi_Seek(interp, achan, 0, SEEK_END);
        pInfo->nByte = Jsi_Tell(interp, achan);
        Jsi_Seek(interp, achan, 0, SEEK_SET);
        if (!winBits)
            rc = inflateInit(stream);
        else
            rc = inflateInit2(stream, winBits);
    } else {
        if (mode&JSI_FS_APPEND)
            Jsi_Seek(interp, achan, 0, SEEK_END);
        if (!winBits)
            rc = deflateInit(stream, zLevel);
        else
            rc = deflateInit2(stream, zLevel, Z_DEFLATED, winBits, 9, Z_DEFAULT_STRATEGY);
    }
    if (rc) {
        Jsi_Free(pInfo->zBuf);
        Jsi_Free(pInfo);
        Jsi_LogError("zinit failure");
        Jsi_Close(interp, achan);
        return NULL;
    }
    
    chan = (Jsi_Chan*)Jsi_Calloc(1,sizeof(Jsi_Chan));
    chan->fname = Jsi_ValueString(interp, path, NULL);
    chan->data = pInfo;
    Jsi_Strcpy(chan->modes, modes);
    chan->flags = (mode | (zLevel<<16)); 
    return chan;
}

static int Jfz_FSWriteProc(Jsi_Channel chan, const char *buf, int size) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    Jsi_Interp *interp = pInfo->interp;
    ZVFSSIGASSERT(pInfo, PINFO);
    char obuf[COMPR_BUF_SIZE];
    if (size<0) size = 0;
    int n, cnt = size;
    if (!buf) size = 0;
    z_stream *stream = &pInfo->stream;
    stream->next_out = (unsigned char*)obuf;
    stream->avail_out = sizeof(obuf);
    stream->next_in = (unsigned char*)buf;
    stream->avail_in = n = size;
    while (stream->avail_in > 0 || buf == NULL) {
        stream->total_out = 0;
        int status = deflate(stream, (buf?(size==0?Z_FULL_FLUSH:0):Z_FINISH));
        int out = stream->total_out;
        if (pInfo->emuGzip && buf && (status == Z_STREAM_END || status == Z_OK))
            pInfo->iCRC = crc32(pInfo->iCRC, (unsigned char*)buf, size - stream->avail_in);
        if ((status == Z_STREAM_END || status == Z_OK) && out>0)
        {
            if (pInfo->emuGzip && pInfo->readSoFar == 0 && /* Add gzip header */
                Jsi_Write(interp, pInfo->chan, "\37\213\10\0\0\0\0\0\0\3", 10) != 10)
                goto writefail;
            if (out != Jsi_Write(interp, pInfo->chan, obuf, out)) {
writefail:
                Jsi_LogError("write failed");
                cnt = -1;
                break;
            }
            cnt += out;
            pInfo->curPos = (pInfo->readSoFar += cnt);
            stream->next_out = (unsigned char*)(obuf+cnt);
            stream->avail_out = sizeof(obuf);
            if (pInfo->emuGzip && !buf) { /* Add gzip size and crc. */
                char psp[8];
                put32(psp, pInfo->iCRC);
                put32(psp+4, pInfo->nByte);
                if (Jsi_Write(interp, pInfo->chan, (char*)psp, 8) != 8)
                    goto writefail;
            }
            //cnt += out;
        }
        pInfo->nByte += (size - stream->avail_in);
        if (status == Z_STREAM_END)
            break;
        if (status != Z_OK) {
            Jsi_LogError("unzip failed: %s", stream->msg);
            cnt = -1;
            break;
        }
        if (!buf)
            break;
    }
    return cnt;
    
}

static int Jfz_FSFlushProc(Jsi_Channel chan) {
    ZVFSSIGASSERT((ZvfsChannelInfo*)chan->data, PINFO);
    Jfz_FSWriteProc(chan, NULL, 0);
    return 0;
}

static int Jfz_FSStatProc(Jsi_Interp *interp, Jsi_Value *path, Jsi_StatBuf *buf) {
    return Jsi_Stat(interp, path, buf);
}

static int Jfz_FSLstatProc(Jsi_Interp *interp, Jsi_Value *path, Jsi_StatBuf *buf) {
    return Jsi_Lstat(interp, path, buf);
}

static int Jfz_FSTellProc(Jsi_Channel chan) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    return pInfo->curPos;
}

static int Jfz_FSReadProc(Jsi_Channel chan, char *s, int size) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    Jsi_Interp *interp = pInfo->interp;
    ZVFSSIGASSERT(pInfo, PINFO);
    z_stream *stream = &pInfo->stream;
    int cnt = 0;
    *s = 0;
    int rem = (pInfo->nByte - pInfo->readSoFar);
    int bsize = sizeof(pInfo->buf);
    stream->next_out = (unsigned char*)s;
    int oaout = stream->avail_out = size;
    if (rem <= 0)
        return 0;
    
    while (cnt<size)
    {
      int status;
      if (!stream->avail_in)
      {
        // Input buffer is empty, so read more bytes from input file.
        int n = (rem<bsize? rem : bsize);

        if (pInfo->emuGzip && pInfo->readSoFar == 0) { /* Strip gzip header */
            if (Jsi_Read(interp, pInfo->chan, pInfo->buf, 10) != 10)
                goto readFail;
            n -= 10;
            const char *cp = pInfo->buf;
            if (cp[0] != '\37' || cp[1] != '\213' || cp[2] != '\10') {
                Jsi_LogError("not a gzip file");
                return -1;
            }
        }
            
        if (Jsi_Read(interp, pInfo->chan, pInfo->buf, n) != n)
        {
readFail:
            Jsi_LogError("Failed reading from input file!");
            return -1;
        }
        pInfo->curPos = (pInfo->readSoFar += n);

        stream->next_in = (unsigned char*)pInfo->buf;
        stream->avail_in = n;

        rem -= n;
      }

      status = inflate(stream, Z_SYNC_FLUSH);

      if ((status == Z_STREAM_END) || (!stream->avail_out))
      {
        uint n = oaout - stream->avail_out;
        cnt += n;
        stream->next_out = (unsigned char*)(s+cnt);
        stream->avail_out = (size-cnt);
      }

      if (status == Z_STREAM_END)
        break;
      else if (status != Z_OK)
      {
        Jsi_LogError("inflate failed: %s", stream->msg);
        return -1;
      }
    }

    return cnt;
}


static int Jfz_FSCloseProc(Jsi_Channel chan) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    ZVFSSIGASSERT(pInfo, PINFO);
    if( pInfo->zBuf ) {
        Jsi_Free(pInfo->zBuf);
    }
    int mode = chan->flags;
    if (mode & JSI_FS_READONLY) {
        if (inflateEnd(&pInfo->stream) != Z_OK)
        {
          fprintf(stderr, "inflateEnd() failed!\n");
          return -1;
        }
    } else {
        Jfz_FSWriteProc(chan, NULL, 0);
        if (deflateEnd(&pInfo->stream) != Z_OK)
        {
          fprintf(stderr, "deflateEnd() failed!\n");
          return -1;
        }
    }
    if (pInfo->chan)
        Jsi_Close(pInfo->interp, pInfo->chan);
    Jsi_Free((char*)pInfo);
    return 0;
}


static char * Jfz_FSGetsProc(Jsi_Channel chan, char *s, int size) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    char *cp = s;
    int n = 0;
    *s = 0;
    while (n<(size-1)) {
        int m = Jfz_FSReadProc(chan, pInfo->buf, 1);
        if (m <= 0)
            break;
        *cp = pInfo->buf[0];
        if (*cp == '\n') {
            n++;
            break;
        }
        n++;
        cp++;
    }
    if (n>0)
        s[n]=0;
    return s;
}

/************** Zip Archive filesystem ******************/

static int Jaz_FSCloseProc(Jsi_Channel chan) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    if( pInfo->zBuf ) {
        Jsi_Free(pInfo->zBuf);
        inflateEnd(&pInfo->stream);
    }
    Jsi_Free((char*)pInfo);
    return 0;
}

static Jsi_Channel Jaz_FSOpenProc (Jsi_Interp *interp, Jsi_Value *path, const char *mode)
{
    ZvfsFile *pFile;
    ZvfsChannelInfo *pInfo;
    Jsi_Channel chan;
    static int count = 1;
    char zName[50];
    unsigned char zBuf[50];
    if (*mode != 'r') {
        Jsi_LogError("readonly");
        return NULL;
    }
    pFile = ZvfsLookup(interp, path);
    if (!pFile)
        return NULL;

    /*if (Jsi_SetChannelOption(interp, chan, "-translation", "binary")
            || Jsi_SetChannelOption(interp, chan, "-encoding", "binary")
      ) {
    } */
    Jsi_Channel achan;
    achan = pFile->pArchive->chan;
    if (!achan) {
        Jsi_LogError("archive not found");
        return NULL;
    }
    Jsi_Seek(interp, achan, pFile->iOffset, SEEK_SET);
    Jsi_Read(interp, achan, (char*)zBuf, 30);
    if( memcmp(zBuf, "\120\113\03\04", 4) ) {
        Jsi_LogError("local header mismatch: ");
        return NULL;
    }
    pInfo = (ZvfsChannelInfo*)Jsi_Calloc(1, sizeof(*pInfo) );
    pInfo->pFile = pFile;
    pInfo->chan = achan;
    //Jsi_CreateExitHandler(vfsExit, pInfo);
    pInfo->isCompressed = INT16(zBuf, 8);
    if( pInfo->isCompressed ) {
        z_stream *stream = &pInfo->stream;
        pInfo->zBuf = (unsigned char*)Jsi_Calloc(1, COMPR_BUF_SIZE);
        stream->avail_in = 2;
        stream->next_in = pInfo->zBuf;
        pInfo->zBuf[0] = 0x78;
        pInfo->zBuf[1] = 0x01;
        inflateInit(&pInfo->stream);
    } else {
        pInfo->zBuf = 0;
    }
    pInfo->nByte = INT32(zBuf,22);
    pInfo->nByteCompr = INT32(zBuf,18);
    if (pFile && (!pInfo->nByte || !pInfo->nByteCompr)) {
        pInfo->nByte  = pFile->nByte;
        pInfo->nByteCompr  = pFile->nByteCompr;
    }
    pInfo->nData = pInfo->nByteCompr;
    pInfo->readSoFar = 0;
    Jsi_Seek(interp, achan, INT16(zBuf,26)+INT16(zBuf,28), SEEK_CUR);
    pInfo->curPos = pInfo->startOfData = Jsi_Tell(interp, achan);
    snprintf(zName, sizeof(zName), "vfs_%lx_%x",(long)(((uintptr_t)pFile)>>12),count++);
    chan = (Jsi_Channel)Jsi_Calloc(1,sizeof(Jsi_Chan));
    chan->fname = pFile->zName;
    chan->data = pInfo;
    return chan;
}

/*
** This routine does a stat() system call for a ZVFS file.
*/
static int Jaz_FSStatProc(Jsi_Interp *interp, Jsi_Value *path, Jsi_StatBuf *buf) {
    ZvfsFile *pFile;

    pFile = ZvfsLookup(jsiIntData.zvfslocal.interp, path);
    if( pFile==0 ) {
        return -1;
    }
    memset(buf, 0, sizeof(*buf));
    buf->st_ino = 0;
    buf->st_size = pFile->nByte;
    buf->st_mtime = pFile->timestamp;
    buf->st_ctime = pFile->timestamp;
    buf->st_atime = pFile->timestamp;
    if (pFile->isdir) {
        buf->st_mode = 040555;
        if (!buf->st_size)
            buf->st_size = 1;
    } else
        buf->st_mode = (0100000|pFile->permissions);
    return 0;
}

/*
** This routine does an access() system call for a ZVFS file.
*/
static int Jaz_FSAccessProc(Jsi_Interp *interp, Jsi_Value *path, int mode) {
    ZvfsFile *pFile;

    if( mode & 3 ) {
        return -1;
    }
    pFile = ZvfsLookup(jsiIntData.zvfslocal.interp, path);
    if( pFile==0 ) {
        return -1;
    }
    return 0;
}

static int Jaz_FSScandirProc(Jsi_Interp *interp, Jsi_Value *dirpath, Jsi_Dirent ***namelist,
   int (*filter)(const Jsi_Dirent *), int (*compar)(const Jsi_Dirent **, const Jsi_Dirent**))
{

    Jsi_HashEntry *pEntry;
    Jsi_HashSearch sSearch;
    int len, n = 0, rc = JSI_OK, deSpace = 0;
    Jsi_Dirent *de, dd;
    Jsi_Dirent **dep = NULL;
    *namelist = NULL;
    Jsi_DString pStr;

    const char *zp=Jsi_ValueNormalPath(interp, dirpath, &pStr);
    len = Jsi_Strlen(zp);

    for(pEntry = Jsi_HashSearchFirst(jsiIntData.zvfslocal.fileHash, &sSearch);
            pEntry && rc == JSI_OK;
            pEntry = Jsi_HashSearchNext(&sSearch)
       ) {
        ZvfsFile *pFile = (ZvfsFile*)Jsi_HashValueGet(pEntry);
        char *z = pFile->zName;
        int zlen = Jsi_Strlen(z);
        if (zlen<len || Jsi_Strncmp(z,zp,len) || z[len] != '/' || !z[len+1])
            continue;
        z = z + len + 1;
        if (Jsi_Strchr(z,'/'))
            continue;
#ifndef __WIN32
        dd.d_type = (pFile->isdir ? DT_DIR : DT_REG);
#endif
        Jsi_Strncpy(dd.d_name,  z, sizeof(dd.d_name));
        if (filter && !(filter)(&dd))
            continue;
        if (n >= deSpace) {
            deSpace += 50;
            dep = (Jsi_Dirent**)Jsi_Realloc(dep, deSpace * sizeof(Jsi_Dirent*));
        }
        de = dep[n] = (Jsi_Dirent*)Jsi_Calloc(deSpace, sizeof(Jsi_Dirent));
        *de = dd;
        n++;
    }
    if (n>0) {
#if defined(__WIN32) || JSI__MUSL==1 || defined(__FreeBSD__)
#define __compar_fn_t void*
#endif
        if (compar)
            qsort(dep, n, sizeof(Jsi_Dirent*), (__compar_fn_t)compar);
        *namelist = dep;
    }
    Jsi_DSFree(&pStr);
    return n;
}
/*
int Jaz_FSMatchInDirectoryProc (Jsi_Interp* interp, Jsi_Value *result, Jsi_Value *path,
                                Jsi_Value *pattern, int flags, Jsi_FSGlobOpts *opts)
{
    Jsi_HashEntry *pEntry;
    Jsi_HashSearch sSearch;
    int len, isreg = 0, rc = JSI_OK;
    char *zPattern = NULL;
    char *pathPtr = Jsi_ValueString(interp, path, &len);
    if (!(isreg=Jsi_ValueIsObjType(interp, pattern, JSI_OT_REGEXP)))
        zPattern = Jsi_ValueString(interp, pattern, NULL);
    const char *zp=pathPtr;
    if (!zp) return JSI_ERROR;
    if (len && zp[len-1] == '/')
        len--;
    Jsi_Obj *obj = Jsi_ObjNew(interp);
    Jsi_ValueMakeArrayObject(interp, result, obj);
    for(pEntry = Jsi_HashSearchFirst(jsiIntData.zvfslocal.fileHash, &sSearch);
            pEntry && rc == JSI_OK;
            pEntry = Jsi_HashSearchNext(&sSearch)
       ) {
        ZvfsFile *pFile = Jsi_HashValueGet(pEntry);
        char *z = pFile->zName;
        int zlen = Jsi_Strlen(z);
        if (zlen<len || Jsi_Strncmp(z,zp,len) || z[len] != '/' || !z[len+1])
            continue;
        z = z + len + 1;
        if (Jsi_Strchr(z,'/'))
            continue;
        if (isreg) {
           int ismat;
            Jsi_RegExpMatch(interp, pattern, z, &ismat);
            if (!ismat)
                continue;
        } else if (zPattern != NULL && Jsi_GlobMatch(zPattern, z, 0) == 0)
            continue;
        if (pFile->isdir) {
            if (!(flags&JSI_FILE_TYPE_DIRS))
                continue;
        } else {
            if (!(flags&JSI_FILE_TYPE_FILES))
                continue;
        }
        rc = Jsi_ObjArrayAdd(interp, obj, Jsi_ValueNewStringKey(interp, z));
    }
    return JSI_OK;
}
*/

/* Functionto check whether a path is in
* this filesystem.  This is the most
* important filesystem procedure. */
static bool Jaz_FSPathInFilesystemProc (Jsi_Interp *interp, Jsi_Value *path, void* *clientDataPtr) {
    ZvfsFile *zFile;
    if (jsiIntData.zvfslocal.archiveHash->numEntries<=0)
        return 0;
    /*if (ZvfsLookupMount(jsiIntData.zvfslocal.interp, path)==0)
        return 0;*/
    //  TODO: also check this is the archive.
    zFile = ZvfsLookup(jsiIntData.zvfslocal.interp, path);
    if (zFile!=NULL && Jsi_Strcmp(SS(path), SS(zFile->pArchive->Name)))
        return 1;
    return 0;
}

static Jsi_Value *Jaz_FSListVolumesProc (Jsi_Interp *interp) {
    Jsi_HashEntry *pEntry;     /* Hash table entry */
    Jsi_HashSearch zSearch;   /* Search all mount points */
    ZvfsArchive *pArchive;     /* The ZIP archive being mounted */
    Jsi_Value *pVols = Jsi_ValueNew(interp);
    Jsi_Obj* obj = Jsi_ObjNew(interp);
    pEntry=Jsi_HashSearchFirst(jsiIntData.zvfslocal.archiveHash,&zSearch);
    while (pEntry) {
        if ((pArchive = (ZvfsArchive*)Jsi_HashValueGet(pEntry))) {
            Jsi_ObjArrayAdd(interp, obj, pArchive->MountPoint);
        }
        pEntry=Jsi_HashSearchNext(&zSearch);
    }
    return pVols;
}

static int Jaz_FSLstatProc(Jsi_Interp *interp, Jsi_Value *path, Jsi_StatBuf *buf) {
    return Jaz_FSStatProc(interp, path, buf);
}

static int Jaz_FSFlushProc(Jsi_Channel chan) {
    return 0;
}

static int Jaz_FSTellProc(Jsi_Channel chan) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    return pInfo->curPos;
}

static int Jaz_FSEofProc(Jsi_Channel chan) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    return (pInfo->curPos >= pInfo->pFile->nByte);
}

static int Jaz_FSRewindProc(Jsi_Channel chan) {
    int rc = 0;
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    vfsSeek(pInfo, 0, SEEK_SET, &rc);
    return rc;
}

static int Jaz_FSSeekProc(Jsi_Channel chan, Jsi_Wide offset, int mode) {
    int rc = 0;
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    vfsSeek(pInfo, offset, mode, &rc);
    return rc;
}

/*static int Jaz_FSWriteProc(Jsi_Channel chan, const char *buf, int size) { return -1;}*/

static int Jaz_FSReadProc(Jsi_Channel chan, char *s, int size) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    char *cp = s;
    int n = 0, rc = 0;
    *s = 0;
    while (n<size) {
        if (pInfo->bpos >= pInfo->bsiz) {
            if (!(pInfo->bsiz = vfsInput(pInfo, pInfo->buf, sizeof(pInfo->buf), &rc))) {
                break;
            }
            pInfo->bpos = 0;
        }
        *cp = pInfo->buf[pInfo->bpos++];
        n++;
        cp++;
    }
    if (n>0 && n<size)
        s[n]=0;
    return n;
}

static int Jaz_FSGetcProc(Jsi_Channel chan) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    int rc = 0;
    if (pInfo->bpos >= pInfo->bsiz) {
        if (!(pInfo->bsiz = vfsInput(pInfo, pInfo->buf, sizeof(pInfo->buf), &rc))) {
            return 0;
        }
        pInfo->bpos = 0;
    }
    return pInfo->buf[pInfo->bpos++];
}

static int Jaz_FSUngetcProc(Jsi_Channel chan, int ch) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    if (pInfo->bpos > 0 && pInfo->buf[pInfo->bpos-1] == ch)
        pInfo->bpos--;
    else {
        /* TODO: go back */
        return -1;
    }
    return ch;
}

static char * Jaz_FSGetsProc(Jsi_Channel chan, char *s, int size) {
    ZvfsChannelInfo *pInfo = (ZvfsChannelInfo*)chan->data;
    char *cp = s;
    int n = 0, rc = 0;
    *s = 0;
    while (n<(size-1)) {
        if (pInfo->bpos >= pInfo->bsiz) {
            if ((int)pInfo->stream.total_out >= pInfo->pFile->nByte)
                return NULL;
            if ((pInfo->bsiz = vfsInput(pInfo, pInfo->buf, sizeof(pInfo->buf), &rc))<=0) {
                if (n==0)
                    return NULL;
                break;
            }
            pInfo->bpos = 0;
        }
        *cp = pInfo->buf[pInfo->bpos++];
        if (*cp == '\n') {
            n++;
            break;
        }
        n++;
        cp++;
    }
    if (n>0)
        s[n]=0;
    return s;
}



/*
** Write a 16- or 32-bit integer as little-endian into the given buffer.
*/
static void put16(char *z, int v) {
    z[0] = v & 0xff;
    z[1] = (v>>8) & 0xff;
}
static void put32(char *z, int v) {
    z[0] = v & 0xff;
    z[1] = (v>>8) & 0xff;
    z[2] = (v>>16) & 0xff;
    z[3] = (v>>24) & 0xff;
}

/*
** Translate a DOS time and date stamp into a human-readable string.
*/
static void translateDosTimeDate(char *zStr, int zsiz, int dosDate, int dosTime) {
    static const char *zMonth[] = { "nil",
                              "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
                            };

    snprintf(zStr, zsiz, "%02d-%s-%d %02d:%02d:%02d",
            dosDate & 0x1f,
            zMonth[ ((dosDate&0x1e0)>>5) ],
            ((dosDate&0xfe00)>>9) + 1980,
            (dosTime&0xf800)>>11,
            (dosTime&0x7e)>>5,
            dosTime&0x1f
           );
}


#define FN_list JSI_INFO("\
Return contents of zip directory as an array of arrays. The first element \
contains the labels, ie: \
\n\
[ 'Name', 'Special', 'Offset', 'Bytes', 'BytesCompressed' ] \
")
static Jsi_RC ZvfsListCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Channel chan;
    ZFile *pList = NULL;
    Jsi_Obj *nobj;
    Jsi_Obj *sobj;
    Jsi_Value *sval;
 
    Jsi_Value *Filename = Jsi_ValueArrayIndex(interp, args, 0);;
    chan = Jsi_Open(interp, Filename, "rb");
    if (chan==0)
        return JSI_ERROR;
    Jsi_RC rc = ZvfsReadTOC(interp, chan, &pList);
    if( rc==JSI_ERROR ) {
        deleteZFileList(pList);
        return rc;
    }
    Jsi_Close(interp, chan);
    nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    sobj = Jsi_ObjNew(interp);
    sval = Jsi_ValueMakeArrayObject(interp, NULL, sobj);
    static const char *keys[] = {"Name", "Special", "Offset", "Bytes", "BytesCompressed", 0 };
    int i;
    for (i=0; keys[i]; i++)
        Jsi_ObjArrayAdd(interp, sobj, Jsi_ValueNewStringKey(interp, keys[i]));
    Jsi_ObjArrayAdd(interp, nobj, sval);
    while( pList ) {
        sobj = Jsi_ObjNew(interp);
        sval = Jsi_ValueMakeArrayObject(interp, NULL, sobj);
        ZFile *pNext;
        char zDateTime[100];
        Jsi_ObjArrayAdd(interp, sobj, Jsi_ValueNewStringDup(interp, pList->zName));
        translateDosTimeDate(zDateTime, sizeof(zDateTime), pList->dosDate, pList->dosTime);
        Jsi_ObjArrayAdd(interp, sobj, Jsi_ValueNewStringDup(interp, zDateTime));
        Jsi_ObjArrayAdd(interp, sobj, Jsi_ValueNewNumber(interp, (Jsi_Number)pList->isSpecial));
        Jsi_ObjArrayAdd(interp, sobj, Jsi_ValueNewNumber(interp, (Jsi_Number)pList->iOffset));
        Jsi_ObjArrayAdd(interp, sobj, Jsi_ValueNewNumber(interp, (Jsi_Number)pList->nByte));
        Jsi_ObjArrayAdd(interp, sobj, Jsi_ValueNewNumber(interp, (Jsi_Number)pList->nByteCompr));
        pNext = pList->pNext;
        Jsi_Free((char*)pList);
        pList = pNext;
        Jsi_ObjArrayAdd(interp, nobj, sval);
    }
    deleteZFileList(pList);
    return JSI_OK;
}

/*
** Write a file record into a ZIP archive at the current position of
** the write cursor for channel "chan".  Add a ZFile record for the file
** to *ppList.  If an error occurs, leave an error message on interp
** and return JSI_ERROR.  Otherwise return JSI_OK.
*/

static Jsi_RC writeFile(
    Jsi_Interp *interp,     /* Leave an error message here */
    Jsi_Channel out,        /* Write the file here */
    Jsi_Channel in,         /* Read data from this file */
    Jsi_Value *Src,         /* Name of file entry */
    Jsi_Value *Dest,        /* Name of new ZIP file entry */
    ZFile **ppList          /* Put a ZFile struct for the new file here */
) {
    z_stream stream;
    ZFile *p;
    int iEndOfData;
    int nameLen;
    int skip;
    int toOut;
    char zHdr[30];
    char zInBuf[COMPR_BUF_SIZE*1];
    char zOutBuf[COMPR_BUF_SIZE*1];
    struct tm *tm;
    time_t now;
    struct stat stat;
    const char *zDest = Jsi_ValueToString(interp, Dest, NULL);
    int isdir = (in == NULL);
    nameLen = Jsi_Strlen(zDest);

    /* Create a new ZFile structure for this file.
    */
    nameLen+=isdir;
    p = newZFile(nameLen, ppList);
    Jsi_Strcpy(p->zName, zDest);
    if (isdir)
        Jsi_Strcpy(p->zName + nameLen-1, "/");
    p->isSpecial = 0;
    Jsi_Stat(interp, Src, &stat);
    now=stat.st_mtime;
    tm = localtime(&now);
    UnixTimeDate(tm, &p->dosDate, &p->dosTime);
    p->iOffset = Jsi_Tell(interp, out);
    p->nByte = 0;
    p->nByteCompr = 0;
    p->nExtra = 0;
    p->iCRC = 0;
    p->permissions = stat.st_mode;

    /* Fill in as much of the header as we know.
    */
    put32(&zHdr[0], 0x04034b50);
    put16(&zHdr[4], 0x0014);
    put16(&zHdr[6], 0);
    put16(&zHdr[8], 8);
    put16(&zHdr[10], p->dosTime);
    put16(&zHdr[12], p->dosDate);
    put16(&zHdr[26], nameLen);
    put16(&zHdr[28], 0);

    /* Write the header and filename.
    */
    Jsi_Write(interp, out, zHdr, 30);
    Jsi_Write(interp, out, p->zName, nameLen);

    /* The first two bytes that come out of the deflate compressor are
    ** some kind of header that ZIP does not use.  So skip the first two
    ** output bytes.
    */
    skip = 2;

    /* Write the compressed file.  Compute the CRC as we progress.
    */
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = 0;
    stream.avail_in = 0;
    stream.next_in = (uchar*)zInBuf;
    stream.avail_out = sizeof(zOutBuf);
    stream.next_out = (uchar*)zOutBuf;
    if (in) {
#if 1
    deflateInit(&stream, Z_DEFAULT_COMPRESSION);
#else
    {
        int i, err, WSIZE = 0x8000, windowBits, level=6;
        for (i = ((unsigned)WSIZE), windowBits = 0; i != 1; i >>= 1, ++windowBits);
        err = deflateInit2(&stream, level, Z_DEFLATED, -windowBits, 8, 0);

    }
#endif
        p->iCRC = crc32(0, 0, 0);
        while(!Jsi_Eof(interp, in) ) {
            if( stream.avail_in==0 ) {
                int amt = Jsi_Read(interp, in, zInBuf, sizeof(zInBuf));
                if( amt<=0 ) break;
                p->iCRC = crc32(p->iCRC, (uchar*)zInBuf, amt);
                stream.avail_in = amt;
                stream.next_in = (uchar*)zInBuf;
            }
            deflate(&stream, 0);
            toOut = sizeof(zOutBuf) - stream.avail_out;
            if( toOut>skip ) {
                Jsi_Write(interp, out, &zOutBuf[skip], toOut - skip);
                skip = 0;
            } else {
                skip -= toOut;
            }
            stream.avail_out = sizeof(zOutBuf);
            stream.next_out = (uchar*)zOutBuf;
        }
        do {
            stream.avail_out = sizeof(zOutBuf);
            stream.next_out = (uchar*)zOutBuf;
            deflate(&stream, Z_FINISH);
            toOut = sizeof(zOutBuf) - stream.avail_out;
            if( toOut>skip ) {
                Jsi_Write(interp, out, &zOutBuf[skip], toOut - skip);
                skip = 0;
            } else {
                skip -= toOut;
            }
        } while( stream.avail_out==0 );
        p->nByte = stream.total_in;
        p->nByteCompr = stream.total_out - 2;
        deflateEnd(&stream);
        Jsi_Flush(interp, out);
    }

    /* Remember were we are in the file.  Then go back and write the
    ** header, now that we know the compressed file size.
    */
    iEndOfData = Jsi_Tell(interp, out);
    Jsi_Seek(interp, out, p->iOffset, SEEK_SET);
    put32(&zHdr[14], p->iCRC);
    put32(&zHdr[18], p->nByteCompr);
    put32(&zHdr[22], p->nByte);
    Jsi_Write(interp, out, zHdr, 30);
    Jsi_Seek(interp, out, iEndOfData, SEEK_SET);

    /* Close the input file.
    */
    if (in)
        Jsi_Close(interp, in);

    /* Finished!
    */
    return JSI_OK;
}

/*
** The arguments are two lists of ZFile structures sorted by iOffset.
** Either or both list may be empty.  This routine merges the two
** lists together into a single sorted list and returns a pointer
** to the head of the unified list.
**
** This is part of the merge-sort algorithm.
*/
static ZFile *mergeZFiles(ZFile *pLeft, ZFile *pRight) {
    ZFile fakeHead;
    ZFile *pTail;

    pTail = &fakeHead;
    while( pLeft && pRight ) {
        ZFile *p;
        if( pLeft->iOffset <= pRight->iOffset ) {
            p = pLeft;
            pLeft = p->pNext;
        } else {
            p = pRight;
            pRight = p->pNext;
        }
        pTail->pNext = p;
        pTail = p;
    }
    if( pLeft ) {
        pTail->pNext = pLeft;
    } else if( pRight ) {
        pTail->pNext = pRight;
    } else {
        pTail->pNext = 0;
    }
    return fakeHead.pNext;
}

/*
** Sort a ZFile list so in accending order by iOffset.
*/
static ZFile *sortZFiles(ZFile *pList) {
# define NBIN 30
    int i;
    ZFile *p;
    ZFile *aBin[NBIN+1];

    for(i=0; i<=NBIN; i++) aBin[i] = 0;
    while( pList ) {
        p = pList;
        pList = p->pNext;
        p->pNext = 0;
        for(i=0; i<NBIN && aBin[i]; i++) {
            p = mergeZFiles(aBin[i],p);
            aBin[i] = 0;
        }
        aBin[i] = aBin[i] ? mergeZFiles(aBin[i], p) : p;
    }
    p = 0;
    for(i=0; i<=NBIN; i++) {
        if( aBin[i]==0 ) continue;
        p = mergeZFiles(p, aBin[i]);
    }
    return p;
}

/*
** Write a ZIP archive table of contents to the given
** channel.
*/
static void writeTOC(Jsi_Interp *interp, Jsi_Channel chan, ZFile *pList) {
    int iTocStart, iTocEnd;
    int nEntry = 0;
    int i;
    char zBuf[100];

    iTocStart = Jsi_Tell(interp, chan);
    for(; pList; pList=pList->pNext) {
        if( pList->isSpecial ) continue;
        put32(&zBuf[0], 0x02014b50);
        put16(&zBuf[4], 0x0317);
        put16(&zBuf[6], 0x0014);
        put16(&zBuf[8], 0);
        put16(&zBuf[10], pList->nByte>pList->nByteCompr ? 0x0008 : 0x0000);
        put16(&zBuf[12], pList->dosTime);
        put16(&zBuf[14], pList->dosDate);
        put32(&zBuf[16], pList->iCRC);
        put32(&zBuf[20], pList->nByteCompr);
        put32(&zBuf[24], pList->nByte);
        put16(&zBuf[28], Jsi_Strlen(pList->zName));
        put16(&zBuf[30], 0);
        put16(&zBuf[32], pList->nExtra);
        put16(&zBuf[34], 1);
        put16(&zBuf[36], 0);
        put32(&zBuf[38], pList->permissions<<16);
        put32(&zBuf[42], pList->iOffset);
        Jsi_Write(interp, chan, zBuf, 46);
        Jsi_Write(interp, chan, pList->zName, Jsi_Strlen(pList->zName));
        for(i=pList->nExtra; i>0; i-=40) {
            int toWrite = i<40 ? i : 40;
            Jsi_Write(interp, chan,"                                             ",toWrite);
        }
        nEntry++;
    }
    iTocEnd = Jsi_Tell(interp, chan);
    put32(&zBuf[0], 0x06054b50);
    put16(&zBuf[4], 0);
    put16(&zBuf[6], 0);
    put16(&zBuf[8], nEntry);
    put16(&zBuf[10], nEntry);
    put32(&zBuf[12], iTocEnd - iTocStart);
    put32(&zBuf[16], iTocStart);
    put16(&zBuf[20], 0);
    Jsi_Write(interp, chan, zBuf, 22);
    Jsi_Flush(interp, chan);
}

static const char *
GetExtension( const char *name)
{
    const char *p, *lastSep;
#ifdef __WIN32__
    lastSep = NULL;
    for (p = name; *p != '\0'; p++) {
        if (Jsi_Strchr("/\\:", *p) != NULL) {
            lastSep = p;
        }
    }
#else
    lastSep = Jsi_Strrchr(name, '/');
#endif
    p = Jsi_Strrchr(name, '.');
    if ((p != NULL) && (lastSep != NULL) && (lastSep > p)) {
        p = NULL;
    }
    return p;
}

static Jsi_RC CreateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr, int append)
{
    Jsi_Value *Archive;
    Jsi_Channel chan = 0;
    ZFile *pList = NULL, *pToc;
    Jsi_RC rc = JSI_OK;
    int i, k, create = 0, argc, alen = Jsi_ValueGetLength(interp, args);
    Jsi_Value *flist = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *prefix, *nSrc = NULL;
    const char *ext;
    Jsi_DString dStr = {};

    for (i=1; i<alen; i++) {
        flist = Jsi_ValueArrayIndex(interp, args, i);
        if (!Jsi_ValueIsArray(interp, flist)) 
            return Jsi_LogError("expected array of files");
        if (++i<alen) {
            prefix = Jsi_ValueArrayIndex(interp, args, i);
            if (Jsi_ValueIsString(interp, prefix)==0 && Jsi_ValueIsNull(interp, prefix)==0) 
                return Jsi_LogError("expected string or null");
        }
                
    }
    
    /* Open the archive and read the table of contents
    */

    Archive = Jsi_ValueArrayIndex(interp, args, 0);
    chan = Jsi_Open(interp, Archive, "rb+");
    if (chan==0 ) {
        if (append) {
            Jsi_LogError("archive must exists: %s", SS(Archive));
            rc = JSI_ERROR;
            goto doexit;
        }
        chan = Jsi_Open(interp, Archive, "wb+");
        create = 1;
    }
    if (chan==0 ) {
        rc = Jsi_LogError("archive create failed for: %s", SS(Archive));
        goto doexit;
    }
    Jsi_Seek(interp, chan, 0, SEEK_END);

    if (Jsi_Tell(interp, chan) != 0) {
        /* Null file is ok, we're creating new one. */
        Jsi_Seek(interp, chan, 0, SEEK_SET);
        rc = ZvfsReadTOC(interp, chan, &pList);
        if (rc == JSI_OK && !append) {
            rc = Jsi_LogError("archive create already has zvfs: %s", SS(Archive));
            goto doexit;
        }
        if( rc==JSI_ERROR ) {
            goto doexit;
        } else rc=JSI_OK;
    }

    /* Find TOC */
    for(pToc=pList; pToc; pToc=pToc->pNext) {
        if( pToc->isSpecial && Jsi_Strcmp(pToc->zName,"*TOC*")==0 ) break;
    }
    if( pToc ) {
        Jsi_Seek(interp, chan, pToc->iOffset, SEEK_SET);
    } else {
        Jsi_Seek(interp, chan, 0, SEEK_END);
    }
    if (pToc && !append) {
        rc = Jsi_LogError("already an archive: use zvfs.truncate() first or zvfs.append().");
        goto doexit;
    }
    Jsi_Seek(interp, chan, 0, SEEK_END);
    /* Add new files to the end of the archive. */
    for (k=1; k<alen; k++) {
        flist = Jsi_ValueArrayIndex(interp, args, k);
        argc = Jsi_ValueGetLength(interp, flist);
        prefix = NULL;
        if (++k<alen)
            prefix = Jsi_ValueArrayIndex(interp, args, k);
        
        for(i=0; rc==JSI_OK && i<argc; i++) {
            Jsi_Value *Src = Jsi_ValueArrayIndex(interp, flist, i), *fSrc = Src;
            Jsi_Channel in;
            char *fname = Jsi_ValueString(interp, Src, 0);
            
            ext = (fname?GetExtension(fname):NULL);
            if (ext && *ext == '.' && (ext[1] == 0 || (ext[1] == '.' && ext[2] == 0)))
                continue;
            if (prefix && !Jsi_ValueIsNull(interp, prefix)) {
                char *pstr = SS(prefix);
                if (*pstr) {
                    Jsi_DSSetLength(&dStr, 0);
                    Jsi_DSAppend(&dStr, pstr, "/", SS(Src), NULL);
                    fSrc = nSrc = Jsi_ValueNewStringDup(interp, Jsi_DSValue(&dStr));
                    Jsi_IncrRefCount(interp, nSrc);
                }
            }
            /* Open the file that is to be added to the ZIP archive
             */
            in = NULL;
            Jsi_StatBuf sb;
            if (Jsi_Stat(interp, fSrc, &sb) == 0 && S_ISDIR(sb.st_mode)) {
            } else {
                in = Jsi_Open(interp, fSrc, "rb");
                if( in==0 ) {
                    rc = Jsi_LogError("open failed on: %s", SS(fSrc));
                    break;
                }
            }
            if (rc == JSI_OK) {
                rc = writeFile(interp, chan, in, fSrc, Src, &pList);
                if (rc != JSI_OK)
                    Jsi_LogError("write failed on: %s", SS(Src));
            }
            if (nSrc) {
                Jsi_DecrRefCount(interp, nSrc);
                nSrc = NULL;
            }
        }
    }
    /* Write the table of contents at the end of the archive.
    */
    if (rc == JSI_OK) {
        pList = sortZFiles(pList);
        writeTOC(interp, chan, pList);
    }
doexit:
    /* Close the channel and exit */
    if (pList)
        deleteZFileList(pList);
    if (chan)
        Jsi_Close(interp, chan);
    if (nSrc)
        Jsi_DecrRefCount(interp, nSrc);
    Jsi_DSFree(&dStr);
    if (rc != JSI_OK && create)
        Jsi_Remove(interp, Archive, 0);

    return rc;
}

#define FN_create JSI_INFO("\
This command creates a zip archive and adds files to it. Files are relative \
the given 'path', or the current directory. \
If the destignation file already exist but is not an archive (eg. an executable), \
zip data is appended to the end of the file. \
If the existing file is already an archive, an error will be thrown. \
To truncate an existing archive, use zvfs.truncate(). Or use zvfs.append() instead. \
\n\
   zvfs.create('foo.zip',['main.js', 'bar.js'], 'src', ['a.html', 'css/a.css'], 'html');")

static Jsi_RC ZvfsCreateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return CreateCmd(interp, args, _this, ret, funcPtr, 0);
}

static Jsi_RC ZvfsAppendCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_LogWarn("Zvfs.append does not handle/remove duplicates");
    return CreateCmd(interp, args, _this, ret, funcPtr, 1);
}

#define FN_truncate JSI_INFO("\
Opens and scans the file to determine start of zip data \
and truncate this off the end of the file.  \
For ordinary zip archives, the resulting truncated file will be of zero length. \
If an optional bool argument can disable errors. \
In any case, the start offset of zip data (or 0) is returned.")

static Jsi_RC ZvfsTruncateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Channel chan;
    ZFile *pList = NULL;
    int rc = JSI_OK;
    bool noerror = 1; 
    int zipStart;

    Jsi_Value *Archive = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vbool = Jsi_ValueArrayIndex(interp, args, 1);
    if (vbool && Jsi_ValueGetBoolean(interp, vbool, &noerror) != JSI_OK) 
        return Jsi_LogError("expected boolean");
    chan = Jsi_Open(interp, Archive, "rb+");
    if( chan==0 ) return JSI_ERROR;

    if (Jsi_Seek(interp, chan, 0, SEEK_END) != 0) {
        Jsi_Close(interp, chan);
        Jsi_ValueMakeNumber(interp, ret, 0.0);
        return JSI_OK;
    }
    Jsi_Seek(interp, chan, 0, SEEK_SET);
    rc = ZvfsReadTOCStart(interp, chan, &pList, &zipStart);
    if( rc!=JSI_OK ) {
        deleteZFileList(pList);
        Jsi_Close(interp, chan);
        if (noerror)
            return JSI_OK;
        return Jsi_LogError("not an archive");
    }

    /* Close the channel and exit */
    deleteZFileList(pList);
    Jsi_Truncate(interp, chan, zipStart);
    Jsi_Close(interp, chan);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)zipStart);
    return JSI_OK;
}

static Jsi_RC ZvfsOffsetCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                        Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Channel chan;
    ZFile *pList = NULL;
    int rc = JSI_OK; 
    int zipStart;

    Jsi_Value *Archive = Jsi_ValueArrayIndex(interp, args, 0);
    chan = Jsi_Open(interp, Archive, "rb");
    if( chan==0 ) return JSI_ERROR;

    if (Jsi_Seek(interp, chan, 0, SEEK_END) != 0) {
        Jsi_Close(interp, chan);
        Jsi_ValueMakeNumber(interp, ret, 0.0);
        return JSI_OK;
    }
    Jsi_Seek(interp, chan, 0, SEEK_SET);
    rc = ZvfsReadTOCStart(interp, chan, &pList, &zipStart);
    if( rc!=JSI_OK ) {
        deleteZFileList(pList);
        Jsi_Close(interp, chan);
        return Jsi_LogError("not an archive");
    }

    /* Close the channel and exit */
    deleteZFileList(pList);
    Jsi_Close(interp, chan);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)zipStart);
    return JSI_OK;
}

static Jsi_Filesystem Jaz_Filesystem = {
    .typeName="jaz",
    .structureLength=sizeof(Jsi_Filesystem),
    .version=1,
    .pathInFilesystemProc=Jaz_FSPathInFilesystemProc,
    .realpathProc=0,
    .statProc=Jaz_FSStatProc,
    .lstatProc=Jaz_FSLstatProc,
    .accessProc=Jaz_FSAccessProc,
    .chmodProc=0,
    .openProc=Jaz_FSOpenProc,
    .scandirProc=Jaz_FSScandirProc,
    .readProc=Jaz_FSReadProc,
    .writeProc=0,
    .getsProc=Jaz_FSGetsProc,
    .getcProc=Jaz_FSGetcProc,
    .ungetcProc=Jaz_FSUngetcProc,
    .putsProc=0,
    
    .flushProc=Jaz_FSFlushProc,
    .seekProc=Jaz_FSSeekProc,
    .tellProc=Jaz_FSTellProc,
    .eofProc=Jaz_FSEofProc,
    .truncateProc=0,
    .rewindProc=Jaz_FSRewindProc,
    .closeProc=Jaz_FSCloseProc,
    .linkProc=0,
    .readlinkProc=0,
    .listVolumesProc=Jaz_FSListVolumesProc,
};

static Jsi_Filesystem Jfz_Filesystem = {
    .typeName="jfz",
    .structureLength=sizeof(Jsi_Filesystem),
    .version=1,
    .pathInFilesystemProc=0,
    .realpathProc=0,
    .statProc=Jfz_FSStatProc,
    .lstatProc=Jfz_FSLstatProc,
    .accessProc=0,
    .chmodProc=0,
    .openProc=Jfz_FSOpenProc,
    .scandirProc=0,
    .readProc=Jfz_FSReadProc,
    .writeProc=Jfz_FSWriteProc,
    .getsProc=Jfz_FSGetsProc,
    .getcProc=0,
    .ungetcProc=0,
    .putsProc=0,
    
    .flushProc=Jfz_FSFlushProc,
    .seekProc=0,
    .tellProc=Jfz_FSTellProc,
    .eofProc=0,
    .truncateProc=0,
    .rewindProc=0,
    .closeProc=Jfz_FSCloseProc,
    .linkProc=0,
    /*.pathInFilesystemProc=Jfz_FSPathInFilesystemProc,
    .accessProc=Jfz_FSAccessProc,
    .scandirProc=Jfz_FSScandirProc,
    .getcProc=Jfz_FSGetcProc,
    .ungetcProc=Jfz_FSUngetcProc,
    .seekProc=Jfz_FSSeekProc,
    .eofProc=Jfz_FSEofProc,
    .rewindProc=Jfz_FSRewindProc,
    .listVolumesProc=Jfz_FSListVolumesProc,*/
};

void (*Zvfs_PostInit)(Jsi_Interp *)=0;

static Jsi_RC ZvfsDeflateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_DString dStr = {};
    int sz, toOut, err, flag=0;
    char buf[JSI_BUFSIZ];
    z_stream stream = {};
    const char *str = Jsi_ValueArrayIndexToStr(interp, args, 0, &sz);
    stream.next_in = (uchar*)str;
    stream.avail_in = sz;
    stream.avail_out = sizeof(buf);
    stream.next_out = (uchar*)buf;
    err = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if (err!=Z_OK)
        goto bail;
    
    while (1) {
        err = deflate(&stream, flag);
        if (err!=Z_OK && err!= Z_STREAM_END) {
            deflateEnd(&stream);
            goto bail;
        }
        toOut = sizeof(buf) - stream.avail_out;
        if( toOut<=0)
            break;
        Jsi_DSAppendLen(&dStr, buf, toOut);
        if (err== Z_STREAM_END)
            break;
        stream.avail_out = sizeof(buf);
        stream.next_out = (uchar*)buf;
        if (!stream.avail_in)
            flag = Z_FINISH;
    }
    err = deflateEnd(&stream);
    if (err!=Z_OK)
        goto bail;
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
bail:
    Jsi_DSFree(&dStr);
    return Jsi_LogError("deflate error: %d", err);
}

static Jsi_RC ZvfsInflateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_DString dStr = {};
    int sz, toOut, err;
    char buf[JSI_BUFSIZ];
    const char *str = Jsi_ValueArrayIndexToStr(interp, args, 0, &sz);
    z_stream stream = {};
    stream.next_in = (uchar*)str;
    stream.avail_in = sz;
    stream.avail_out = sizeof(buf);
    stream.next_out = (uchar*)buf;
    err = inflateInit(&stream);
    if (err!=Z_OK)
        goto bail;
    
    while (1) {
        err = inflate(&stream, Z_SYNC_FLUSH);
        if (err!=Z_OK && err!=Z_STREAM_END) {
            inflateEnd(&stream);
            goto bail;
        }
        toOut = sizeof(buf) - stream.avail_out;
        if( toOut<=0)
            break;
        Jsi_DSAppendLen(&dStr, buf, toOut);
        stream.avail_out = sizeof(buf);
        stream.next_out = (uchar*)buf;
        if (err== Z_STREAM_END)
            break;
    }
    err = inflateEnd(&stream);
    if (err!=Z_OK)
        goto bail;
    Jsi_ValueFromDS(interp, &dStr, ret);
    return JSI_OK;
bail:
    Jsi_DSFree(&dStr);
    return Jsi_LogError("inflate error: %d", err);
}

static Jsi_CmdSpec zvfsCmds[] = {
    { "append",     ZvfsAppendCmd,      2, -1, "archive:string, filelist:array, path:string|null=void, filelist2:array=void, path2:string|null=void, ...",  .help="Like 'create()', but appends to an existing archive (with no dup checking)", .retType=(uint)JSI_TT_VOID },
    { "create",     ZvfsCreateCmd,      2, -1, "archive:string, filelist:array, path:string|null=void, filelist2:array=void, path2:string|null=void, ...",  .help="Create a zip with the given files in prefix path", .retType=(uint)JSI_TT_VOID, .flags=0, .info=FN_create },
    { "list",       ZvfsListCmd,        1,  1, "archive:string",  .help="List files in archive", .retType=(uint)JSI_TT_ARRAY, .flags=0, .info=FN_list },
    { "mount",      ZvfsMountCmd,       1,  2, "archive:string, mountdir:string=void",  .help="Mount zip on mount point", .retType=(uint)JSI_TT_STRING, .flags=0, .info=FN_mount },
    { "names",      ZvfsNamesCmd,       0,  1, "mountdir:string=void",  .help="Return all zvfs mounted zips, or archive for specified mount", .retType=(uint)JSI_TT_ARRAY, .flags=0, .info=FN_info },
    { "offset",     ZvfsOffsetCmd,      1,  1, "archive:string",  .help="Return the start offset of zip data", .retType=(uint)JSI_TT_NUMBER, .flags=0, .info=FN_truncate },
    { "stat",       ZvfsStatCmd,        1,  1, "filename:string",  .help="Return details on file in zvfs mount", .retType=(uint)JSI_TT_OBJECT, .flags=0, .info=FN_stat },
    { "truncate",   ZvfsTruncateCmd,    1,  2, "archive:string, noerror:boolean=false",  .help="Truncate zip data from archive", .retType=(uint)JSI_TT_NUMBER, .flags=0, .info=FN_truncate },
    { "unmount",    ZvfsUnmountCmd,     1,  1, "archive:string",  .help="Unmount zip", .retType=(uint)JSI_TT_VOID },
    { "deflate",    ZvfsDeflateCmd,     1,  1, "data:string",  .help="Compress string using zlib deflate", .retType=(uint)JSI_TT_STRING },
    { "inflate",    ZvfsInflateCmd,     1,  1, "data:string",  .help="Uncompress string using zlib inflate", .retType=(uint)JSI_TT_STRING },
    { NULL, 0,0,0,0, .help="Commands for mounting and accessing .zip files as a filesystem" }
};

static Jsi_RC zvfsInterpDelete(Jsi_Interp *interp, void *ptr) {
    /* Unmount filesystems. */
    return JSI_OK;
}

static Jsi_RC freeFileHashTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    ZvfsFile *pFile = (typeof(pFile))ptr;
    Jsi_Free(ptr);
    return JSI_OK;
}

static Jsi_RC freeArchiveHashTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    ZvfsArchive *pArchive = (ZvfsArchive*)ptr;
    pArchive->pEntry = NULL;
    Zvfs_Unmount(interp, pArchive->MountPoint);
    return JSI_OK;
}


static Jsi_RC Jsi_DoneZvfs(Jsi_Interp *interp) {
    /* TODO: cleanup on last interp. */
    if (interp == jsiIntData.mainInterp && jsiIntData.zvfslocal.isInit)
    {
        Jsi_FSUnregister(&Jaz_Filesystem);
        Jsi_FSUnregister(&Jfz_Filesystem);
        Jsi_HashDelete(jsiIntData.zvfslocal.archiveHash);
        Jsi_HashDelete(jsiIntData.zvfslocal.fileHash);
        jsiIntData.zvfslocal.isInit = 0;
    }
    return JSI_OK;
}

Jsi_RC Jsi_InitZvfs(Jsi_Interp *interp, int release) {
    if (release) return Jsi_DoneZvfs(interp);
    if( !jsiIntData.zvfslocal.isInit ) {
        if (Jsi_FSRegister(&Jaz_Filesystem, NULL) != JSI_OK ||
            Jsi_FSRegister(&Jfz_Filesystem, NULL) != JSI_OK)
            return JSI_ERROR;
        jsiIntData.zvfslocal.fileHash = Jsi_HashNew(interp, JSI_KEYS_STRING, freeFileHashTbl);
        jsiIntData.zvfslocal.archiveHash = Jsi_HashNew(interp, JSI_KEYS_STRING, freeArchiveHashTbl);
        jsiIntData.zvfslocal.isInit = 1;
        jsiIntData.zvfslocal.interp = interp;
    }
    Jsi_CommandCreateSpecs(interp, "Zvfs",  zvfsCmds,   NULL, 0);
    if (Zvfs_PostInit) Zvfs_PostInit(interp);
    Jsi_InterpOnDelete(interp, zvfsInterpDelete, (void*)zvfsInterpDelete);
    return JSI_OK;
}

#endif
#endif
