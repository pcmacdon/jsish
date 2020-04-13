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

#ifndef JSI_LITE_ONLY

#define _JSI_GETFP(ch,in) (ch && ch->fp ? ch->fp : (in?stdin:stdout))

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
static Jsi_RC fileObjFree(Jsi_Interp *interp, void *data);
static bool fileObjIsTrue(void *data);
static bool fileObjEqual(void *data1, void *data2);

#ifdef __WIN32
char *get_current_dir_name() {
    static char buf[MAX_PATH] = ".";
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

static bool jsi_FSPathInFilesystemProc(Jsi_Interp *interp, Jsi_Value* path,void **clientDataPtr) {return 1;}

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
static int jsi_FSFlushProc(Jsi_Channel chan) { return fflush(_JSI_GETFP(chan,0));}
static int jsi_FSTellProc(Jsi_Channel chan) { return ftell(_JSI_GETFP(chan,1));}
static int jsi_FSEofProc(Jsi_Channel chan) { return feof(_JSI_GETFP(chan,1));}
static int jsi_FSTruncateProc(Jsi_Channel chan, unsigned int len) { return ftruncate(fileno(_JSI_GETFP(chan,1)), len);}
static int jsi_FSRewindProc(Jsi_Channel chan) { rewind(_JSI_GETFP(chan,1)); return 0;}
static int jsi_FSCloseProc(Jsi_Channel chan) { return fclose(_JSI_GETFP(chan,1));}
static int jsi_FSSeekProc(Jsi_Channel chan, Jsi_Wide offset, int mode) { return fseek(_JSI_GETFP(chan,1), offset, mode);}

static int jsi_FSWriteProc(Jsi_Channel chan, const char *buf, int size) {
    return fwrite(buf, 1, size, _JSI_GETFP(chan,0));
}

static int jsi_FSReadProc(Jsi_Channel chan, char *buf, int size) {
    return fread(buf, 1, size, _JSI_GETFP(chan,0));
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
    return fgetc(_JSI_GETFP(chan,1));
}

static int jsi_FSUngetcProc(Jsi_Channel chan, int ch) {
    return ungetc(ch, _JSI_GETFP(chan,1));
}

static char * jsi_FSGetsProc(Jsi_Channel chan, char *s, int size) {
    return fgets(s, size, _JSI_GETFP(chan,1));
}

static int jsi_FSPutsProc(Jsi_Channel chan, const char *s) {
    return fputs(s, _JSI_GETFP(chan,0));
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
    .putsProc=jsi_FSPutsProc,
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
    return jsiIntData.stdChans+id;
}

Jsi_RC Jsi_FSRegister(Jsi_Filesystem *fsPtr, void *data) {
    FSList *fsl = (FSList *)Jsi_Calloc(1, sizeof(*fsl));
    fsl->fsPtr = fsPtr;
    fsl->data = data;
    fsl->next = jsiFSList;
    jsiFSList = fsl;
    return JSI_OK;
}

Jsi_RC Jsi_FSUnregister(Jsi_Filesystem *fsPtr) {
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
    if (!pathStr || pathStr[0] == '~')
        return &jsiFilesystem;
    if (pathStr[0] == '.' && pathStr[1] == 0)
        return (jsiIntData.cwdFsPtr ? jsiIntData.cwdFsPtr : &jsiFilesystem);
    Jsi_Value *tpath = NULL;
    if (Jsi_Strstr(pathStr, "..")) {
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        pathStr = Jsi_ValueNormalPath(interp, path, &dStr);
        tpath = path = Jsi_ValueNewStringDup(interp, pathStr);
        Jsi_IncrRefCount(interp, tpath);
        Jsi_DSFree(&dStr);
    }
    while (1) {
        if (fsl->fsPtr->pathInFilesystemProc && fsl->fsPtr->pathInFilesystemProc(interp, path, clientDataPtr))
            break;
        if (!fsl->next)
            break;
        fsl = fsl->next;
    }
    if (tpath)
        Jsi_DecrRefCount(interp, tpath);
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
    if (interp->isSafe && fsPtr && fsPtr == &jsiFilesystem) {
        int aflag = (mode&W_OK ? JSI_INTACCESS_WRITE : JSI_INTACCESS_READ);
        if (Jsi_InterpAccess(interp, path, aflag) != JSI_OK)
            return -1;
    }
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

bool Jsi_FSNative(Jsi_Interp *interp, Jsi_Value *file) {
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
    int fnl = 0;
    const char *fileName = Jsi_ValueString(interp, file, &fnl), *oldFN = fileName;
    if (!fileName || !fnl) {
        Jsi_LogError("expected string filename");
        return NULL;
    }
    if (!Jsi_Strcmp(fileName, "stdin")) return jsiIntData.stdChans;
    if (!Jsi_Strcmp(fileName, "stdout")) return jsiIntData.stdChans+1;
    if (!Jsi_Strcmp(fileName, "stderr")) return jsiIntData.stdChans+2;
    const char *s = modeString;
    bool quiet = 0;
    if (s[0]=='-') {
        quiet = true;
        s++;
    }
    char Mode[sizeof(ch->modes)];
    Jsi_StatBuf sb;
    Jsi_Value *path = NULL;
    int n, i, mode = 0, rc, writ, aflag;
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
        if (!ch) {
            if (!quiet)
                Jsi_LogError("File open failed '%s'", fileName);
            return NULL;
        }
        Jsi_Chan *nch = (Jsi_Chan *)Jsi_Calloc(1,sizeof(*nch));
        *nch = *ch;
        nch->fsPtr = fsPtr;
        return nch;
    }
    for (i=0, n = 0; s[i]; i++) {
        switch (s[i]) {
            case '+': break;
            case 'b': break;
            case 'r': if (!Jsi_Strchr(s,'+')) mode |= JSI_FS_READONLY; break;
            case 'a':
            case 'w': if (!Jsi_Strchr(s,'+')) mode |= JSI_FS_WRITEONLY; break;
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
    if (!fileName) {
        Jsi_LogError("file error: %s", oldFN);
        return NULL;
    }

    if (rc == 0 &&  sb.st_mode & S_IFDIR )
    {
        Jsi_LogError("can not open directory: %s", fileName);
        goto done;
    }
    fsPtr = Jsi_FilesystemForPath(interp, file, &data);
    writ = (Jsi_Strchr(s,'w') || Jsi_Strchr(s,'a') || Jsi_Strchr(s,'+'));
    aflag = (writ ? JSI_INTACCESS_WRITE : JSI_INTACCESS_READ);
    if (fsPtr && fsPtr != &jsiFilesystem) {
        ch = fsPtr->openProc(interp, file, Mode);
        if (ch)
            ch->isNative = 0;
        else
            Jsi_LogError("File open failed '%s'", fileName);
    } else {
        if (interp->isSafe && ((rc && Jsi_InterpAccess(interp, file, JSI_INTACCESS_CREATE) != JSI_OK)
        || Jsi_InterpAccess(interp, file, aflag) != JSI_OK)) {
            Jsi_LogError("%s access denied: %s", writ?"write":"read", fileName);
            goto done;
        }
        FILE *fp = fopen(fileName, Mode);
        fsPtr = &jsiFilesystem;
        if (!fp) {
            if (!quiet)
                Jsi_LogError("File open failed '%s'", fileName);
            goto done;
        }
        ch = (Jsi_Chan *)Jsi_Calloc(1,sizeof(*ch));
        ch->fp = fp;
        ch->isNative = 1;
    }
    if (ch) {
        ch->flags |= mode; // + (zLevel<<24);
        Jsi_Strcpy(ch->modes, s);
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
    
Jsi_Wide Jsi_Seek(Jsi_Interp *interp, Jsi_Channel chan, Jsi_Wide offset, int mode) {
    if (chan->fsPtr==0 || !chan->fsPtr->seekProc) return -1;
    return chan->fsPtr->seekProc(chan, offset, mode);
}
Jsi_Wide Jsi_Tell(Jsi_Interp *interp, Jsi_Channel chan) {
    if (chan->fsPtr==0 || !chan->fsPtr->tellProc) return -1;
    return chan->fsPtr->tellProc(chan);
}
int Jsi_Eof(Jsi_Interp *interp, Jsi_Channel chan) {
    if (chan->fsPtr==0 || !chan->fsPtr->eofProc) return -1;
    return chan->fsPtr->eofProc(chan);
}
Jsi_Wide Jsi_Rewind(Jsi_Interp *interp, Jsi_Channel chan) {
    if (chan->fsPtr==0 || !chan->fsPtr->rewindProc) return -1;
    return chan->fsPtr->rewindProc(chan);
}

/*Jsi_StatBuf* Jsi_AllocStatBuf(void) {return 0;}*/

int Jsi_Read(Jsi_Interp *interp, Jsi_Channel chan, char *bufPtr, int toRead) {
    if (chan->fsPtr==0 || !chan->fsPtr->readProc) return -1;
    return chan->fsPtr->readProc(chan, bufPtr, toRead);
}
int Jsi_Write(Jsi_Interp *interp, Jsi_Channel chan, const char *bufPtr, int slen) {
    if (chan->fsPtr==0 || !chan->fsPtr->writeProc) return -1;
    return chan->fsPtr->writeProc(chan, bufPtr, slen);
}
int Jsi_Truncate(Jsi_Interp *interp, Jsi_Channel chan, unsigned int len) {
    if (chan->fsPtr==0 || !chan->fsPtr->truncateProc) return -1;
    return chan->fsPtr->truncateProc(chan, len);
}

int Jsi_Close(Jsi_Interp *interp, Jsi_Channel chan) {
    if (chan->fsPtr==0 || !chan->fsPtr->closeProc) return -1;
    if (chan->flags&JSI_FS_NOCLOSE) return -1;
    int rc = chan->fsPtr->closeProc(chan);
    if (rc == 0)
        Jsi_Free(chan);
    return rc;
}
int Jsi_Flush(Jsi_Interp *interp, Jsi_Channel chan) {
    if (chan->fsPtr==0 || !chan->fsPtr->flushProc) return -1;
    return chan->fsPtr->flushProc(chan);
}

int Jsi_Getc(Jsi_Interp *interp, Jsi_Channel chan) {
    if (chan->fsPtr==0 || !chan->fsPtr->getcProc) return -1;
    return chan->fsPtr->getcProc(chan);
}

int Jsi_Ungetc(Jsi_Interp *interp, Jsi_Channel chan, int ch) {
    if (chan->fsPtr==0 || !chan->fsPtr->ungetcProc) return -1;
    return chan->fsPtr->ungetcProc(chan, ch);
}

char * Jsi_Gets(Jsi_Interp *interp, Jsi_Channel chan, char *s, int size) {
    if (chan->fsPtr==0 || !chan->fsPtr->getsProc) return NULL;
    return chan->fsPtr->getsProc(chan, s, size);
}

int Jsi_Chdir(Jsi_Interp *interp, Jsi_Value* path) {
    void *data;
    int rc = 0;
    const char *pathPtr = Jsi_ValueToString(interp, path, NULL);
    if (interp->isSafe && Jsi_InterpAccess(interp, path, JSI_INTACCESS_READ) != JSI_OK) 
        return Jsi_LogError("read access denied");
    Jsi_Filesystem *fsPtr = Jsi_FilesystemForPath(interp, path, &data);
    if (fsPtr == &jsiFilesystem) {
        rc = chdir(pathPtr);
        if (rc < 0)
            return -1;
        /* If change out of native fs, GetCwd will use pwdStr */
        fsPtr = NULL;
    }
    Jsi_DSSetLength(&jsiIntData.pwdStr, 0);
    Jsi_DSAppendLen(&jsiIntData.pwdStr, pathPtr, -1);
    jsiIntData.cwdFsPtr = fsPtr;
    jsiIntData.pwd = fsPtr ? Jsi_DSValue(&jsiIntData.pwdStr) : NULL;
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

Jsi_RC Jsi_PathNormalize(Jsi_Interp *interp, Jsi_Value **pathPtr) {
    Jsi_Value *path = *pathPtr;
    if (!path) {
        Jsi_DString dStr = {};
        *pathPtr = Jsi_ValueNewStringDup(interp, Jsi_GetCwd(interp, &dStr));
        Jsi_IncrRefCount(interp, *pathPtr);
        Jsi_DSFree(&dStr);
    } else {
        const char *rn = Jsi_Realpath(interp, path, NULL);
        if (!rn) return JSI_ERROR;
        Jsi_DecrRefCount(interp, *pathPtr);
        *pathPtr = Jsi_ValueNewString(interp, rn, -1);
        Jsi_IncrRefCount(interp, *pathPtr);
    }
    return JSI_OK;
}

char *Jsi_Realpath(Jsi_Interp *interp, Jsi_Value *src, char *newname)
{
    /* TODO: resolve pwd first. */
    void *data;
    const char *cp = NULL;
    Jsi_Filesystem *fsPtr;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    const char *npath = Jsi_ValueNormalPath(interp, src, &dStr);
    if (!npath) return NULL;
    Jsi_Value *tPtr = Jsi_ValueNew1(interp);
    Jsi_ValueMakeStringDup(interp, &tPtr, npath);
    fsPtr = Jsi_FilesystemForPath(interp, tPtr, &data);
    if (fsPtr) {
        if (fsPtr->realpathProc)
            cp = fsPtr->realpathProc(interp, src, newname);
        else if (!newname)
            cp = Jsi_Strdup(npath);
        else {
            Jsi_Strncpy(newname, npath, PATH_MAX);
            if (Jsi_Strlen(npath)>=PATH_MAX)
                newname[PATH_MAX-1] = 0;
        }
    }
    Jsi_DSFree(&dStr);
    Jsi_DecrRefCount(interp, tPtr);
    return (char*)cp;
}


static void fileObjErase(FileObj *fo)
{
    if (fo->filename) {
        Jsi_Close(fo->interp, fo->chan);
        Jsi_Free(fo->filename);
        Jsi_DecrRefCount(fo->interp, fo->fname);
        Jsi_Free(fo->mode);
    }
    fo->filename = NULL;
}

static Jsi_RC fileObjFree(Jsi_Interp *interp, void *data)
{
    FileObj *fo = (FileObj *)data;
    SIGASSERT(fo,FILEOBJ);
    fileObjErase(fo);
    Jsi_Free(fo);
    return JSI_OK;
}

static bool fileObjIsTrue(void *data)
{
    FileObj *fo = (FileObj *)data;
    SIGASSERT(fo,FILEOBJ);
    if (!fo->filename) return JSI_OK;
    else return 1;
}

static bool fileObjEqual(void *data1, void *data2)
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
        int writ = (mode && (Jsi_Strchr(mode,'w')||Jsi_Strchr(mode,'+')));
        int aflag = (writ ? JSI_INTACCESS_WRITE : JSI_INTACCESS_READ);
        if (interp->isSafe && Jsi_InterpAccess(interp, fname, aflag) != JSI_OK)
            return Jsi_LogError("Safe open failed");
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
    if (!udf) \
        return Jsi_LogError("Channel.%s called with non-file object", funcPtr->cmdSpec->name);

static Jsi_RC FilesysOpenCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    if (try_open_file(interp, udf, args) != JSI_OK) {
        Jsi_ValueMakeBool(interp, ret, 0);
    }
    Jsi_ValueMakeBool(interp, ret, 1);
    return JSI_OK;
}

static Jsi_RC FilesysCloseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    fileObjErase(udf);
    Jsi_ValueMakeBool(interp, ret, 1);
    return JSI_OK;
}

static Jsi_RC FilesysGetsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int len;
    UdfGet(udf, _this, funcPtr);
    if (!udf->filename) {
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_OK;
    }
    char buf[JSI_BUFSIZ];
    if (!Jsi_Gets(interp, udf->chan, buf, sizeof(buf))) {
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_OK;
    }
    buf[sizeof(buf)-1] = 0;
    len = Jsi_Strlen(buf);
    if (len > 0 && buf[len-1] == '\n')
        buf[len-1] = 0;
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}


static Jsi_RC FilesysModeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    if (udf->mode)
        Jsi_ValueMakeStringKey(interp, ret, udf->mode);
    return JSI_OK;
}

static Jsi_RC FilesysFilenameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    if (udf->filename)
        Jsi_ValueMakeStringDup(interp, ret, udf->filename);
    return JSI_OK;
}

static Jsi_RC FilesysReadCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int  sum = 0, n;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    UdfGet(udf, _this, funcPtr);
    char buf[JSI_BUFSIZ];
    int argc = Jsi_ValueGetLength(interp, args);
    int nsiz = -1, cnt = 0, rsiz;
    
    if (!udf->filename) {
        goto bail;
    }
    if (argc>0 && Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, 0), &nsiz) != JSI_OK)
        goto bail;
    while (cnt++ < MAX_LOOP_COUNT) {
        rsiz = sizeof(buf);
        if (nsiz>0 && ((sum+rsiz)>nsiz))
            rsiz = (nsiz-sum);
        if ((n = Jsi_Read(interp, udf->chan, buf, rsiz)) <= 0)
            break;
        Jsi_DSAppendLen(&dStr, buf, n);
        sum += n;
        if (nsiz>=0 && sum >=nsiz)
            break;
    }
    if (Jsi_DSLength(&dStr)>0)
        Jsi_ValueMakeDStringObject(interp, ret, &dStr);
    return JSI_OK;
    
bail:
    Jsi_DSFree(&dStr);
    Jsi_ValueMakeUndef(interp, ret);
    return JSI_OK;
}


static Jsi_RC FilesysSeekCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
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
    pos = Jsi_Seek(interp, udf->chan, pos, mode);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)pos);
    return JSI_OK;
}

static Jsi_RC FilesysTruncateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    Jsi_Value *vPos = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Number num;
    if (Jsi_ValueGetNumber(interp, vPos, &num) != JSI_OK)
        return JSI_ERROR;
    num = (Jsi_Number)Jsi_Truncate(interp, udf->chan, (unsigned int)num);
    Jsi_ValueMakeNumber(interp, ret, num);
    return JSI_OK;
}

static Jsi_RC FilesysStatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
#if JSI__FILESYS==1
    return jsi_FileStatCmd(interp, udf->fname, _this, ret, funcPtr, 0);
#else
    return JSI_ERROR;
#endif
}

static Jsi_RC FilesysLstatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
#if JSI__FILESYS==1
    return jsi_FileStatCmd(interp, udf->fname, _this, ret, funcPtr, 1);
#else
    return JSI_ERROR;
#endif
}

static Jsi_RC FilesysTellCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    Jsi_Wide pos = Jsi_Tell(interp, udf->chan);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)pos);
    return JSI_OK;
}

static Jsi_RC FilesysFlushCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    int pos = Jsi_Flush(interp, udf->chan);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)pos);
    return JSI_OK;
}

static Jsi_RC FilesysWriteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int  sum = 0, n, m;
    UdfGet(udf, _this, funcPtr);
    char *buf = Jsi_ValueArrayIndexToStr(interp, args, 0, &m);

    if (!udf->filename) {
        goto bail;
    }
    while (m > 0 && sum < MAX_LOOP_COUNT && (n = Jsi_Write(interp, udf->chan, buf, m)) > 0) {
        /* TODO: limit max size. */
        sum += n;
        m -= n;
    }
bail:
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)sum);
    return JSI_OK;
}

static Jsi_RC FilesysPutsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
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

    if (Jsi_Printf(interp, udf->chan, "%s\n", cp?cp:"") < 0) {
        Jsi_ValueMakeBool(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_ValueMakeBool(interp, ret, 1);
    return JSI_OK;
}

static Jsi_RC FilesysEofCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    UdfGet(udf, _this, funcPtr);
    Jsi_ValueMakeBool(interp, ret, Jsi_Eof(interp, udf->chan));
    return JSI_OK;
}

int Jsi_Printf(Jsi_Interp *interp, Jsi_Channel chan, const char *fmt, ...)
{
    va_list va;
    int n;
    FILE *fp = (chan && chan->fp ? chan->fp : stdout);
    va_start(va,fmt);
    n = vfprintf(fp, fmt, va);
    va_end(va);
    return n;
}

int Jsi_Puts(Jsi_Interp *interp, Jsi_Channel chan, const char *str, int size)
{
    int code = 0, len = Jsi_Strlen(str);
    if (chan->fsPtr==0 || !chan->fsPtr->putsProc) {
        FILE *fp = (chan && chan->fp ? chan->fp : stdout);
        code = fputs(str, fp);
    } else {
        if (interp && interp->debugOpts.putsCallback && interp->parent) {
            int code = 0;
            Jsi_DString jStr={}, kStr={};
            Jsi_DSPrintf(&kStr, "[%s, %d]",
                Jsi_JSONQuote(interp, str, size, &jStr), (chan->fp == stderr?1:0));
            if (Jsi_FunctionInvokeJSON(interp->parent, interp->debugOpts.putsCallback, Jsi_DSValue(&kStr), NULL) != JSI_OK)
                code = 1;
            Jsi_DSFree(&jStr);
            Jsi_DSFree(&kStr);
            return code;
        } else if (interp && interp->stdoutStr) {
            Jsi_DString dStr = {};
            Jsi_DSAppend(&dStr, Jsi_ValueString(interp, interp->stdoutStr, NULL), NULL);
            Jsi_DSAppend(&dStr, str, NULL);
            Jsi_ValueFromDS(interp, &dStr, &interp->stdoutStr);
        } else

            code = chan->fsPtr->putsProc(chan, str);
    }
    if (size>=0 && len < size) {
        Jsi_Puts(interp, chan, "\\0", -1);
        Jsi_Puts(interp, chan, str+len+1, size-len-1);
    }
    return code;
}

static Jsi_RC FilesysConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr);
    

static Jsi_CmdSpec filesysCmds[] = {
    { "Channel",   FilesysConstructor,1,  2,  "file:string, mode:string='r'", .help="A file input/output object. The mode string is r or w and an optional +", .retType=(uint)JSI_TT_USEROBJ, .flags=JSI_CMD_IS_CONSTRUCTOR },
    { "close",  FilesysCloseCmd,   0,  0, "", .help="Close the file", .retType=(uint)JSI_TT_BOOLEAN },
    { "eof",    FilesysEofCmd,     0,  0, "", .help="Return true if read to end-of-file", .retType=(uint)JSI_TT_BOOLEAN },
    { "filename", FilesysFilenameCmd, 0,  0, "", .help="Get file name", .retType=(uint)(uint)JSI_TT_STRING },
    { "flush",  FilesysFlushCmd,   0,  0, "", .help="Flush file output", .retType=(uint)JSI_TT_NUMBER },
    { "gets",   FilesysGetsCmd,    0,  0, "", .help="Get one line of input", .retType=(uint)JSI_TT_STRING|JSI_TT_VOID },
    { "lstat",  FilesysLstatCmd,   0,  0, "", .help="Return status for file", .retType=(uint)JSI_TT_OBJECT },
    { "mode",   FilesysModeCmd,    0,  0, "", .help="Get file mode used with open", .retType=(uint)JSI_TT_STRING },
    { "open",   FilesysOpenCmd,    1, -1, "file:string, mode:string='r'", .help="Open the file (after close)", .retType=(uint)JSI_TT_BOOLEAN },
    { "puts",   FilesysPutsCmd,    1,  1, "str", .help="Write one line of output", .retType=(uint)JSI_TT_BOOLEAN },
    { "read",   FilesysReadCmd,    0,  1, "size:number=-1", .help="Read some or all of file", .retType=(uint)JSI_TT_STRING|JSI_TT_VOID },
    { "seek",   FilesysSeekCmd,    2,  2, "pos:number, whence:string", .help="Seek to position. Return 0 if ok", .retType=(uint)JSI_TT_NUMBER },
    { "stat",   FilesysStatCmd,    0,  0, "", .help="Return status for file", .retType=(uint)JSI_TT_OBJECT },
    { "truncate",FilesysTruncateCmd,    1,  1, "pos:number", .help="Truncate file", .retType=(uint)JSI_TT_NUMBER },
    { "tell",   FilesysTellCmd,    0,  0, "", .help="Return current position", .retType=(uint)JSI_TT_NUMBER },
    { "write",  FilesysWriteCmd,   1,  1, "data", .help="Write data to file", .retType=(uint)JSI_TT_NUMBER },
    { NULL, 0,0,0,0, .help="Commands for accessing Channel objects for file IO" }
};


static Jsi_UserObjReg fileobject = {
    "Channel",
    filesysCmds,
    fileObjFree,
    fileObjIsTrue,
    fileObjEqual
};

static Jsi_RC FilesysConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
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
    if (Jsi_Strlen(name)==1) {
        switch (name[0]) {
            case '0' : return jsiIntData.stdChans;
            case '1' : return jsiIntData.stdChans+1;
            case '2' : return jsiIntData.stdChans+2;
        }
    }
    if (!Jsi_Strcmp(name, "stdin")) return jsiIntData.stdChans;
    if (!Jsi_Strcmp(name, "stdout")) return jsiIntData.stdChans+1;
    if (!Jsi_Strcmp(name, "stderr")) return jsiIntData.stdChans+2;
    Jsi_Obj *obj = jsi_UserObjFromName(interp, name);
    if (!obj)
        return NULL;
    Jsi_UserObj *uobj = obj->d.uobj;
    if (uobj->reg != &fileobject)
        return NULL;
    FileObj *fobj = (FileObj *)uobj->data;
    return fobj->chan;
}

void jsi_FilesysDone(Jsi_Interp *interp)
{
    Jsi_UserObjUnregister(interp, &fileobject);
    Jsi_InitZvfs(interp, 1);
    while (jsiFSList != NULL) {
        FSList *fsPtr = jsiFSList;
        jsiFSList = fsPtr->next;
        if (fsPtr)
            Jsi_Free(fsPtr);
    }
}

Jsi_RC jsi_InitFilesys(Jsi_Interp *interp, int release)
{
    if (release) {
        jsi_FilesysDone(interp);
        return JSI_OK;
    }
    Jsi_DSInit(&jsiIntData.pwdStr);
    Jsi_Hash *fsys = Jsi_UserObjRegister(interp, &fileobject);
    if (!fsys)
        return Jsi_LogBug("Can not init file system");
    Jsi_CommandCreateSpecs(interp, fileobject.name, filesysCmds, fsys, JSI_CMDSPEC_ISOBJ);
    if (jsiFSList == NULL) {
        Jsi_FSRegister(&jsiFilesystem, NULL);
        SetupStdChan(jsiIntData.stdChans, stdin, &jsiFilesystem, JSI_FS_READONLY);
        SetupStdChan(jsiIntData.stdChans+1, stdout, &jsiFilesystem, JSI_FS_WRITEONLY);
        SetupStdChan(jsiIntData.stdChans+2, stderr, &jsiFilesystem, JSI_FS_WRITEONLY);
    }
#if JSI__ZVFS==1
    Jsi_InitZvfs(interp, 0);
#endif
#ifndef JSI_OMIT_VFS
    jsi_InitVfs(interp, 0);
#endif
    return JSI_OK;
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
        char buf[JSI_BUFSIZ*3];
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
    if (!path || !path[0]) return NULL;
    if (*path == '/')
        Jsi_DSAppend(dStr, path, NULL);
#ifdef __WIN32  /* TODO: add proper handling for windows paths. */
    else if (*path && path[1] == ':') {
        Jsi_DSAppend(dStr, path, NULL);
        return Jsi_DSValue(dStr);
    }
#endif
    else if (path[0] == '~') {
        Jsi_DSAppend(dStr, jsi_GetHomeDir(interp), (path[1] == '/' ? "" : "/"), path+1, NULL);
    } else if (path[0] == '.' && path[1] == 0) {
        if (jsiIntData.pwd) {
            Jsi_DSAppend(dStr, jsiIntData.pwd, NULL);
        } else {
            Jsi_DSAppend(dStr, getcwd(cdbuf, sizeof(cdbuf)), NULL);
        }
    } else {
        if (jsiIntData.pwd) {
            Jsi_DSAppend(dStr, jsiIntData.pwd, "/", path, NULL);
        } else {
            Jsi_DSAppend(dStr, getcwd(cdbuf, sizeof(cdbuf)), "/", path, NULL);
        }
    }
    Jsi_DString sStr = {};
    char *cp = Jsi_DSValue(dStr);
#ifdef __WIN32
    if (*cp && cp[1] == ':') {
        prefix[0] = *cp;
        prefix[1] = cp[1];
        prefix[2] = 0;
        cp += 2;
    }
#endif
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
        if (Jsi_Strncmp(sptrs[n],".",slens[n])==0)
            slens[n] = 0;
        else if (Jsi_Strncmp(ss,"..",slens[n])==0) {
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
#ifdef __WIN32
            Jsi_DSAppend(&sStr, "/" /*"\\"*/, NULL);
#else
            Jsi_DSAppend(&sStr, "/", NULL);
#endif
            Jsi_DSAppendLen(&sStr, sptrs[i], slens[i]);
        }
    }
    Jsi_DSSetLength(dStr, 0);
    Jsi_DSAppend(dStr, Jsi_DSValue(&sStr), NULL);
    Jsi_DSFree(&sStr);
    return Jsi_DSValue(dStr);
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

#else

static void NormalizeUnixPath(Jsi_Interp *interp, char *path) {
    char **argv; int argc, i;
    if (!Jsi_Strstr(path, "./")) return;
    Jsi_DString dStr = {}, eStr = {};
    if (path[0] != '/' && Jsi_Strstr(path, "..")) {
        char *npath = Jsi_GetCwd(interp, &eStr);
        if (npath && Jsi_Strcmp(npath,"/")) {
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
        } else if (i && !Jsi_Strcmp(argv[i],".")) {
            continue;
        } else if (!Jsi_Strcmp(argv[i],"..")) {
            char *pcp = Jsi_DSValue(&dStr), *lcp = pcp;
            pcp = Jsi_Strrchr(pcp, '/');
            if (pcp && pcp != Jsi_DSValue(&dStr)) {
                *pcp = 0;
                Jsi_DSSetLength(&dStr, Jsi_Strlen(lcp));
            }
            continue;
        } else {
            Jsi_DSAppend(&dStr, (i>0?"/":""), argv[i], NULL);
        }
    }
    Jsi_DSFree(&sStr);
    Jsi_Strcpy(path, Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    Jsi_DSFree(&eStr);
}
#endif

char * Jsi_GetCwd(Jsi_Interp *interp, Jsi_DString *cwdPtr) {
    char cdbuf[PATH_MAX];
    Jsi_DSInit(cwdPtr);
    if (jsiIntData.cwdFsPtr)
        Jsi_DSAppend(cwdPtr, Jsi_DSValue(&jsiIntData.pwdStr), NULL);
    else
        Jsi_DSAppend(cwdPtr, getcwd(cdbuf, sizeof(cdbuf)), NULL);
#ifdef __WIN32
    DeBackSlashify(Jsi_DSValue(cwdPtr));
#endif
    return Jsi_DSValue(cwdPtr);
}

char *Jsi_FileRealpathStr(Jsi_Interp *interp, const char *path, char *newname)
{
    if (!path || !path[0]) return NULL;
    Jsi_DString dStr, eStr;
    char *npath = (char*)path, *apath;
    Jsi_DSInit(&dStr); Jsi_DSInit(&eStr);
    if (*path == '~') {
#ifndef __WIN32
        struct passwd pw, *pwp; /* TODO: could fallback to using env HOME. */
        char buf[JSI_BUFSIZ];
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
#ifdef __WIN32
    if (Jsi_Strncmp(path, JSI_ZVFS_DIR, sizeof(JSI_ZVFS_DIR)-1)==0 || Jsi_Strncmp(path, JSI_VFS_DIR, sizeof(JSI_VFS_DIR)-1)==0)
        apath = NULL;
    else
#endif
    apath = realpath(npath, newname);
    if (!apath) {
        if ((path[0] == '.' && path[1] == '/') || (path[0] != '/' && 
        !(path[0] == '.' && path[1] == '.') && path[1] != ':')) {
            Jsi_GetCwd(interp, &eStr);
            Jsi_DSAppend(&eStr, "/", path, NULL);
            npath = Jsi_DSValue(&eStr);
            apath = realpath(npath, newname);
            //npath = (char*)path;
        }
    }
    if (!apath) {
        if (newname)
            Jsi_Strcpy(apath=newname, npath);
        else
            apath = Jsi_Strdup(npath);
#ifndef __WIN32
        /* If path not exists on unix we try to eliminate ../ and /./ etc.*/
        NormalizeUnixPath(interp, apath);
#endif
    }
#ifdef __WIN32
    DeBackSlashify(apath);
#endif
    Jsi_DSFree(&dStr); Jsi_DSFree(&eStr);
    return apath;
}

