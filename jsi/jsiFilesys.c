#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <dirent.h>

#ifndef __WIN32
#include <pwd.h>
#include <unistd.h>
#else
#include <windef.h>
#endif
#include <limits.h>

static Jsi_Filesystem *cwdFsPtr = NULL;
static Jsi_DString pwdStr = {};
static char *jsi_pwd = NULL;

#ifndef JSI_LITE_ONLY

#define _JSI_GETFP(ch) (ch && ch->fp ? ch->fp : stdin)

//static Jsi_CmdSpec filesysCmds[];

static char* jsi_FSRealPathProc(Jsi_Interp *interp, Jsi_Value *path, char *newPath);


typedef struct FileObj {
#ifdef JSI_HAS_SIG
    jsi_Sig sig;
#endif
    Jsi_Interp *interp;
    Jsi_Channel chan;
    Jsi_Value *fname;
    char *filename;
    char *mode;
    Jsi_Obj *fobj;
    int objId;
} FileObj;

static void fileObjErase(FileObj *fo);
static int fileObjFree(Jsi_Interp *interp, void *data);
static int fileObjIsTrue(void *data);
static int fileObjEqual(void *data1, void *data2);

#ifdef __WIN32
char *get_current_dir_name() {
    static char buf[MAX_PATH];
     getcwd(buf, sizeof(buf));
     return buf;
}
#endif

static const char *jsi_TildePath(Jsi_Interp *interp, const char* path, Jsi_DString *dStr) {
    if (*path != '~')
        return path;
    const char *homedir = jsi_GetHomeDir(interp);
    if (!homedir)
        return path;
    Jsi_DSAppend(dStr, homedir, path[1] == '/' ? "" : "/", path+1, NULL);
    return Jsi_DSValue(dStr);
}

int jsi_FSScandirProc(Jsi_Interp *interp, Jsi_Value *path, Jsi_Dirent ***namelist,
   int (*filter)(const Jsi_Dirent *), int (*compar)(const Jsi_Dirent **, const Jsi_Dirent**))
{
    const char *dirname = Jsi_ValueToString(interp, path, NULL);
    Jsi_DString dStr = {};
    if (*dirname == '~')
        dirname = jsi_TildePath(interp, dirname, &dStr);
    int rc = scandir(dirname, namelist, filter, compar);
    Jsi_DSFree(&dStr);
    return rc;
}

static int jsi_FSCreateDirectoryProc(Jsi_Interp *interp, Jsi_Value* path) {
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    Jsi_DString dStr = {};
    int rc;
    if (*pathPtr == '~')
        pathPtr = jsi_TildePath(interp, pathPtr, &dStr);

#ifdef __WIN32
    rc = mkdir(pathPtr);
#else
    rc = mkdir(pathPtr, 0666);
#endif
    Jsi_DSFree(&dStr);
    return rc;
}

static int jsi_FSRenameProc(Jsi_Interp *interp, Jsi_Value *src, Jsi_Value *dest) {
    const char *zSrc = Jsi_ValueToString(interp, src, NULL);
    const char *zDest = Jsi_ValueToString(interp, dest, NULL);
    Jsi_DString dStr = {}, eStr = {};
    if (*zSrc == '~')
        zSrc = jsi_TildePath(interp, zSrc, &dStr);
    if (*zDest == '~')
        zDest = jsi_TildePath(interp, zDest, &eStr);
    int rc = rename(zSrc, zDest);
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&eStr);
    return rc;
}

static Jsi_Value * jsi_FSListVolumesProc(Jsi_Interp *interp) {return 0;}

static int jsi_FSRemoveProc(Jsi_Interp *interp, Jsi_Value* path, int flags) {
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    Jsi_DString dStr = {};
    if (*pathPtr == '~')
        pathPtr = jsi_TildePath(interp, pathPtr, &dStr);
    int rc = remove(pathPtr);
    Jsi_DSFree(&dStr);
    return rc;
}

static int jsi_FSPathInFilesystemProc(Jsi_Interp *interp, Jsi_Value* path,void **clientDataPtr) {return 1;}

static int jsi_FSAccessProc(Jsi_Interp *interp, Jsi_Value* path, int mode) {
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    Jsi_DString dStr = {};
    if (*pathPtr == '~')
        pathPtr = jsi_TildePath(interp, pathPtr, &dStr);
    int rc = access(pathPtr, mode);
    Jsi_DSFree(&dStr);
    return rc;
}

static int jsi_FSChmodProc(Jsi_Interp *interp, Jsi_Value* path, int mode) {
#ifdef __WIN32
    return -1;
#else
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    Jsi_DString dStr = {};
    if (*pathPtr == '~')
        pathPtr = jsi_TildePath(interp, pathPtr, &dStr);
    int rc = chmod(pathPtr, mode);
    Jsi_DSFree(&dStr);
    return rc;
#endif
}

static int jsi_FSStatProc(Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf) {
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    Jsi_DString dStr = {};
    if (*pathPtr == '~')
        pathPtr = jsi_TildePath(interp, pathPtr, &dStr);
    int rc = stat(pathPtr, buf);
    Jsi_DSFree(&dStr);
    return rc;
}

static int jsi_FSLstatProc(Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf) {
#ifdef __WIN32
    return jsi_FSStatProc(interp, path, buf);
#else
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    Jsi_DString dStr = {};
    if (*pathPtr == '~')
        pathPtr = jsi_TildePath(interp, pathPtr, &dStr);
    int rc = lstat(pathPtr, buf);
    Jsi_DSFree(&dStr);
    return rc;
#endif
}
static int jsi_FSFlushProc(Jsi_Channel chan) { return fflush(_JSI_GETFP(chan));}
static int jsi_FSTellProc(Jsi_Channel chan) { return ftell(_JSI_GETFP(chan));}
static int jsi_FSEofProc(Jsi_Channel chan) { return feof(_JSI_GETFP(chan));}
static int jsi_FSTruncateProc(Jsi_Channel chan, unsigned int len) { return ftruncate(fileno(_JSI_GETFP(chan)), len);}
static int jsi_FSRewindProc(Jsi_Channel chan) { rewind(_JSI_GETFP(chan)); return 0;}
static int jsi_FSCloseProc(Jsi_Channel chan) { return fclose(_JSI_GETFP(chan));}
static int jsi_FSSeekProc(Jsi_Channel chan, Jsi_Wide offset, int mode) { return fseek(_JSI_GETFP(chan), offset, mode);}

static int jsi_FSWriteProc(Jsi_Channel chan, const char *buf, int size) {
    Jsi_Interp *interp = chan->interp;
    Jsi_MutexUnlock(interp, interp->Mutex);
    int rc = fwrite(buf, 1, size, _JSI_GETFP(chan));
    if (Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
        Jsi_LogBug("could not get mutex in write");
    }
    return rc;
}

static int jsi_FSReadProc(Jsi_Channel chan, char *buf, int size) {
    Jsi_Interp *interp = chan->interp;
    Jsi_MutexUnlock(interp, interp->Mutex);
    int rc = fread(buf, 1, size, _JSI_GETFP(chan));
    if (Jsi_MutexLock(interp, interp->Mutex) != JSI_OK) {
        Jsi_LogBug("could not get mutex in read");
    }
    return rc;
}
#ifdef __WIN32
#define jsi_FSLinkProc NULL
#define jsi_FSReadlinkProc NULL
#else //__WIN32
static int jsi_FSLinkProc(Jsi_Interp *interp, Jsi_Value* path, Jsi_Value *toPath, int linkType) {
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    const char *toPtr = Jsi_ValueToString(interp, toPath, NULL);
    Jsi_DString dStr = {}, eStr = {};
    int rc;
    if (*pathPtr == '~')
        pathPtr = jsi_TildePath(interp, pathPtr, &dStr);
    if (*toPtr == '~')
        toPtr = jsi_TildePath(interp, toPtr, &eStr);
    if (linkType != 0)
        rc = link(pathPtr, toPtr);
    else
        rc = symlink(pathPtr, toPtr);
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&eStr);
    return rc;
}

static int jsi_FSReadlinkProc(Jsi_Interp *interp, Jsi_Value *path, char *buf, int size) {
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    Jsi_DString dStr = {};
    if (*pathPtr == '~')
        pathPtr = jsi_TildePath(interp, pathPtr, &dStr);
    int rc = readlink(pathPtr, buf, size);
    Jsi_DSFree(&dStr);
    return rc;
}
#endif // __WIN32

static int jsi_FSGetcProc(Jsi_Channel chan) {
    return fgetc(_JSI_GETFP(chan));
}

static int jsi_FSUngetcProc(Jsi_Channel chan, int ch) {
    return ungetc(ch, _JSI_GETFP(chan));
}

static char * jsi_FSGetsProc(Jsi_Channel chan, char *s, int size) {
    return fgets(s, size, _JSI_GETFP(chan));
}

/* Not used as Jsi_Open already handles native. */
Jsi_Channel jsi_FSOpenProc(Jsi_Interp *interp, Jsi_Value *file, const char *modeString)
{
    return Jsi_Open(interp, file, modeString);
}

typedef struct FSList {
    Jsi_Filesystem* fsPtr;
    void *data;
    struct FSList *next;
} FSList;

static FSList *jsiFSList = NULL;
static Jsi_Chan StdChans[3];

static Jsi_Filesystem jsiFilesystem = {
    .typeName="native",
    .structureLength=sizeof(Jsi_Filesystem),
    .version=1,
    .pathInFilesystemProc=jsi_FSPathInFilesystemProc,
    .realpathProc=jsi_FSRealPathProc,
    .statProc=jsi_FSStatProc,
    .lstatProc=jsi_FSLstatProc,
    .accessProc=jsi_FSAccessProc,
    .chmodProc=jsi_FSChmodProc,
    .openProc=jsi_FSOpenProc,
    .scandirProc=jsi_FSScandirProc,
    .readProc=jsi_FSReadProc,
    .writeProc=jsi_FSWriteProc,
    .getsProc=jsi_FSGetsProc,
    .getcProc=jsi_FSGetcProc,
    .ungetcProc=jsi_FSUngetcProc,
    .flushProc=jsi_FSFlushProc,
    .seekProc=jsi_FSSeekProc,
    .tellProc=jsi_FSTellProc,
    .eofProc=jsi_FSEofProc,
    .truncateProc=jsi_FSTruncateProc,
    .rewindProc=jsi_FSRewindProc,
    .closeProc=jsi_FSCloseProc,
    .linkProc=jsi_FSLinkProc,
    .readlinkProc=jsi_FSReadlinkProc,
    .listVolumesProc=jsi_FSListVolumesProc,
    .createDirectoryProc=jsi_FSCreateDirectoryProc,
    .removeProc=jsi_FSRemoveProc,
    .renameProc=jsi_FSRenameProc,
};

Jsi_Channel Jsi_GetStdChannel(Jsi_Interp *interp, int id) {
    if (id<0 || id>2)
        return NULL;
    return StdChans+id;
}

int Jsi_FSRegister(Jsi_Filesystem *fsPtr, void *data) {
    FSList *fsl = (FSList *)Jsi_Calloc(1, sizeof(*fsl));
    fsl->fsPtr = fsPtr;
    fsl->data = data;
    fsl->next = jsiFSList;
    jsiFSList = fsl;
    return JSI_OK;
}

int Jsi_FSUnregister(Jsi_Filesystem *fsPtr) {
    FSList *fsl = jsiFSList, *flast = NULL;
    while (fsl) {
        if (fsl->fsPtr == fsPtr) {
            if (flast)
                flast->next = fsl->next;
            else
                jsiFSList = fsl->next;
            Jsi_Free(fsl);
            break;
        }
        flast = fsl;
        fsl = fsl->next;
    }
    return JSI_OK;
}

Jsi_Filesystem* Jsi_FilesystemForPath(Jsi_Interp *interp, Jsi_Value* path, void**clientDataPtr) {
    FSList *fsl = jsiFSList;
    if (!fsl) return NULL;
    clientDataPtr = NULL;
    const char *pathStr = Jsi_ValueToString(interp, path, NULL);
    if (pathStr[0] == '~')
        return &jsiFilesystem;
    if (pathStr[0] == '.' && pathStr[1] == 0)
        return (cwdFsPtr ? cwdFsPtr : &jsiFilesystem);
    while (1) {
        if (fsl->fsPtr->pathInFilesystemProc && fsl->fsPtr->pathInFilesystemProc(interp, path, clientDataPtr))
            break;
        if (!fsl->next)
            break;
        fsl = fsl->next;
    }
    return (fsl ? fsl->fsPtr : &jsiFilesystem);
}
int Jsi_Readlink(Jsi_Interp *interp, Jsi_Value* path, char *ret, int len) {
#ifdef __WIN32
    return -1;
#else
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == NULL || !fsPtr->readlinkProc) return -1;
    return fsPtr->readlinkProc(interp, path, ret, len);
#endif
}

int Jsi_Stat(Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf) {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == NULL || !fsPtr->statProc) return -1;
    return fsPtr->statProc(interp, path, buf);
}

int Jsi_Link(Jsi_Interp *interp, Jsi_Value* src, Jsi_Value *dest, int typ) {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, src, &data);
    if (fsPtr == NULL || !fsPtr->linkProc) return -1;
    return fsPtr->linkProc(interp, src, dest, typ);
}

int Jsi_Lstat(Jsi_Interp *interp, Jsi_Value* path, Jsi_StatBuf *buf) {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == NULL || !fsPtr->lstatProc) return -1;
    return fsPtr->lstatProc(interp, path, buf);
}

int Jsi_Chmod(Jsi_Interp *interp, Jsi_Value* path, int mode) {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == NULL || !fsPtr->chmodProc) return -1;
    return fsPtr->chmodProc(interp, path, mode);
}
int Jsi_Access(Jsi_Interp *interp, Jsi_Value* path, int mode) {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == NULL || !fsPtr->accessProc) return -1;
    return fsPtr->accessProc(interp, path, mode);
}
int Jsi_Remove(Jsi_Interp *interp, Jsi_Value* path, int flags) {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == NULL || !fsPtr->removeProc) return -1;
    return fsPtr->removeProc(interp, path, flags);
}
int Jsi_Rename(Jsi_Interp *interp, Jsi_Value *src, Jsi_Value *dst) {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, src, &data);
    if (fsPtr != Jsi_FilesystemForPath(interp, src, &data)) return -1;
    if (fsPtr == NULL || !fsPtr->renameProc) return -1;
    return fsPtr->renameProc(interp, src,dst);
}

int Jsi_Scandir(Jsi_Interp *interp, Jsi_Value* path, Jsi_Dirent ***namelist,
 int (*filter)(const Jsi_Dirent *), int (*compar)(const Jsi_Dirent **, const Jsi_Dirent**)) 
 {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == NULL || !fsPtr->scandirProc) return -1;
    return fsPtr->scandirProc(interp, path, namelist, filter, compar);
}

int Jsi_IsNative(Jsi_Interp *interp, Jsi_Value *file) {
    void *data;
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, file, &data);
    if (fsPtr && fsPtr == &jsiFilesystem)
        return 1;
    else
        return 0;
}

static Jsi_Filesystem *jsi_FilesysFind(const char *name)
{
    FSList *fsPtr = jsiFSList;
    while (fsPtr != NULL) {
        if (!Jsi_Strcmp(fsPtr->fsPtr->typeName, name))
            return fsPtr->fsPtr;
        fsPtr = fsPtr->next;
    }
    return NULL;
}

Jsi_Channel Jsi_Open(Jsi_Interp *interp, Jsi_Value *file, const char *modeString)
{
    /* Find fsys, and use open there. */
    Jsi_Filesystem *fsPtr;
    Jsi_Chan *ch = NULL;
    void *data;
    const char *fileName = Jsi_ValueToString(interp, file, NULL);
    const char *s = modeString;
    char Mode[sizeof(ch->modes)];
    Jsi_StatBuf sb;
    Jsi_Value *path = NULL;
    int n, i, mode = 0, rc, writ;
    if (!s)
        s = "r";
    if (Jsi_Strlen(s) >= sizeof(ch->modes)) {
        Jsi_LogError("mode too long: %s", s);
        return NULL;
    }
    if (Jsi_Strchr(s, 'z') || Jsi_Strchr(s, 'Z')) {
        Jsi_Filesystem *fsPtr = jsi_FilesysFind("jfz");
        if (!fsPtr) {
            Jsi_LogError("compressed files unsupported");
            return NULL;
        }
        ch = fsPtr->openProc(interp, file, s);
        if (!ch)
            return NULL;
        Jsi_Chan *nch = (Jsi_Chan *)Jsi_Calloc(1,sizeof(*nch));
        *nch = *ch;
        nch->fsPtr = fsPtr;
        return nch;
    }
    for (i=0, n = 0; s[i]; i++) {
        switch (s[i]) {
            case '+': break;
            case 'b': break;
            case 'r': if (!strchr(s,'+')) mode |= JSI_FS_READONLY; break;
            case 'a':
            case 'w': if (!strchr(s,'+')) mode |= JSI_FS_WRITEONLY; break;
            default: Jsi_LogError("unknown mode char: %c", s[i]); return NULL;
        }
        Mode[n++] = s[i];
    }
    Mode[n] = 0;
    /* Fixup files in the ~ dir */
    rc = Jsi_Stat(interp, file,&sb);
    if ((rc != 0 || *fileName == '~') && (fileName = Jsi_FileRealpath(interp, file, NULL))) {
        path = Jsi_ValueNewString(interp, fileName, -1);
        Jsi_IncrRefCount(interp, path);
        rc = Jsi_Stat(interp, path, &sb);
        if (rc == 0)
            file = path;
    }

    if (rc == 0 &&  sb.st_mode & S_IFDIR )
    {
        Jsi_LogError("can not open a directory");
        goto done;
    }
    fsPtr = Jsi_FilesystemForPath(interp, file, &data);
    writ = (strchr(s,'w') || strchr(s,'a') || strchr(s,'+'));
    if (interp->isSafe && Jsi_InterpAccess(interp, file, writ) != JSI_OK) {
        Jsi_LogError("%s access denied", writ?"write":"read");
        goto done;
    }
    if (fsPtr && fsPtr != &jsiFilesystem) {
        ch = fsPtr->openProc(interp, file, Mode);
        if (ch)
            ch->isNative = 0;
    } else {
        FILE *fp = fopen(fileName, Mode);
        fsPtr = &jsiFilesystem;
        if (!fp)
            goto done;
        ch = (Jsi_Chan *)Jsi_Calloc(1,sizeof(*ch));
        ch->fp = fp;
        ch->isNative = 1;
    }
    if (ch) {
        ch->flags |= mode; // + (zLevel<<24);
        Jsi_Strcpy(ch->modes, s);
        ch->interp = interp;
        ch->fsPtr = fsPtr;
        ch->fname = fileName;
    }
done:
    if (path)
        Jsi_DecrRefCount(interp, path);
    return ch;
}

int Jsi_SetChannelOption(Jsi_Interp *interp, Jsi_Channel chan, const char *optionName,
    const char *newValue) {return JSI_OK;}
    
Jsi_Wide Jsi_Seek(Jsi_Channel chan, Jsi_Wide offset, int mode) {
    if (!chan->fsPtr->seekProc) return -1;
    return chan->fsPtr->seekProc(chan, offset, mode);
}
Jsi_Wide Jsi_Tell(Jsi_Channel chan) {
    if (!chan->fsPtr->tellProc) return -1;
    return chan->fsPtr->tellProc(chan);
}
int Jsi_Eof(Jsi_Channel chan) {
    if (!chan->fsPtr->eofProc) return -1;
    return chan->fsPtr->eofProc(chan);
}
Jsi_Wide Jsi_Rewind(Jsi_Channel chan) {
    if (!chan->fsPtr->rewindProc) return -1;
    return chan->fsPtr->rewindProc(chan);
}

/*Jsi_StatBuf* Jsi_AllocStatBuf(void) {return 0;}*/

int Jsi_Read(Jsi_Channel chan, char *bufPtr, int toRead) {
    if (!chan->fsPtr->readProc) return -1;
    return chan->fsPtr->readProc(chan, bufPtr, toRead);
}
int Jsi_Write(Jsi_Channel chan, const char *bufPtr, int slen) {
    if (!chan->fsPtr->writeProc) return -1;
    return chan->fsPtr->writeProc(chan, bufPtr, slen);
}
int Jsi_Truncate(Jsi_Channel chan, unsigned int len) {
    if (!chan->fsPtr->truncateProc) return -1;
    return chan->fsPtr->truncateProc(chan, len);
}

int Jsi_Close(Jsi_Channel chan) {
    if (!chan->fsPtr->closeProc) return -1;
    if (chan->flags&JSI_FS_NOCLOSE) return -1;
    int rc = chan->fsPtr->closeProc(chan);
    if (rc == 0)
        Jsi_Free(chan);
    return rc;
}
int Jsi_Flush(Jsi_Channel chan) {
    if (!chan->fsPtr->flushProc) return -1;
    return chan->fsPtr->flushProc(chan);
}

int Jsi_Getc(Jsi_Channel chan) {
    if (!chan->fsPtr->getcProc) return -1;
    return chan->fsPtr->getcProc(chan);
}

int Jsi_Ungetc(Jsi_Channel chan, int ch) {
    if (!chan->fsPtr->ungetcProc) return -1;
    return chan->fsPtr->ungetcProc(chan, ch);
}

char * Jsi_Gets(Jsi_Channel chan, char *s, int size) {
    if (!chan->fsPtr->getsProc) return NULL;
    return chan->fsPtr->getsProc(chan, s, size);
}

int Jsi_Chdir(Jsi_Interp *interp, Jsi_Value* path) {
    void *data;
    int rc = 0;
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    if (interp->isSafe && Jsi_InterpAccess(interp, path, 0) != JSI_OK) {
            Jsi_LogError("read access denied");
            return JSI_ERROR;
    }
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == &jsiFilesystem) {
        rc = chdir(pathPtr);
        if (rc < 0)
            return -1;
        /* If change out of native fs, GetCwd will use pwdStr */
        fsPtr = NULL;
    }
    Jsi_DSSetLength(&pwdStr, 0);
    Jsi_DSAppendLen(&pwdStr, pathPtr, -1);
    cwdFsPtr = fsPtr;
    jsi_pwd = fsPtr ? Jsi_DSValue(&pwdStr) : NULL;
    return rc;
}

char* Jsi_ValueNormalPath(Jsi_Interp *interp, Jsi_Value *file, Jsi_DString *dStr) {
    return Jsi_NormalPath(interp, Jsi_ValueString(interp, file, NULL), dStr);
}

char *Jsi_FileRealpath(Jsi_Interp *interp, Jsi_Value *spath, char *newname)
{
    char *path = Jsi_ValueString(interp, spath, 0);
    if (!path) return NULL;
    return Jsi_FileRealpathStr(interp, path, newname);
}

static char* jsi_FSRealPathProc(Jsi_Interp *interp, Jsi_Value *src, char *newPath) {
    return Jsi_FileRealpath(interp, src, newPath);
}

char *Jsi_Realpath(Jsi_Interp *interp, Jsi_Value *src, char *newname)
{
    /* TODO: resolve pwd first. */
    void *data;
    Jsi_Filesystem *fsPtr;
    Jsi_DString dStr;
    const char *npath = Jsi_ValueNormalPath(interp, src, &dStr);
    Jsi_Value *tPtr = Jsi_ValueNew1(interp);
    Jsi_ValueMakeStringDup(interp, &tPtr, npath);
    fsPtr = Jsi_FilesystemForPath(interp, tPtr, &data);
    Jsi_DSFree(&dStr);
    Jsi_DecrRefCount(interp, tPtr);
    if (fsPtr == NULL || !fsPtr->realpathProc) return NULL;
    return fsPtr->realpathProc(interp, src, newname);
}


static void fileObjErase(FileObj *fo)
{
    if (fo->filename) {
        Jsi_Close(fo->chan);
        Jsi_Free(fo->filename);
        Jsi_DecrRefCount(fo->interp, fo->fname);
        Jsi_Free(fo->mode);
    }
    fo->filename = NULL;
}

static int fileObjFree(Jsi_Interp *interp, void *data)
{
    FileObj *fo = (FileObj *)data;
    SIGASSERT(fo,FILEOBJ);
    fileObjErase(fo);
    Jsi_Free(fo);
    return JSI_OK;
}

static int fileObjIsTrue(void *data)
{
    FileObj *fo = (FileObj *)data;
    SIGASSERT(fo,FILEOBJ);
    if (!fo->filename) return JSI_OK;
    else return 1;
}

static int fileObjEqual(void *data1, void *data2)
{
    return (data1 == data2);
}

static int try_open_file(Jsi_Interp *interp, FileObj *udf, Jsi_Value *args)
{
    int ret = JSI_ERROR;
    fileObjErase(udf);
    // TODO: stdin, stdout, stderr, etc.
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    if (fname && Jsi_ValueIsString(interp, fname)) {
        Jsi_Value *vmode = Jsi_ValueArrayIndex(interp, args, 1);
        const char *mode = NULL;
        const char *fstr = Jsi_ValueString(interp, fname, NULL);
        if (vmode && Jsi_ValueIsString(interp,vmode)) {
            mode = Jsi_ValueString(interp, vmode, NULL);
        }
        if (interp->isSafe && Jsi_InterpAccess(interp, fname, (mode && (strchr(mode,'w')||strchr(mode,'+')))) != JSI_OK)
            return JSI_ERROR;
        char *rmode = Jsi_Strdup(mode ? mode : "r");
        Jsi_Channel chan = Jsi_Open(interp, fname, rmode);
        if (chan) {
            udf->chan = chan;
            udf->fname = fname;
            udf->interp = interp;
            Jsi_IncrRefCount(interp, fname);
            udf->filename = Jsi_Strdup(fstr);
            udf->mode = Jsi_Strdup(rmode);
            ret = JSI_OK;
        }
        Jsi_Free(rmode);
    }
    return ret;
}

#define UdfGet(udf, _this, funcPtr) \
   FileObj *udf = (FileObj *)Jsi_UserObjGetData(interp, _this, funcPtr); \
    if (!udf) { \
        Jsi_LogError("File.%s called with non-file object\n", funcPtr->cmdSpec->name); \
        return JSI_ERROR; \
    }

static int FilesysOpenCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    if (try_open_file(interp, udf, args) != JSI_OK) {
        Jsi_ValueMakeBool(interp, ret, 0);
    }
    Jsi_ValueMakeBool(interp, ret, 1);
    return JSI_OK;
}

static int FilesysCloseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    fileObjErase(udf);
    Jsi_ValueMakeBool(interp, ret, 1);
    return JSI_OK;
}

static int FilesysGetsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int len;
    UdfGet(udf, _this, funcPtr);
    if (!udf->filename) {
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_OK;
    }
    char buf[BUFSIZ>8196?BUFSIZ:8196];
    if (!Jsi_Gets(udf->chan, buf, sizeof(buf))) {
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_OK;
    }
    buf[sizeof(buf)-1] = 0;
    len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n')
        buf[len-1] = 0;
    Jsi_ValueMakeString(interp, ret, Jsi_Strdup(buf));
    return JSI_OK;
}


static int FilesysModeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    if (udf->mode)
        Jsi_ValueMakeStringKey(interp, ret, udf->mode);
    return JSI_OK;
}

static int FilesysFilenameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    if (udf->filename)
        Jsi_ValueMakeString(interp, ret, Jsi_Strdup(udf->filename));
    return JSI_OK;
}

static int FilesysReadCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int  sum = 0, n;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    UdfGet(udf, _this, funcPtr);
    char buf[BUFSIZ];
    int argc = Jsi_ValueGetLength(interp, args);
    int nsiz = -1, cnt = 0, rsiz;
    
    if (!udf->filename) {
        goto bail;
    }
    if (argc>0 && Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, 0), &nsiz) != JSI_OK)
        goto bail;
    while (cnt++ < MAX_LOOP_COUNT) {
        /* TODO: limit max size. */
        rsiz = sizeof(buf);
        if (nsiz>0 && ((sum+n)>nsiz))
            rsiz = (nsiz-sum);
        if ((n = Jsi_Read(udf->chan, buf, rsiz)) <= 0)
            break;
        Jsi_DSAppendLen(&dStr, buf, n);
        sum += n;
        if (nsiz>=0 && sum >=nsiz)
            break;
    }
    Jsi_ValueMakeDStringObject(interp, ret, &dStr);
    return JSI_OK;
    
bail:
    Jsi_DSFree(&dStr);
    Jsi_ValueMakeUndef(interp, ret);
    return JSI_OK;
}


static int FilesysSeekCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    static const char *posStr[] = { "set", "cur", "end", NULL };
    enum { W_SET, W_CUR, W_END };
    UdfGet(udf, _this, funcPtr);
    Jsi_Value *vPos = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vWhence = Jsi_ValueArrayIndex(interp, args, 1);
    int mode = 0, p;
    Jsi_Wide pos;
    Jsi_Number num;
    if (Jsi_ValueGetNumber(interp, vPos, &num) != JSI_OK)
        return JSI_ERROR;
    if (Jsi_ValueGetIndex(interp, vWhence, posStr, "position", 0, &p) != JSI_OK)
        return JSI_ERROR;
    switch (p) {
        case W_SET: mode = SEEK_SET; break;
        case W_CUR: mode = SEEK_CUR; break;
        case W_END: mode = SEEK_END; break;
    }
    pos = (Jsi_Wide)num;
    pos = Jsi_Seek(udf->chan, pos, mode);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)pos);
    return JSI_OK;
}

static int FilesysTruncateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    Jsi_Value *vPos = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Number num;
    if (Jsi_ValueGetNumber(interp, vPos, &num) != JSI_OK)
        return JSI_ERROR;
    num = (Jsi_Number)Jsi_Truncate(udf->chan, (unsigned int)num);
    Jsi_ValueMakeNumber(interp, ret, num);
    return JSI_OK;
}

static int FilesysStatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    return jsi_FileStatCmd(interp, udf->fname, _this, ret, funcPtr, 0);
}

static int FilesysLstatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    return jsi_FileStatCmd(interp, udf->fname, _this, ret, funcPtr, 1);
}

static int FilesysTellCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    Jsi_Wide pos = Jsi_Tell(udf->chan);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)pos);
    return JSI_OK;
}

static int FilesysFlushCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    int pos = Jsi_Flush(udf->chan);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)pos);
    return JSI_OK;
}

static int FilesysWriteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int  sum = 0, n, m;
    UdfGet(udf, _this, funcPtr);
    char *buf = Jsi_ValueArrayIndexToStr(interp, args, 0, &m);

    if (!udf->filename) {
        goto bail;
    }
    while (m > 0 && sum < MAX_LOOP_COUNT && (n = Jsi_Write(udf->chan, buf, m)) > 0) {
        /* TODO: limit max size. */
        sum += n;
        m -= n;
    }
bail:
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)sum);
    return JSI_OK;
}

static int FilesysPutsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
   
    UdfGet(udf, _this, funcPtr);
    if (!udf->filename) {
        Jsi_ValueMakeBool(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_Value *toput = Jsi_ValueArrayIndex(interp, args, 0);
    if (!toput) {
        Jsi_ValueMakeBool(interp, ret, 0);
        return JSI_OK;
    }
    const char * cp = Jsi_ValueToString(interp, toput, NULL);

    if (Jsi_Printf(udf->chan, "%s\n", cp?cp:"") < 0) {
        Jsi_ValueMakeBool(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_ValueMakeBool(interp, ret, 1);
    return JSI_OK;
}

static int FilesysEofCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    Jsi_ValueMakeBool(interp, ret, Jsi_Eof(udf->chan));
    return JSI_OK;
}

int Jsi_Printf(Jsi_Channel chan, const char *fmt, ...)
{
    va_list va;
    int n;
    FILE *fp = (chan && chan->fp ? chan->fp : stdout);
    va_start(va,fmt);
    n = vfprintf(fp, fmt, va);
    va_end(va);
    return n;
}


static int FilesysConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
    

static Jsi_CmdSpec filesysCmds[] = {
    { "Channel",   FilesysConstructor,1,  2,  "file:string, mode:string='r'", JSI_CMD_IS_CONSTRUCTOR, .help="A file input/output object. The mode string is r or w and an optional +", .retType=(uint)JSI_TT_USEROBJ },
    { "close",  FilesysCloseCmd,   0,  0, "", .help="close the file", .retType=(uint)JSI_TT_VOID },
//    { "conf",   FilesysConfCmd,     0,  1, "val:string|options",.help="Configure options" , .opts=filesysOptions},
    { "eof",    FilesysEofCmd,     0,  0, "", .help="Return true if read to end-of-file", .retType=(uint)JSI_TT_BOOL },
    { "filename", FilesysFilenameCmd, 0,  0, "", .help="Get file name", .retType=(uint)(uint)JSI_TT_STRING },
    { "flush",  FilesysFlushCmd,   0,  0, "", .help="Flush file output", .retType=(uint)JSI_TT_NUMBER },
    { "gets",   FilesysGetsCmd,    0,  0, "", .help="Get one line of input", .retType=(uint)JSI_TT_STRING },
    { "lstat",  FilesysLstatCmd,   0,  0, "", .help="Return status for file", .retType=(uint)JSI_TT_OBJECT },
    { "mode",   FilesysModeCmd,    0,  0, "", .help="Get file mode used with open", .retType=(uint)JSI_TT_STRING },
    { "open",   FilesysOpenCmd,    1, -1, "file:string, mode:string='r'", .help="Open the file (after close)", .retType=(uint)JSI_TT_BOOL },
    { "puts",   FilesysPutsCmd,    1,  1, "str", .help="Write one line of output", .retType=(uint)JSI_TT_BOOL },
    { "read",   FilesysReadCmd,    0,  1, "size:number=-1", .help="Read some or all of file", .retType=(uint)JSI_TT_STRING },
    { "seek",   FilesysSeekCmd,    2,  2, "pos:number, whence:number", .help="Seek to position. Return 0 if ok", .retType=(uint)JSI_TT_NUMBER },
    { "stat",   FilesysStatCmd,    0,  0, "", .help="Return status for file", .retType=(uint)JSI_TT_OBJECT },
    { "truncate",FilesysTruncateCmd,    1,  1, "pos:number", .help="Truncate file", .retType=(uint)JSI_TT_NUMBER },
    { "tell",   FilesysTellCmd,    0,  0, "", .help="Return current position", .retType=(uint)JSI_TT_NUMBER },
    { "write",  FilesysWriteCmd,   1,  1, "data", .help="Write data to file", .retType=(uint)JSI_TT_NUMBER },
    { NULL, .help="Commands for accessing Channel objects for file IO" }
};


static Jsi_UserObjReg fileobject = {
    "Channel",
    filesysCmds,
    fileObjFree,
    fileObjIsTrue,
    fileObjEqual
};

static int FilesysConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *that = _this;
    if (!Jsi_FunctionIsConstructor(funcPtr)) {
        Jsi_Obj *o = Jsi_ObjNew(interp);
        Jsi_PrototypeObjSet(interp, "File", o);
        Jsi_ValueMakeObject(interp, ret, o);
        that = *ret;
    }

    FileObj *fobj = (FileObj *)Jsi_Calloc(1,sizeof(*fobj));
    SIGINIT(fobj, FILEOBJ);
    if (try_open_file(interp, fobj, args) != JSI_OK) { /* Error out on open fail */
        Jsi_Free(fobj);
        Jsi_LogError("open failed");
        return JSI_ERROR;
    }
    Jsi_Obj *nobj = (Jsi_Obj*)Jsi_ValueGetObj(interp, that);
    fobj->objId = Jsi_UserObjNew(interp, &fileobject, nobj, fobj);
    if (fobj->objId<0) {
        Jsi_Free(fobj); // TODO: finish cleanup
        return JSI_ERROR;
    }
    fobj->fobj = nobj;
    return JSI_OK;
}

static void SetupStdChan(Jsi_Chan* ch, FILE *fp, Jsi_Filesystem *fsPtr, int flags) {
    memset(ch, 0, sizeof(*ch));
    ch->fp = fp;
    ch->fsPtr = fsPtr;
    ch->flags = flags|JSI_FS_NOCLOSE;
}

Jsi_Channel Jsi_FSNameToChannel(Jsi_Interp *interp, const char *name)
{
    if (strlen(name)==1) {
        switch (name[0]) {
            case '0' : return StdChans;
            case '1' : return StdChans+1;
            case '2' : return StdChans+2;
        }
    }
    if (!strcmp(name, "stdin")) return StdChans;
    if (!strcmp(name, "stdout")) return StdChans+1;
    if (!strcmp(name, "stderr")) return StdChans+2;
    Jsi_Obj *obj = jsi_UserObjFromName(interp, name);
    if (!obj)
        return NULL;
    Jsi_UserObj *uobj = obj->d.uobj;
    if (uobj->reg != &fileobject)
        return NULL;
    FileObj *fobj = (FileObj *)uobj->data;
    return fobj->chan;
}

int Jsi_DoneFilesys(Jsi_Interp *interp)
{
    Jsi_UserObjUnregister(interp, &fileobject);
    return JSI_OK;
}

int jsi_FilesysInit(Jsi_Interp *interp)
{

    Jsi_DSInit(&pwdStr);
    Jsi_Hash *fsys;
    if (!(fsys = Jsi_UserObjRegister(interp, &fileobject))) {
        Jsi_LogFatal("Can not init file system\n");
        return JSI_ERROR;
    }
    Jsi_CommandCreateSpecs(interp, fileobject.name, filesysCmds, fsys, 0);
    if (jsiFSList == NULL) {
        Jsi_FSRegister(&jsiFilesystem, NULL);
        SetupStdChan(StdChans, stdin, &jsiFilesystem, JSI_FS_READONLY);
        SetupStdChan(StdChans+1, stdout, &jsiFilesystem, JSI_FS_WRITEONLY);
        SetupStdChan(StdChans+2, stderr, &jsiFilesystem, JSI_FS_WRITEONLY);
    }
#ifndef JSI_OMIT_ZVFS
    Jsi_InitZvfs(interp);
#endif
    return JSI_OK;
}

void jsi_FilesysDone(Jsi_Interp *interp)
{
    Jsi_DoneZvfs(interp);
    while (jsiFSList != NULL) {
        FSList *fsPtr = jsiFSList;
        jsiFSList = fsPtr->next;
        if (fsPtr)
            Jsi_Free(fsPtr);
    }
}

#endif // JSI_LITE_ONLY


const char *jsi_GetHomeDir(Jsi_Interp *interp) {
    const char *str = NULL;
    if (interp->homeDir)
        return interp->homeDir;
#ifdef __WIN32
    str = getenv("USERPROFILE"); /* TODO: windows home dir. */
#else
        
    if ((str = getenv("HOME")) == NULL) {
        struct passwd pwd, *pw;
        char buf[20000];
        if (getpwuid_r(getuid(), &pwd, buf, sizeof(buf), &pw) == 0 && pw->pw_dir)        
            str = pw->pw_dir;
    }
#endif
    if (!str) {
        Jsi_LogBug("no home dir");
        str = "/";
    }
#ifdef JSI_LITE_ONLY
    return str;
#else
    return (interp->homeDir = Jsi_KeyAdd(interp, str));
#endif
}

/* TODO: reconcile with NormalizeUnixPath */
char* Jsi_NormalPath(Jsi_Interp *interp, const char *path, Jsi_DString *dStr) {
    char prefix[3] = "";
    char cdbuf[PATH_MAX];
    Jsi_DSInit(dStr);
    if (!path) return NULL;
    if (*path == '/')
        Jsi_DSAppend(dStr, path, NULL);
#ifdef __WIN32  /* TODO: add proper handling for windows paths. */
    else if (*path && path[1] == ':') {
        prefix[0] = *path;
        prefix[1] = path[1];
        prefix[2] = 0;
        path += 2;
        goto full;
    }
#endif
    else if (path[0] == '~') {
        Jsi_DSAppend(dStr, jsi_GetHomeDir(interp), (path[1] == '/' ? "" : "/"), path+1, NULL);
    } else if (path[0] == '.' && path[1] == 0) {
        if (jsi_pwd) {
            Jsi_DSAppend(dStr, jsi_pwd, NULL);
        } else {
            Jsi_DSAppend(dStr, getcwd(cdbuf, sizeof(cdbuf)), NULL);
        }
    } else {
#ifdef __WIN32
full:
#endif
        if (jsi_pwd) {
            Jsi_DSAppend(dStr, jsi_pwd, "/", path, NULL);
        } else {
            Jsi_DSAppend(dStr, getcwd(cdbuf, sizeof(cdbuf)), "/", path, NULL);
        }
    }
    Jsi_DString sStr = {};
    char *cp = Jsi_DSValue(dStr);
    int i=0, n=0, m, nmax, unclean=0, slens[PATH_MAX];
    char *sp = cp, *ss;
    char *sptrs[PATH_MAX];
    while (*cp && n<PATH_MAX) {
        while (*cp && *cp == '/') {
            cp++;
            if (*cp == '/')
                unclean = 1;
        }
        sptrs[n] = cp;
        if (cp[0] == '.' && (cp[1] == '.' || cp[1] == '/'))
            unclean = 1;
        ss = cp++;
        while (*ss && *ss != '/')
            ss++;
        slens[n++] = (ss-cp) + 1;
        cp = ss;
    }
    if (!unclean)
        return sp;
    /* Need to remove //, /./, /../ */
    nmax = n--;
    while (n>0) {
        if (slens[n]<=0) {
            n--;
            continue;
        }
        if (strncmp(sptrs[n],".",slens[n])==0)
            slens[n] = 0;
        else if (strncmp(ss,"..",slens[n])==0) {
            int cnt = 0;
            m = n-1;
            while (m>=0 && cnt<1) {
                if (slens[m])
                    cnt++;
                slens[m] = 0;
                m--;
            }
            if (cnt<1)
                return sp;  /* Can't fix it */
        }
        n--;
    }
    /* TODO: prefix for windows. */
    Jsi_DSAppend(&sStr, prefix, NULL);
    for (i=0; i<nmax; i++) {
        if (slens[i]) {
            Jsi_DSAppend(&sStr, "/", NULL);
            Jsi_DSAppendLen(&sStr, sptrs[i], slens[i]);
        }
    }
    Jsi_DSSetLength(dStr, 0);
    Jsi_DSAppend(dStr, Jsi_DSValue(&sStr), NULL);
    Jsi_DSFree(&sStr);
    return Jsi_DSValue(dStr);
}

char * Jsi_GetCwd(Jsi_Interp *interp, Jsi_DString *cwdPtr) {
    char cdbuf[PATH_MAX];
    Jsi_DSInit(cwdPtr);
    if (cwdFsPtr)
        Jsi_DSAppend(cwdPtr, Jsi_DSValue(&pwdStr), NULL);
    else
        Jsi_DSAppend(cwdPtr, getcwd(cdbuf, sizeof(cdbuf)), NULL);
    return Jsi_DSValue(cwdPtr);
}

#ifdef __WIN32
#define realpath(R,N) _fullpath(N,R, _MAX_PATH)

/* For ridding backslashes from env vars */
static void DeBackSlashify(char *cp) {
    char *dp = cp;
    while (*cp) {
        if (*cp == '\\') {
            *dp = '/';
        } else
            *dp = *cp;
        cp++; dp++;
    }
    *dp = 0;
}
#endif

static void NormalizeUnixPath(Jsi_Interp *interp, char *path) {
    char **argv; int argc, i;
    if (!strstr(path, "./")) return;
    Jsi_DString dStr = {}, eStr = {};
    if (path[0] != '/' && strstr(path, "..")) {
        char *npath = Jsi_GetCwd(interp, &eStr);
        if (npath && strcmp(npath,"/")) {
            Jsi_DSAppend(&eStr, "/", path, NULL);
            path = Jsi_DSValue(&eStr);
        }
    }
    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    Jsi_SplitStr(path, &argc, &argv, "/", &sStr);
    char *cp = path;
    *cp = 0;
    for (i=0; i<argc; i++) {
        if (i == 0 && argv[0][0] == 0) {
            continue;
        } else if (argv[i][0] == 0) {
            continue;
        } else if (!strcmp(argv[i],".")) {
            continue;
        } else if (!strcmp(argv[i],"..")) {
            char *pcp = dStr.str;
            pcp = strrchr(pcp, '/');
            if (pcp && pcp != dStr.str) {
                *pcp = 0;
            }
            continue;
        } else {
            Jsi_DSAppend(&dStr, "/", argv[i], NULL);
        }
    }
    Jsi_DSFree(&sStr);
    Jsi_Strcpy(path, dStr.str);
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&eStr);
}

char *Jsi_FileRealpathStr(Jsi_Interp *interp, const char *path, char *newname)
{
    if (!path) return NULL;
    Jsi_DString dStr;
    char *npath = (char*)path, *apath;
    Jsi_DSInit(&dStr);
    if (*path == '~') {
#ifndef __WIN32
        struct passwd pw, *pwp; /* TODO: could fallback to using env HOME. */
        char buf[BUFSIZ];
        int n = getpwuid_r(getuid(), &pw, buf, sizeof(buf), &pwp);
        const char *homedir = (n == 0 ? pwp->pw_dir : "");
        Jsi_DSAppend(&dStr, homedir, path[1] == '/' ? "" : "/", path+1, NULL);
#else
        const char *homedir = getenv("HOMEPATH");
        if (!homedir) homedir = "/";
        const char *homedrv = getenv("HOMEDRIVE");
        if (!homedrv) homedrv = "";
        Jsi_DSAppend(&dStr, homedrv, homedir, path[1] == '/' ? "" : "/", path+1, NULL);
#endif
        npath = Jsi_DSValue(&dStr);
    }
    apath = realpath(npath, newname);
    if (!apath) {
        apath = Jsi_Strdup(npath);
#ifndef __WIN32
        /* If path not exists on unix we try to eliminate ../ and /./ etc.*/
        NormalizeUnixPath(interp, apath);
#endif
    }
#ifdef __WIN32
    DeBackSlashify(apath);
#endif
    Jsi_DSFree(&dStr);
    return apath;
}

