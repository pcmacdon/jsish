#ifndef JSI_LITE_ONLY
#ifndef JSI_OMIT_VFS

#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#ifdef __FreeBSD__
//typedef int(* __compar_fn_t) (const void *, const void *);
#endif

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

typedef enum { VFS_SIG_PINFO = 0xbeefbaed, VFS_SIG_ARCHIVE, VFS_SIG_FILE, VFS_SIG_ZFILE } VFS_Sig;
#define VFSSIGASSERT(s,n) assert((s)->sig == VFS_SIG_##n)


typedef struct VfsData {
    Jsi_Value *Name;           /* Name of the archive */
    Jsi_Value *mount;          /* Where this archive is mounted */
    Jsi_Hash *fileHash;
    const char *mountStr, *type, *version;
    //Jsi_Channel chan;
    Jsi_Value *fileList, *callback;
    Jsi_Value *file, *info, *extra, *param, *user;
    Jsi_HashEntry *hPtr;
    bool noAddDirs, writeable, noPatches;
} VfsData;

typedef struct VfsFile {
    VFS_Sig sig;
    const char *file, *fullpath;
    ssize_t size;
    time_t timestamp;
    const char *dataPtr;
    Jsi_Value *data;
    Jsi_HashEntry *hPtr;
    int isdir;
    uint32_t perms;
    VfsData* cmdPtr;
} VfsFile;

static void jsi_vfsDelete(Jsi_Interp *interp, VfsData* cmdPtr);

static VfsData* VfsLookupMount(Jsi_Interp *interp, const char *path) {
    Jsi_HashEntry *hPtr; 
    Jsi_HashSearch zSearch;
    VfsData *pArchive, *match=NULL;
    hPtr=Jsi_HashSearchFirst(interp->vfsMountHash, &zSearch);
    while (hPtr) {
        if ((pArchive = (VfsData*)Jsi_HashValueGet(hPtr))) {
            const char *zpath = pArchive->mountStr;
            int iLen = Jsi_Strlen(zpath);
            if (!Jsi_Strncmp(zpath, path, iLen) && (path[iLen]==0 || path[iLen]=='/')) {
                if (!match || Jsi_Strlen(match->mountStr)>(uint)iLen)
                    match=(VfsData*)Jsi_HashValueGet(hPtr);
            }
        }
        hPtr=Jsi_HashSearchNext(&zSearch);
    }

    return match;
}

/*
** Locate the VfsFile structure that corresponds to the file named.
** Return NULL if there is no such VfsFile.
*/
static VfsFile *jsi_VfsLookup(Jsi_Interp *interp, Jsi_Value *path) {
    char *zt;
    Jsi_HashEntry *hPtr;
    VfsFile *pFile;
    Jsi_DString dStr = {};
    int len, isdir = 0;
    zt = Jsi_ValueNormalPath(interp, path, &dStr);
    if (!zt)
        return NULL;
    len = dStr.len;
    if (len && zt[len-1] == '/') {
        isdir = 1;
        zt[len-1] = 0;
    }
    VfsData* cmdPtr = VfsLookupMount(interp, zt);
    if (!cmdPtr) return NULL;
    hPtr = Jsi_HashEntryFind(cmdPtr->fileHash, zt);
    pFile = (VfsFile*)(hPtr ? Jsi_HashValueGet(hPtr) : 0);
    Jsi_DSFree(&dStr);
    if (isdir && pFile && !pFile->isdir)
        return NULL;
    return pFile;
}

/*
** Move the file pointer so that the next byte read will be "offset".
*/
static int jsi_VfsSeek(
    Jsi_Channel chan,
    VfsFile* pInfo,    /* The file structure */
    long offset,                /* Offset to seek to */
    int mode,                   /* One of SEEK_CUR, SEEK_SET or SEEK_END */
    int *pErrorCode             /* Write the error code here */
) {
    switch( mode ) {
        case SEEK_CUR: {
            chan->resInt[0] -= offset;
            break;
        }
        case SEEK_END: {
            chan->resInt[0] = pInfo->size - offset;
            break;
        }
        case SEEK_SET: {
            chan->resInt[0] = offset;
            break;
        }
        default: {
            /* Do nothing */
            break;
        }
    }
    if (chan->resInt[0] < 0) chan->resInt[0] = 0;
    else if (chan->resInt[0] > pInfo->size) chan->resInt[0] = pInfo->size;
    return 0;
}

static int Jav_FSCloseProc(Jsi_Channel chan) {
    return 0;
}

static Jsi_Channel Jav_FSOpenProc (Jsi_Interp *interp, Jsi_Value *path, const char *mode)
{
    VfsFile *pFile;
    Jsi_Channel chan;
    pFile = jsi_VfsLookup(interp, path);
    if (!pFile)
        return NULL;
    VfsData *cmdPtr = pFile->cmdPtr;
    
    if (!cmdPtr->writeable && *mode != 'r') {
        Jsi_LogError("VFS is readonly");
        return NULL;
    }
    
    if (!pFile->dataPtr) {
        Jsi_Value *retStr = Jsi_ValueNew1(interp);
        Jsi_Value *vpargs, *vargs[3];
        vargs[0] = Jsi_ValueNewStringDup(interp, "file");
        vargs[1] = cmdPtr->mount;
        vargs[2] = Jsi_ValueNewStringKey(interp, pFile->file);
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, 3, 0));
        
        Jsi_IncrRefCount(interp, vargs[0]);
        Jsi_IncrRefCount(interp, vargs[2]);
        Jsi_IncrRefCount(interp, vpargs);
        Jsi_RC jrc = Jsi_FunctionInvoke(interp, cmdPtr->callback, vpargs, &retStr, NULL);
        Jsi_DecrRefCount(interp, vargs[0]);
        Jsi_DecrRefCount(interp, vargs[2]);
        Jsi_DecrRefCount(interp, vpargs);
        if (jrc == JSI_OK && !Jsi_ValueIsUndef(interp, retStr) && Jsi_ValueString(interp, retStr, NULL)) {
            pFile->data = retStr;
            pFile->dataPtr = Jsi_ValueString(interp, pFile->data, NULL);
            retStr = NULL;
        }
        if (retStr)
            Jsi_DecrRefCount(interp, retStr);
        if (!pFile->dataPtr)
            return NULL;
    }

    chan = (Jsi_Channel)Jsi_Calloc(1,sizeof(Jsi_Chan));
    chan->fname = pFile->fullpath;
    chan->data = pFile;
    return chan;
}

/*
** This routine does a stat() system call for a VFS file.
*/
static int Jav_FSStatProc(Jsi_Interp *interp, Jsi_Value *path, Jsi_StatBuf *buf) {
    VfsFile *pFile = jsi_VfsLookup(interp, path);
    if( pFile==0 ) {
        return -1;
    }
    memset(buf, 0, sizeof(*buf));
    if (pFile->isdir)
        buf->st_mode = 040555;
    else
        buf->st_mode = (0100000|pFile->perms);
    buf->st_ino = 0;
    buf->st_size = pFile->size;
    buf->st_mtime = pFile->timestamp;
    buf->st_ctime = pFile->timestamp;
    buf->st_atime = pFile->timestamp;
    return 0;
}

/*
** This routine does an access() system call for a VFS file.
*/
static int Jav_FSAccessProc(Jsi_Interp *interp, Jsi_Value *path, int mode) {
    VfsFile *pFile;

    if( mode & 3 ) {
        return -1;
    }
    pFile = jsi_VfsLookup(interp, path);
    if( pFile==0 ) {
        return -1;
    }
    return 0;
}

static int Jav_FSScandirProc(Jsi_Interp *interp, Jsi_Value *dirpath, Jsi_Dirent ***namelist,
   int (*filter)(const Jsi_Dirent *), int (*compar)(const Jsi_Dirent **, const Jsi_Dirent**))
{
    Jsi_HashEntry *hPtr;
    Jsi_HashSearch sSearch;
    int len, n = 0, rc = JSI_OK, deSpace = 0;
    Jsi_Dirent *de, dd;
    Jsi_Dirent **dep = NULL;
    *namelist = NULL;
    Jsi_DString pStr;

    const char *zp=Jsi_ValueNormalPath(interp, dirpath, &pStr);
    len = Jsi_Strlen(zp);
    
    VfsData *cmdPtr = VfsLookupMount(interp, zp);
    if (!cmdPtr) return -1;
    for(hPtr = Jsi_HashSearchFirst(cmdPtr->fileHash, &sSearch);
            hPtr && rc == JSI_OK;
            hPtr = Jsi_HashSearchNext(&sSearch)
       ) {
        VfsFile *pFile = (VfsFile*)Jsi_HashValueGet(hPtr);
        const char *z = pFile->fullpath;
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

/* Function to check whether a path is in this filesystem.  This is the most important filesystem procedure. */
static bool Jav_FSPathInFilesystemProc (Jsi_Interp *interp, Jsi_Value *path, void* *clientDataPtr) {
    VfsFile *pFile;
    if (!interp->vfsMountHash || interp->vfsMountHash->numEntries<=0)
        return 0;
    if (Jsi_HashEntryFind(interp->vfsMountHash, Jsi_ValueString(interp, path, NULL)))
        return 1;
    /*if (VfsLookupMount(interp, path)==0)
        return 0;*/
    //  TODO: also check this is the archive.
    pFile = jsi_VfsLookup(interp, path);
    if (pFile /*&& Jsi_Strcmp(Jsi_ValueString(interp, path, NULL), pFile->cmdPtr->mountStr)*/)
        return 1;
    return 0;
}

static Jsi_Value *Jav_FSListVolumesProc (Jsi_Interp *interp) {
    Jsi_HashEntry *hPtr;     /* Hash table entry */
    Jsi_HashSearch zSearch;   /* Search all mount points */
    VfsData *pArchive;     /* The ZIP archive being mounted */
    Jsi_Value *pVols = Jsi_ValueNew(interp);
    Jsi_Obj* obj = Jsi_ObjNew(interp);
    hPtr=Jsi_HashSearchFirst(interp->vfsMountHash, &zSearch);
    while (hPtr) {
        if ((pArchive = (VfsData*)Jsi_HashValueGet(hPtr))) {
            Jsi_ObjArrayAdd(interp, obj, pArchive->mount);
        }
        hPtr=Jsi_HashSearchNext(&zSearch);
    }
    return pVols;
}

static int Jav_FSLstatProc(Jsi_Interp *interp, Jsi_Value *path, Jsi_StatBuf *buf) {
    return Jav_FSStatProc(interp, path, buf);
}

static int Jav_FSFlushProc(Jsi_Channel chan) {
    return 0;
}

static int Jav_FSTellProc(Jsi_Channel chan) {
    return chan->resInt[0];
}

static int Jav_FSEofProc(Jsi_Channel chan) {
    VfsFile *pInfo = (VfsFile*)chan->data;
    return (chan->resInt[0] >= pInfo->size);
}

static int Jav_FSRewindProc(Jsi_Channel chan) {
    int rc = 0;
    VfsFile *pInfo = (VfsFile*)chan->data;
    jsi_VfsSeek(chan, pInfo, 0, SEEK_SET, &rc);
    return rc;
}

static int Jav_FSSeekProc(Jsi_Channel chan, Jsi_Wide offset, int mode) {
    int rc = 0;
    VfsFile *pInfo = (VfsFile*)chan->data;
    jsi_VfsSeek(chan, pInfo, offset, mode, &rc);
    return rc;
}

/*static int Jav_FSWriteProc(Jsi_Channel chan, const char *buf, int size) { return -1;}*/

static int Jav_FSReadProc(Jsi_Channel chan, char *s, int size) {
    VfsFile *pInfo = (VfsFile*)chan->data;
    int n, left = (pInfo->size-chan->resInt[0]);
    if (size <= left)
        n = size;
    else
        n = left;
    if (n>0)
        memcpy(s, pInfo->dataPtr+chan->resInt[0], n);
    chan->resInt[0] += n;
    return n;
}

static int Jav_FSWriteProc(Jsi_Channel chan, const char *buf, int size) {
    return -1;
}

static int Jav_FSGetcProc(Jsi_Channel chan) {
    VfsFile *pInfo = (VfsFile*)chan->data;
    if (chan->resInt[0]>=pInfo->size)
        return 0;
    return pInfo->dataPtr[chan->resInt[0]++];
}

static int Jav_FSUngetcProc(Jsi_Channel chan, int ch) {
    VfsFile *pInfo = (VfsFile*)chan->data;
    if (pInfo->dataPtr && chan->resInt[0] > 0 && pInfo->dataPtr[chan->resInt[0]-1] == ch)
        chan->resInt[0]--;
    else {
        /* TODO: go back */
        return -1;
    }
    return ch;
}

static char * Jav_FSGetsProc(Jsi_Channel chan, char *s, int size) {
    VfsFile *pInfo = (VfsFile*)chan->data;
    char *cp = s;
    int n = 0;
    *s = 0;
    while (n<(size-1) && chan->resInt[0]<pInfo->size) {
        *cp = pInfo->dataPtr[chan->resInt[0]++];
        if (*cp == '\n') {
            n++;
            break;
        }
        n++;
        cp++;
    }
    if (n>0 && n<size)
        s[n]=0;
    if (n<=0) return NULL;
    return s;
}


static Jsi_Filesystem Jav_Filesystem = {
    .typeName="vfs",
    .structureLength=sizeof(Jsi_Filesystem),
    .version=1,
    .pathInFilesystemProc=Jav_FSPathInFilesystemProc,
    .realpathProc=0,
    .statProc=Jav_FSStatProc,
    .lstatProc=Jav_FSLstatProc,
    .accessProc=Jav_FSAccessProc,
    .chmodProc=0,
    .openProc=Jav_FSOpenProc,
    .scandirProc=Jav_FSScandirProc,
    .readProc=Jav_FSReadProc,
    .writeProc=Jav_FSWriteProc,
    .getsProc=Jav_FSGetsProc,
    .getcProc=Jav_FSGetcProc,
    .ungetcProc=Jav_FSUngetcProc,
    .putsProc=0,
    
    .flushProc=Jav_FSFlushProc,
    .seekProc=Jav_FSSeekProc,
    .tellProc=Jav_FSTellProc,
    .eofProc=Jav_FSEofProc,
    .truncateProc=0,
    .rewindProc=Jav_FSRewindProc,
    .closeProc=Jav_FSCloseProc,
    .linkProc=0,
    .readlinkProc=0,
    .listVolumesProc=Jav_FSListVolumesProc,
};

static Jsi_OptionSpec VfsFileOptions[] = {
    JSI_OPT(STRING,     VfsFile, data,      .help="Data for file"),
    JSI_OPT(STRKEY,     VfsFile, file,      .help="File pathname", .flags=JSI_OPT_REQUIRED, .custom=0 ),
    JSI_OPT(UINT32,     VfsFile, perms,     .help="Permissions for file"),
    JSI_OPT(SSIZE_T,    VfsFile, size,      .help="Size of file", .flags=0),
    JSI_OPT(TIME_T,     VfsFile, timestamp, .help="Timestamp of file"),
    JSI_OPT_END(VfsFile, .help="Options for each VFS file in the fileList")
};

#define _DEF_VfsOptions(ff) \
    JSI_OPT(FUNC,   VfsData, callback,  .help="Function implementing VFS", .flags=ff, .custom=0, .data=(void*)"op:string, mount:string, arg:string|object|null" ), \
    JSI_OPT(OBJ,    VfsData, extra,     .help="Extra info, typically used by predefined VFS type", .flags=0), \
    JSI_OPT(BOOL,   VfsData, noAddDirs, .help="Disable auto-adding of directories; needed by File.glob", .flags=0), \
//    JSI_OPT(BOOL,   VfsData, writeable, .help="Filesystem is writeable (unsupported)", .flags=0),

static Jsi_OptionSpec VfsOptions[] = {
    _DEF_VfsOptions(0)
    JSI_OPT(STRING, VfsData, file,      .help="Fossil file to mount", .flags=0),
    JSI_OPT(ARRAY,  VfsData, fileList,  .help="List of files in the VFS (from listFunc)"),
    JSI_OPT(OBJ,    VfsData, info,      .help="Info for VFS that is stored upon init"),
    JSI_OPT(STRING, VfsData, mount,     .help="Mount point for the VFS"),
    JSI_OPT(BOOL,   VfsData, noPatches, .help="Ignore patchlevel updates: accepts only X.Y releases", .flags=0),
    JSI_OPT(OBJ,    VfsData, param,     .help="Optional 3rd argument passed to mount", .flags=0),
    JSI_OPT(STRKEY, VfsData, type,      .help="Type for predefined VFS", .flags=0),
    JSI_OPT(OBJ,    VfsData, user,      .help="User data", .flags=0),
    JSI_OPT(STRKEY, VfsData, version,   .help="Version to mount", .flags=0),
    JSI_OPT_END(VfsData, .help="Options for VFS mount")
};

static Jsi_OptionSpec VfsDefOptions[] = {
    _DEF_VfsOptions(JSI_OPT_REQUIRED)
    JSI_OPT_END(VfsData, .help="Options for VFS define")
};



static Jsi_RC freeVfsFileHashTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    VfsFile *v = (typeof(v))ptr;
    Jsi_OptionsFree(interp, VfsFileOptions, v, 0);
    Jsi_Free(ptr);
    return JSI_OK;
}

static void jsi_vfsDelete(Jsi_Interp *interp, VfsData* cmdPtr) {
    if (cmdPtr->fileHash)
        Jsi_HashDelete(cmdPtr->fileHash);
    Jsi_OptionsFree(interp, VfsOptions, cmdPtr, 0);
    if (cmdPtr->fileList)
        Jsi_DecrRefCount(interp, cmdPtr->fileList);
    Jsi_Free(cmdPtr);
}

static Jsi_RC VfsMountCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *vpargs, *oargs, *vargs[6], *param;
    uint i, n = 0;
    Jsi_Value *cmd = Jsi_NameLookup(interp, "Vfs.vmount");
    if (!cmd) return JSI_ERROR;
    vargs[n++] = Jsi_ValueNewStringKey(interp, "type");
    vargs[n++] = Jsi_ValueArrayIndex(interp, args, 0);
    vargs[n++] = Jsi_ValueNewStringKey(interp, "file");
    vargs[n++] = Jsi_ValueArrayIndex(interp, args, 1);
    param = Jsi_ValueArrayIndex(interp, args, 2);
    if (param) {
        vargs[n++] = Jsi_ValueNewStringKey(interp, "param");
        vargs[n++] = param;
    }
    oargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewObj(interp, vargs, n));
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, &oargs, 1, 0));
    
    Jsi_IncrRefCount(interp, vpargs);
    for (i=0; i<n; i++)
        Jsi_IncrRefCount(interp, vargs[i]);
    Jsi_RC rc = Jsi_FunctionInvoke(interp, cmd, vpargs, ret, NULL);
    for (i=0; i<n; i++)
        Jsi_DecrRefCount(interp, vargs[i]);
    Jsi_DecrRefCount(interp, vpargs);
    return rc;
}

static Jsi_RC VfsVmountCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_DString dStr = {};
    VfsData *cmdPtr = NULL, *cmdPtr2 = NULL, cmd = {};
    //Jsi_Value* fileList = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value* opts = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_HashEntry *hPtr = NULL, *hPtr2, *hPtr3;
    char *cp;
    char mbuf[PATH_MAX];
    bool isNew;
    Jsi_Obj *o;
    VfsFile ff = {};
    VfsFile *fptr;
    int n, i, nFile;

    if (!opts || !Jsi_ValueIsObjType(interp, opts, JSI_OT_OBJECT))
        return Jsi_LogError("arg1: expected object");
    
    if (Jsi_OptionsProcess(interp, VfsOptions, &cmd, opts, 0) < 0)
        goto bail;

    if (cmd.type) { // Lookup predefined VFS from type
        if (cmd.callback || cmd.fileList) {
            Jsi_LogError("can not use both type and callback/fileList");
            goto bail;
        }
        hPtr3 = Jsi_HashEntryFind(interp->vfsDefHash, cmd.type);
        if (!hPtr3) {
            Jsi_PkgRequire(interp, "Jsi_Vfs", 0);
            hPtr3 = Jsi_HashEntryFind(interp->vfsDefHash, cmd.type);
        }
        if (hPtr3) cmdPtr2 = (typeof(cmdPtr2))Jsi_HashValueGet(hPtr3);
        if (!cmdPtr2) {
            Jsi_LogError("unknown VFS type: %s", cmd.type);
            goto bail;
        }
        cmd.callback = cmdPtr2->callback; Jsi_IncrRefCount(interp, cmd.callback);
        cmd.noAddDirs = cmdPtr2->noAddDirs;
        cmd.extra = cmdPtr2->extra;
        if (cmd.extra)
            Jsi_IncrRefCount(interp, cmd.extra);
    }
    
    if (!cmd.mount) {
        n = 0;
        while (n++ < 1000) {
            snprintf(mbuf, sizeof(mbuf), "%s%d", JSI_VFS_DIR, n);
            hPtr = Jsi_HashEntryNew(interp->vfsMountHash, mbuf, &isNew);
            if (hPtr && isNew)
                break;
        }
        if (n>=1000) goto bail;
        cmd.mount = Jsi_ValueNewStringDup(interp, mbuf);
        Jsi_IncrRefCount(interp, cmd.mount);
    } else {
        const char *cp = Jsi_ValueString(interp, cmd.mount, NULL);
        int clen = (cp ? Jsi_Strlen(cp) : 0);
        if (clen<=0 || *cp!='/' || cp[clen-1]=='/') {
            Jsi_LogError("invalid mount");
            goto bail;
        }
        hPtr = Jsi_HashEntryNew(interp->vfsMountHash, cp, &isNew);
    }
    if (!hPtr || !isNew) {
        Jsi_LogError("Mount already found");
        goto bail;
    }
    
    cmdPtr = (typeof(cmdPtr))Jsi_Calloc(1, sizeof(*cmdPtr));
    *cmdPtr = cmd;
    cmdPtr->hPtr = hPtr;
    Jsi_HashValueSet(hPtr, cmdPtr);
    cmdPtr->mountStr = (typeof(cmdPtr->mountStr))Jsi_HashKeyGet(hPtr);

    if (!cmdPtr->info && cmdPtr->callback) {
        Jsi_Value *retStr = Jsi_ValueNew1(interp);
        Jsi_Value *vpargs, *vargs[3];
        vargs[0] = Jsi_ValueNewStringDup(interp, "init");
        vargs[1] = cmdPtr->mount;
        vargs[2] = interp->NullValue;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, 3, 0));
        
        Jsi_IncrRefCount(interp, vargs[0]);
        Jsi_IncrRefCount(interp, vpargs);
        Jsi_RC jrc = Jsi_FunctionInvoke(interp, cmdPtr->callback, vpargs, &retStr, NULL);
        Jsi_DecrRefCount(interp, vpargs);
        Jsi_DecrRefCount(interp, vargs[0]);
        if (jrc != JSI_OK) {
            Jsi_DecrRefCount(interp, retStr);
            goto bail;
        }
        cmdPtr->info = retStr;
    }
    
    if (!cmdPtr->fileList) {
        if (!cmdPtr->callback) {
            Jsi_LogError("must provide either filelist or listFunc");
            goto bail;
        }
        Jsi_Value *retStr = Jsi_ValueNew1(interp);
        Jsi_Value *vpargs, *vargs[3];
        vargs[0] = Jsi_ValueNewStringDup(interp, "list");
        vargs[1] = cmdPtr->mount;
        vargs[2] = interp->NullValue;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, 3, 0));
        
        Jsi_IncrRefCount(interp, vargs[0]);
        Jsi_IncrRefCount(interp, vpargs);
        Jsi_RC jrc = Jsi_FunctionInvoke(interp, cmdPtr->callback, vpargs, &retStr, NULL);
        Jsi_DecrRefCount(interp, vpargs);
        Jsi_DecrRefCount(interp, vargs[0]);
        if (jrc != JSI_OK) {
            Jsi_DecrRefCount(interp, retStr);
            goto bail;
        }
        cmdPtr->fileList = retStr;
        if (!Jsi_ValueIsObjType(interp, cmdPtr->fileList, JSI_OT_ARRAY)) {
            Jsi_LogError("expected filelist array");
            goto bail;
        }
    }
        
    if (!cmdPtr->fileList || !Jsi_ValueIsObjType(interp, cmdPtr->fileList, JSI_OT_ARRAY)) {
        Jsi_LogError("expected fileList object");
        goto bail;
    }
    
    cmdPtr->fileHash = Jsi_HashNew(interp, JSI_KEYS_STRING, freeVfsFileHashTbl);
    o = cmdPtr->fileList->d.obj;
    nFile = o->arrCnt;
    for (i=0; i<nFile; i++) {
        Jsi_Value *it = o->arr[i];
        if (!it || !Jsi_ValueIsObjType(interp, it, JSI_OT_OBJECT)) {
            Jsi_LogError("Expected object at index %d", i);
            goto bail;
        }
        memset(&ff, 0, sizeof(ff));
        if (Jsi_OptionsProcess(interp, VfsFileOptions, &ff, it, 0) < 0)
            goto bail;
        Jsi_DSSetLength(&dStr, 0);
        Jsi_DSPrintf(&dStr, "%s/%s", cmdPtr->mountStr, ff.file);
        ff.fullpath = Jsi_DSValue(&dStr);
        ff.cmdPtr = cmdPtr;
        hPtr2 = Jsi_HashEntryNew(cmdPtr->fileHash, ff.fullpath, &isNew);
        if (!hPtr2 || !isNew) {
            Jsi_LogError("bad or duplicate file: %s", ff.fullpath);
            goto bail;
        }
        if (S_ISDIR(ff.perms))
            ff.isdir = 1;
aadd:
        if (!cmdPtr->noAddDirs) {
            cp = Jsi_Strrchr(ff.fullpath, '/');
            if (cp != ff.fullpath && Jsi_Strcmp(ff.fullpath, cmdPtr->mountStr)) {
                *cp = 0;
                hPtr3 = Jsi_HashEntryNew(cmdPtr->fileHash, ff.fullpath, &isNew);
                if (hPtr3 && isNew) {
                    fptr = (typeof(fptr))Jsi_Calloc(1, sizeof(*fptr));
                    *fptr = ff;
                    fptr->hPtr = hPtr3;
                    fptr->fullpath = (typeof(fptr->fullpath))Jsi_HashKeyGet(hPtr3);
                    fptr->isdir = 1;
                    Jsi_HashValueSet(hPtr3, fptr);
                    if (Jsi_Strcmp(ff.fullpath, cmdPtr->mountStr))
                        goto aadd;
                }
            }
        }
        ff.fullpath = (typeof(fptr->fullpath))Jsi_HashKeyGet(hPtr2);
        fptr = (typeof(fptr))Jsi_Calloc(1, sizeof(*fptr));
        ff.hPtr = hPtr2;
        *fptr = ff;
        Jsi_HashValueSet(hPtr2, fptr);
    }
    Jsi_DSFree(&dStr);
    Jsi_ValueMakeStringDup(interp, ret, cmdPtr->mountStr);
    return JSI_OK;

bail:
    Jsi_DSFree(&dStr);
    if (hPtr)
        Jsi_HashEntryDelete(hPtr);
    else
        Jsi_OptionsFree(interp, VfsOptions, &cmd, 0);
    return JSI_ERROR;
}

static Jsi_RC VfsUnmountCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
                          Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char* name = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    if (!name || !name[0]) 
        return Jsi_LogError("Expected mount name");
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->vfsMountHash, name);
    VfsData *cmdPtr = (typeof(cmdPtr))(hPtr?Jsi_HashValueGet(hPtr):NULL);
    Jsi_RC jrc = JSI_OK;
    if (!cmdPtr) 
        return Jsi_LogError("No such mount: %s", name);
    if (cmdPtr->callback) {
        Jsi_Value *retStr = Jsi_ValueNew1(interp);
        Jsi_Value *vpargs, *vargs[3];
        vargs[0] = Jsi_ValueNewStringDup(interp, "done");
        vargs[1] = cmdPtr->mount;
        vargs[2] = interp->NullValue;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, 3, 0));
        
        Jsi_IncrRefCount(interp, vargs[0]);
        Jsi_IncrRefCount(interp, vpargs);
        jrc = Jsi_FunctionInvoke(interp, cmdPtr->callback, vpargs, &retStr, NULL);
        Jsi_DecrRefCount(interp, vpargs);
        Jsi_DecrRefCount(interp, vargs[0]);
        Jsi_DecrRefCount(interp, retStr);
    }
    Jsi_HashEntryDelete(cmdPtr->hPtr);
    return jrc;
}



static Jsi_RC VfsConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char* name = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    if (!name || !name[0]) 
        return Jsi_LogError("Expected mount name");
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->vfsMountHash, name);
    VfsData *cmdPtr = (typeof(cmdPtr))(hPtr?Jsi_HashValueGet(hPtr):NULL);
    if (!cmdPtr) 
        return Jsi_LogError("No such mount: %s", name);
    return Jsi_OptionsConf(interp, VfsOptions, cmdPtr, Jsi_ValueArrayIndex(interp, args, 1), ret, 0);
}

static Jsi_RC VfsFileConfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *name = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL),
        *file = Jsi_ValueArrayIndexToStr(interp, args, 1, NULL);
    if (!name || !name[0]) 
        return Jsi_LogError("Expected mount name");
    if (!file || !file[0]) 
        return Jsi_LogError("Expected file name");
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->vfsMountHash, name);
    VfsData *cmdPtr = (typeof(cmdPtr))(hPtr?Jsi_HashValueGet(hPtr):NULL);
    if (!cmdPtr) 
        return Jsi_LogError("No such mount: %s", name);
    Jsi_DString dStr = {};
    hPtr = Jsi_HashEntryFind(cmdPtr->fileHash, Jsi_DSPrintf(&dStr, "%s/%s", name, file) );
    Jsi_DSFree(&dStr);
    VfsFile *pInfo = NULL;
    if (hPtr)
        pInfo = (typeof(pInfo))Jsi_HashValueGet(hPtr);
    if (!pInfo)
        return Jsi_LogError("file not found: %s", file);
    return Jsi_OptionsConf(interp, VfsFileOptions, pInfo, Jsi_ValueArrayIndex(interp, args, 2), ret, 0);
}

static Jsi_RC VfsListCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return Jsi_HashKeysDump(interp, interp->vfsMountHash, ret, 0);
}

static Jsi_RC VfsTypeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    VfsData *cmdPtr = NULL, cmd = {};
    const char* type = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    Jsi_Value* opts = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_HashEntry *hPtr;
    bool isNew;

    if (!type) {
        Jsi_PkgRequire(interp, "Jsi_Vfs", 0);
        return Jsi_HashKeysDump(interp, interp->vfsDefHash, ret, 0);
    }
    
    hPtr = Jsi_HashEntryFind(interp->vfsDefHash, type);
    if (!opts || Jsi_ValueIsNull(interp, opts)) {
        if (hPtr) cmdPtr = (typeof(cmdPtr))Jsi_HashValueGet(hPtr);
        if (!cmdPtr)
            return Jsi_LogError("unknown VFS type: %s", type);
        if (!opts)
            return Jsi_OptionsConf(interp, VfsDefOptions, cmdPtr, NULL, ret, 0);
        Jsi_HashEntryDelete(hPtr);
        return JSI_OK;
    }

    if (hPtr)
        return Jsi_LogError("VFS type already exists: %s", type);

    if (!Jsi_ValueIsObjType(interp, opts, JSI_OT_OBJECT))
        return Jsi_LogError("arg2: expected object");
        
    hPtr = Jsi_HashEntryNew(interp->vfsDefHash, type, &isNew);
    if (!hPtr || !isNew)
        return Jsi_LogError("type failed: %s", type); // Can't happen.

    if (Jsi_OptionsProcess(interp, VfsDefOptions, &cmd, opts, 0) < 0)
        return JSI_ERROR;
        
    cmdPtr = (typeof(cmdPtr))Jsi_Calloc(1, sizeof(*cmdPtr));
    *cmdPtr = cmd;
    cmdPtr->hPtr = hPtr;
    Jsi_HashValueSet(hPtr, cmdPtr);
    return JSI_OK;
}

static Jsi_RC VfsExecCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *cmd = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    if (!cmd || !cmd[0]) 
        return Jsi_LogError("Expected command");
    const char *ops[] = {"fossil info ", "fossil ls ", "fossil cat ", "fossil tag ", NULL};
    const char *chs = ";&|><*?`$(){}[]!#";
    int i;
    for (i=0; chs[i]; i++)
        if (Jsi_Strchr(cmd, chs[i]))
            return Jsi_LogError("invalid char in exec: %s", cmd);
    for (i=0; ops[i]; i++)
        if (Jsi_Strncmp(ops[i], cmd, Jsi_Strlen(ops[i]))==0) break;
    if (!ops[i]) 
        return Jsi_LogError("invalid exec: %s", cmd);
    return jsi_SysExecCmd(interp, args, _this, ret, funcPtr, true);
}


static Jsi_CmdSpec vfsCmds[] = {
    { "conf",       VfsConfCmd,     1,  2, "mount:string, options:string|object|string=void",  .help="Configure mount", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=VfsOptions },
    { "exec",       VfsExecCmd,     1,  1, "cmd:string",  .help="Safe mode exec for VFS support cmds eg. fossil info/ls/cat", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=VfsFileOptions },
    { "fileconf",   VfsFileConfCmd, 2,  3, "mount:string, path:string, options:string|object=void",  .help="Configure file info which is same info as in fileList", .retType=(uint)JSI_TT_ANY, .flags=0, .info=0, .opts=VfsFileOptions },
    { "list",       VfsListCmd,     0,  0, "",  .help="Return list of all vfs mounts", .retType=(uint)JSI_TT_ARRAY, .flags=0 },
    { "mount",      VfsMountCmd,    2,  3, "type:string, file:string, param:object=void",  .help="Mount fossil file as given VFS type name, returning the mount point: frontend for vmount", .retType=(uint)JSI_TT_STRING },
    { "vmount",     VfsVmountCmd,   1,  1, "options:object=void",  .help="Create and mount a VFS, returning the mount point", .retType=(uint)JSI_TT_STRING, .flags=0, .info=0, .opts=VfsOptions },
    { "unmount",    VfsUnmountCmd,  1,  1, "mount:string",  .help="Unmount a VFS", .retType=(uint)JSI_TT_VOID },
    { "type",       VfsTypeCmd,     0,  2, "type:string=void, options:object|null=void",  .help="Set/get/delete VFS type name", .retType=0, .flags=0, .info=0, .opts=VfsDefOptions },
    { NULL, 0,0,0,0, .help="Commands for creating in memory readonly Virtual file-systems" }
};

static Jsi_RC freeVfsArchiveHashTbl(Jsi_Interp *interp, Jsi_HashEntry *hPtr, void *ptr) {
    if (!ptr) return JSI_OK;
    VfsData *cmdPtr = (VfsData*)ptr;
    cmdPtr->hPtr = NULL;
    jsi_vfsDelete(interp, cmdPtr);
    return JSI_OK;
}


static Jsi_RC Jsi_DoneVfs(Jsi_Interp *interp) {
    if (interp->vfsMountHash)
        Jsi_HashDelete(interp->vfsMountHash);
    if (interp->vfsDefHash)
        Jsi_HashDelete(interp->vfsDefHash);
    interp->vfsMountHash = NULL;
    interp->vfsDefHash = NULL;
    if (interp == jsiIntData.mainInterp)
        Jsi_FSUnregister(&Jav_Filesystem);
    return JSI_OK;
}

Jsi_RC jsi_InitVfs(Jsi_Interp *interp, int release) {
    if (release) return Jsi_DoneVfs(interp);
    if (interp == jsiIntData.mainInterp)
        Jsi_FSRegister(&Jav_Filesystem, NULL);
        
    interp->vfsMountHash = Jsi_HashNew(interp, JSI_KEYS_STRING, freeVfsArchiveHashTbl);
    interp->vfsDefHash = Jsi_HashNew(interp, JSI_KEYS_STRING, freeVfsArchiveHashTbl);
    Jsi_CommandCreateSpecs(interp, "Vfs",  vfsCmds,   NULL, 0);
    return JSI_OK;
}

#endif //JSI_OMIT_VFS
#endif //JSI_LITE_ONLY
