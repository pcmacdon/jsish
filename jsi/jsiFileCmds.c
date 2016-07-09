#ifndef JSI_LITE_ONLY
#ifndef JSI_OMIT_FILESYS
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#ifndef __WIN32
#include <pwd.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fcntl.h>
#endif
#include <limits.h>

#define HAVE_UNISTD_H
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif defined(_MSC_VER)
#include <direct.h>
#define F_OK 0
#define W_OK 2
#define R_OK 4
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define GSVal(s) GetStringVal(interp, s)
static const char *GetStringVal(Jsi_Interp *interp, Jsi_Value *s) {
    const char *cp = Jsi_ValueString(interp, s, 0);
    return cp ? cp : "";
}

#define SAFEACCESS(fname, writ)  \
if (interp->isSafe && Jsi_InterpAccess(interp, fname, writ) != JSI_OK) { \
        Jsi_LogError("%s access denied", writ?"write":"read"); \
        return JSI_ERROR;\
    }

int jsi_FileStatCmd(Jsi_Interp *interp, Jsi_Value *fnam, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int islstat)
{
    int rc;
    Jsi_StatBuf st;
    SAFEACCESS(fnam, 0)
    if (islstat)
        rc = Jsi_Lstat(interp, fnam, &st);
    else
        rc = Jsi_Stat(interp, fnam, &st);
    if (rc != 0) {
        Jsi_LogError("file not found: %s", GSVal(fnam));
        return JSI_ERROR;
    }
   /* Single object containing result members. */
    Jsi_Value *vres;
    Jsi_Obj  *ores;
    Jsi_Value *nnv;
    vres = Jsi_ValueMakeObject(interp, NULL, ores = Jsi_ObjNew(interp));
    Jsi_IncrRefCount(interp, vres);
#define MKDBL(nam,val) \
    nnv = Jsi_ValueMakeNumber(interp, NULL, (Jsi_Number)val); \
    Jsi_ObjInsert(interp, ores, nam, nnv, 0);
    
    MKDBL("dev",st.st_dev); MKDBL("ino",st.st_ino); MKDBL("mode",st. st_mode);
    MKDBL("nlink",st.st_nlink); MKDBL("uid",st.st_uid); MKDBL("gid",st.st_gid);
    MKDBL("rdev",st.st_rdev);
#ifndef __WIN32
    MKDBL("blksize",st.st_blksize); MKDBL("blocks",st.st_blocks);
#endif
    MKDBL("atime",st.st_atime); MKDBL("mtime",st.st_mtime); MKDBL("ctime",st.st_ctime);    
    MKDBL("size",st.st_size);
    Jsi_ValueDup2(interp, ret, vres);
    Jsi_DecrRefCount(interp, vres);
    return JSI_OK;
}


static int FileStatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return jsi_FileStatCmd(interp, Jsi_ValueArrayIndex(interp, args, 0), _this, ret, funcPtr, 0);
}

static int FileLstatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return jsi_FileStatCmd(interp, Jsi_ValueArrayIndex(interp, args, 0), _this, ret, funcPtr, 1);
}

static const char *getFileType(int mode, int lmode)
{
#ifdef S_ISLNK
    if (S_ISLNK(mode) || S_ISLNK(lmode)) {
        return "link";
    }
#endif
    if (S_ISDIR(mode)) {
        return "directory";
    }
#ifdef S_ISCHR
    else if (S_ISCHR(mode)) {
        return "characterSpecial";
    }
#endif
#ifdef S_ISBLK
    else if (S_ISBLK(mode)) {
        return "blockSpecial";
    }
#endif
#ifdef S_ISFIFO
    else if (S_ISFIFO(mode)) {
        return "fifo";
    }
#endif
#ifdef S_ISSOCK
    else if (S_ISSOCK(mode)) {
        return "socket";
    }
#endif
    else if (S_ISREG(mode)) {
        return "file";
    }
    return "unknown";
}

enum { FSS_Exists, FSS_Atime, FSS_Mtime, FSS_Writable, FSS_Readable, FSS_Executable, FSS_Type, 
FSS_Owned, FSS_Isdir, FSS_Isfile };

static int _FileSubstat(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int sub)
{
    Jsi_Value *fnam = Jsi_ValueArrayIndex(interp, args, 0);
    int rc;
    Jsi_StatBuf st, lst;
    rc = Jsi_Stat(interp, fnam, &st) | Jsi_Lstat(interp, fnam, &lst);
    if (rc != 0 && sub != FSS_Exists) {
        if (!interp->fileStrict)
            return JSI_OK;
        Jsi_LogError("file not found: %s", GSVal(fnam));
        return JSI_ERROR;
    }
    switch (sub) {
        case FSS_Exists: Jsi_ValueMakeBool(interp, ret, (rc == 0)); break;
        case FSS_Atime: Jsi_ValueMakeNumber(interp, ret, st.st_atime); break;
        case FSS_Mtime: Jsi_ValueMakeNumber(interp, ret, st.st_mtime); break;
        case FSS_Writable: Jsi_ValueMakeBool(interp, ret, (Jsi_Access(interp, fnam, W_OK) != -1));  break;
        case FSS_Readable: Jsi_ValueMakeBool(interp, ret, (Jsi_Access(interp, fnam, R_OK) != -1));  break;
        case FSS_Executable:
#ifdef X_OK
            Jsi_ValueMakeBool(interp, ret, Jsi_Access(interp, fnam, X_OK) != -1);
#else
            Jsi_ValueMakeBool(interp, ret, 1);
#endif
            break;
        case FSS_Type: Jsi_ValueMakeStringKey(interp, ret, (char*)getFileType((int)st.st_mode, (int)lst.st_mode)); break;
        case FSS_Owned:
#ifndef __WIN32
            Jsi_ValueMakeBool(interp, ret, geteuid() == st.st_uid);
#endif
            break;
        case FSS_Isdir: Jsi_ValueMakeBool(interp, ret, S_ISDIR(st.st_mode));  break;
        case FSS_Isfile: Jsi_ValueMakeBool(interp, ret, S_ISREG(st.st_mode));  break;
        
    }
    return JSI_OK;
}
#define MAKE_FSS_SUB(nam) \
static int File##nam##Cmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, \
    Jsi_Value **ret, Jsi_Func *funcPtr) \
{\
    return _FileSubstat(interp, args, _this, ret, funcPtr, FSS_##nam);\
}
MAKE_FSS_SUB(Exists) MAKE_FSS_SUB(Atime) MAKE_FSS_SUB(Writable) MAKE_FSS_SUB(Readable)
MAKE_FSS_SUB(Executable) MAKE_FSS_SUB(Type) MAKE_FSS_SUB(Owned)
MAKE_FSS_SUB(Isdir) MAKE_FSS_SUB(Isfile) MAKE_FSS_SUB(Mtime)
#ifndef __WIN32
#define MKDIR_DEFAULT(PATHNAME) mkdir(PATHNAME, 0755)
#else
#define MKDIR_DEFAULT(PATHNAME) mkdir(PATHNAME)
#endif

static int mkdir_all(Jsi_Interp *interp, Jsi_Value *file)
{
    int ok = 1;
    char *path = Jsi_ValueString(interp, file, NULL);
    if (!path) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }

    /* First time just try to make the dir */
    goto first;

    while (ok--) {
        /* Must have failed the first time, so recursively make the parent and try again */
        {
            char *slash = strrchr(path, '/');

            if (slash && slash != path) {
                *slash = 0;
                if (mkdir_all(interp, file) != 0) {
                    return -1;
                }
                *slash = '/';
            }
        }
      first:
        if (MKDIR_DEFAULT(path) == 0) {
            return 0;
        }
        if (errno == ENOENT) {
            /* Create the parent and try again */
            continue;
        }
        /* Maybe it already exists as a directory */
        if (errno == EEXIST) {
            Jsi_StatBuf sb;

            if (Jsi_Stat(interp, file, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                return 0;
            }
            /* Restore errno */
            errno = EEXIST;
        }
        /* Failed */
        break;
    }
    return -1;
}

static int FilePwdCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_DString dStr = {};
    Jsi_ValueMakeStringDup(interp, ret, Jsi_GetCwd(interp, &dStr));
    Jsi_DSFree(&dStr);
    return JSI_OK;
}

static int FileChdirCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    if (!path) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    int rc = Jsi_Chdir(interp, path);
    if (rc != 0) {
        Jsi_LogError("can't change to directory \"%s\": %s", GSVal(path), strerror(errno));
        return JSI_ERROR;
    }    
    return JSI_OK;
}

static int FileMkdirCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 1);
    char *spath =  Jsi_ValueString(interp, path,0);
    int rc, force = 0; 

    if (!spath) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) {
        Jsi_LogError("expected boolean");
        return JSI_ERROR;
    }
    if (vf)
        force = vf->d.val;
    if (force==0)
        rc = MKDIR_DEFAULT(spath);
    else {
        Jsi_Value *npath = Jsi_ValueNewStringDup(interp, spath);
        rc = mkdir_all(interp, npath);
        Jsi_ValueFree(interp, npath);
    }
    if (rc != 0) {
        Jsi_LogError("can't create directory \"%s\": %s", spath, strerror(errno));
        return JSI_ERROR;
    }    
    return JSI_OK;
}

static int FileTempfileCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *vt = Jsi_ValueArrayIndex(interp, args, 0);
    char *filename;
    const char *tp, *templ = "/tmp/jsiXXXXXX";

    if (vt && (tp = Jsi_ValueString(interp, vt, NULL))) {
        templ = tp;
    }
#ifndef __WIN32
    filename = Jsi_Strdup(templ);
    int fd = mkstemp(filename);
    if (fd < 0)
        goto fail;
    close(fd);
#else
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
   char name[MAX_PATH];
    HANDLE handle;

    if (!GetTempPath(MAX_PATH, name) || !GetTempFileName(name, "JSI", 0, name)) {
        Jsi_LogError("failed to get temp file");
        return JSI_ERROR;
    }
    filename = Jsi_Strdup(name);
#endif
    Jsi_ValueMakeString(interp, ret, filename);
    return JSI_OK;
    
fail:
    Jsi_LogError("Failed to create tempfile");
    Jsi_Free(filename);
    return JSI_ERROR;
}

static int FileTruncateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{   
    int rc;
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *sizv = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Number siz;
    if (Jsi_GetNumberFromValue(interp, sizv, &siz) != JSI_OK)
        return JSI_ERROR;
    SAFEACCESS(fname, 1)
    Jsi_Channel ch = Jsi_Open(interp, fname, "rb+");
    if (!ch)
        return JSI_ERROR;
    rc = Jsi_Truncate(ch, (unsigned int)siz);
    Jsi_Close(ch);
    return rc;
}


static int FileChmodCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *modv = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Number fmod;
    if (Jsi_GetNumberFromValue(interp, modv, &fmod) != JSI_OK)
        return JSI_ERROR;
    SAFEACCESS(fname, 1)
    return Jsi_Chmod(interp, fname, (unsigned int)fmod);
}

static int FileReadCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) /* TODO limit. */
{   
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vmode = Jsi_ValueArrayIndex(interp, args, 1);
    const char *mode = (vmode ? Jsi_ValueString(interp, vmode, NULL) : NULL);
    SAFEACCESS(fname, 0)

    Jsi_Channel chan = Jsi_Open(interp, fname, (mode ? mode : "rb"));
    int n, sum = 0;
    if (!chan) {
        Jsi_LogError("failed open for read: %s", GSVal(fname));
        return JSI_ERROR;
    }
    Jsi_DString dStr = {};
    char buf[BUFSIZ];
    while (sum < MAX_LOOP_COUNT && (n = Jsi_Read(chan, buf, sizeof(buf))) > 0) {
        Jsi_DSAppendLen(&dStr, buf, n);
        sum += n;
    }
    Jsi_Close(chan);
    Jsi_ValueMakeDStringObject(interp, ret, &dStr);
    return JSI_OK;
}


static int FileWriteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{   
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    const char *data;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *vmode = Jsi_ValueArrayIndex(interp, args, 2);
    const char *mode = (vmode ? Jsi_ValueString(interp, vmode, NULL) : NULL);
    Jsi_Channel chan;
    int n, len, cnt = 0, sum = 0;
    SAFEACCESS(fname, 1)
    if (!(data = Jsi_ValueGetStringLen(interp, v, &len))) {
        return JSI_ERROR;
    }
    chan = Jsi_Open(interp, fname, (mode ? mode : "wb+"));
    if (!chan) {
        Jsi_LogError("failed open for write: %s", GSVal(fname));
        return JSI_ERROR;
    }
    while (cnt < MAX_LOOP_COUNT && len > 0 && (n = Jsi_Write(chan, data, len)) > 0) {
        len -= n;
        sum += n;
        cnt++;
    }
    Jsi_Close(chan);
    /* TODO: handle nulls. */
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)sum);
    return JSI_OK;
}

static int FileRenameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{   
    Jsi_Value *source = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *dest = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 2);
    int force = 0; 

    SAFEACCESS(source, 1)
    SAFEACCESS(dest, 1)
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) {
        Jsi_LogError("expected boolean");
        return JSI_ERROR;
    }
    if (vf)
        force = vf->d.val;
    if (force==0 && Jsi_Access(interp, dest, F_OK) == 0) {
        Jsi_LogError("error renaming \"%s\" to \"%s\": target exists", GSVal(source), GSVal(dest));
        return JSI_ERROR;
    }

    if (Jsi_Rename(interp, source, dest) != 0) {
        Jsi_LogError( "error renaming \"%s\" to \"%s\": %s", GSVal(source),
            GSVal(dest), strerror(errno));
        return JSI_ERROR;
    }
    return JSI_OK;
}

static int FileCopy(Jsi_Interp *interp, Jsi_Value* src, Jsi_Value* dest) {
    Jsi_Channel ich = Jsi_Open(interp, src, "rb");
    if (!ich)
        return -1;
    Jsi_Channel och = Jsi_Open(interp, dest, "wb+");
    if (!och)
        return -1;
    while (1) {
        char buf[BUFSIZ];
        int n;
        n = Jsi_Read(ich, buf, BUFSIZ);
        if (n<=0)
            break;
        if (Jsi_Write(och, buf, n) != n) {
            Jsi_Close(ich);
            Jsi_Close(och);
            return -1;
        }
    }
    /* TODO: set perms. */
#ifndef __WIN32
    Jsi_StatBuf sb;
    Jsi_Stat(interp, src, &sb);
    Jsi_Chmod(interp, dest, sb.st_mode);
#endif
    Jsi_Close(ich);
    Jsi_Close(och);
    return 0;
}

#define FN_copy JSI_INFO("\
Directories are not handled. \
The third argument if given is a boolean force value \
which if true allows overwrite of an existing file. ")
static int FileCopyCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *source = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *dest = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 2);
    int force = 0; 
    SAFEACCESS(source, 0)
    SAFEACCESS(dest, 1)
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) {
        Jsi_LogError("expected boolean");
        return JSI_ERROR;
    }
    if (vf)
        force = vf->d.val;
    if (force==0 && Jsi_Access(interp, dest, F_OK) == 0) {
        Jsi_LogError("error copying \"%s\" to \"%s\": target exists", GSVal(source), GSVal(dest));
        return JSI_ERROR;
    }

    if (FileCopy(interp, source, dest) != 0) {
        Jsi_LogError( "error copying \"%s\" to \"%s\": %s", GSVal(source),
            GSVal(dest), strerror(errno));
        return JSI_ERROR;
    }
    return JSI_OK;
}

#define FN_link JSI_INFO("\
The second argument is the destination file to be created. "\
"If a third bool argument is true, a hard link is created.")
static int FileLinkCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *source = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *dest = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 2);
    int hard = 0; 
    SAFEACCESS(source, 1)
    SAFEACCESS(dest, 1)
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) {
        Jsi_LogError("expected boolean");
        return JSI_ERROR;
    }
    if (vf)
        hard = vf->d.val;

    if (Jsi_Link(interp, source, dest, hard) != 0) {
        Jsi_LogError( "error linking \"%s\" to \"%s\": %s", GSVal(source),
            GSVal(dest), strerror(errno));
        return JSI_ERROR;
    }
    return JSI_OK;
}

/* TODO: switch to MatchesInDir */
static int RmdirAll(Jsi_Interp *interp, Jsi_Value *path, int force)
{
    DIR *dir;
    struct dirent *entry;
    char spath[PATH_MAX];
    int erc = JSI_OK;
    char *dirname = Jsi_ValueString(interp, path, 0);
    if (!dirname) {
        Jsi_LogError("expected string");
        return JSI_ERROR;
    }

    /* TODO: change to Jsi_Scandir */
    dir = opendir(dirname);
    if (dir == NULL) {
        if (force)
            return JSI_OK;
        Jsi_LogError("opening directory: %s", dirname);
        return JSI_ERROR;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            snprintf(spath, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
            Jsi_Value *tpPtr = Jsi_ValueNew1(interp);
            Jsi_ValueMakeStringDup(interp, &tpPtr, spath);
#ifndef __WIN32
            if (entry->d_type == DT_DIR) {
                int rc = RmdirAll(interp, tpPtr, force);
                if (rc != JSI_OK)
                    erc = rc;
            } else 
#endif
            {
                if (Jsi_Remove(interp, tpPtr, force) != 0) {
                    if (force)
                        Jsi_LogError("deleting file: %s", GSVal(tpPtr));
                    erc = JSI_ERROR;
                }
            }
            Jsi_DecrRefCount(interp, tpPtr);
        }
    }
    closedir(dir);
    Jsi_Remove(interp, path, force);
    return erc;
}

static int FileRemoveCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *source = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 1);
    const char *spath = GSVal(source);
    int rc, force = 0; 
    SAFEACCESS(source, 1)

    Jsi_StatBuf st;
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) {
        Jsi_LogError("expected boolean");
        return JSI_ERROR;
    }
    if (vf)
        force = vf->d.val;
    rc = Jsi_Stat(interp, source, &st);
    if (strcmp(spath,"/")==0)
        return JSI_ERROR;
    if (rc != 0) {
        if (force)
            return JSI_OK;
        Jsi_LogError("error deleting \"%s\": target not found", spath);
        return JSI_ERROR;
    }

    if (Jsi_Remove(interp, source, force) != 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (force)
                return JSI_OK;
            Jsi_LogError("error deleting \"%s\"", spath);
            return JSI_ERROR;
        }
        if (force==0) {
            Jsi_LogError("error deleting \"%s\": directory not empty", spath);
            return JSI_ERROR;
        }
        return RmdirAll(interp, source, force);
    }
    return JSI_OK;
}

static int FileReadlinkCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    SAFEACCESS(path, 1)

    char *linkValue = (char*)Jsi_Malloc(MAXPATHLEN + 1);

    int linkLength = Jsi_Readlink(interp, path, linkValue, MAXPATHLEN);

    if (linkLength == -1) {
        Jsi_Free(linkValue);
        Jsi_LogError("couldn't readlink \"%s\": %s", GSVal(path), strerror(errno));
        return JSI_ERROR;
    }
    linkValue[linkLength] = 0;
    Jsi_ValueMakeString(interp, ret, linkValue);
    return JSI_OK;
}

static int FileDirnameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    const char *p = strrchr(path, '/');

    if (!p && path[0] == '.' && path[1] == '.' && path[2] == '\0') {
        Jsi_ValueMakeStringKey(interp, ret, "..");
    } else if (!p) {
        Jsi_ValueMakeStringKey(interp, ret, ".");
    }
    else if (p == path) {
        Jsi_ValueMakeStringKey(interp, ret, "/");
    }
#if defined(__MINGW32__) || defined(_MSC_VER)
    else if (p[-1] == ':') {
        Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(path, 0, p-path+1));
    }
#endif
    else {
        Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(path, 0, p-path));
    }
    return JSI_OK;
}

static int FileRootnameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    const char *lastSlash = strrchr(path, '/');
    const char *p = strrchr(path, '.');

    if (p == NULL || (lastSlash != NULL && lastSlash > p)) {
        Jsi_ValueMakeStringDup(interp, ret, path);
    }
    else {
        Jsi_ValueMakeString(interp, ret, jsi_SubstrDup(path,0, p-path));
    }
    return JSI_OK;
}
static int FileExtensionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    const char *lastSlash = strrchr(path, '/');
    const char *p = strrchr(path, '.');

    if (p == NULL || (lastSlash != NULL && lastSlash >= p)) {
        p = "";
    }
    Jsi_ValueMakeStringDup(interp, ret, p);
    return JSI_OK;

}
static int FileTailCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    const char *lastSlash = strrchr(path, '/');

    if (lastSlash) {
        Jsi_ValueMakeStringDup(interp, ret, lastSlash+1);
    }
    else {
        Jsi_ValueMakeStringDup(interp, ret, path);
    }
    return JSI_OK;
}

static int FileRealpathCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    char *newname = Jsi_FileRealpath(interp, path, NULL);
    if (newname)
        Jsi_ValueMakeString(interp, ret, (char*)newname);
    return JSI_OK;

}

static int FileJoinCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path1 = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
    const char *path2 = Jsi_ValueArrayIndexToStr(interp, args, 1, NULL);
    Jsi_Value *p2 = Jsi_ValueArrayIndex(interp, args, 1);

    char *newname;
    if (*path2 == '/' || *path2 == '~')
        newname = Jsi_FileRealpath(interp, p2, NULL);
    else {
        Jsi_DString dStr = {};
        Jsi_DSAppend(&dStr, path1, "/", path2, NULL);
        Jsi_Value *tpPtr = Jsi_ValueNew1(interp);
        Jsi_ValueMakeStringDup(interp, &tpPtr, Jsi_DSValue(&dStr));
        newname = Jsi_FileRealpath(interp, tpPtr, NULL);
        Jsi_DSFree(&dStr);
        Jsi_DecrRefCount(interp, tpPtr);
    }
    if (newname ==  NULL)
        return JSI_ERROR;
    Jsi_ValueMakeString(interp, ret, (char*)newname);
    return JSI_OK;
}

static int FileSizeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    int rc;
    Jsi_StatBuf st;
    SAFEACCESS(path, 0)
    if (Jsi_ValueArrayIndex(interp, args, 1))
        rc = Jsi_Lstat(interp, path, &st);
    else
        rc = Jsi_Stat(interp, path, &st);
    if (rc != 0) {
        Jsi_LogError("bad file");
        return JSI_ERROR;
    }
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)st.st_size);
    return JSI_OK;
}

typedef struct {
    int max;            /* Max number of results to return. */
    Jsi_Bool recurse;        /* Recurse into directories. */
    Jsi_Bool tails;          /* Return only the tails. */
    int maxDepth;       /* For recursive */
    int flags;
    int (*filter)(const Jsi_Dirent *);
    int (*compar)(const Jsi_Dirent **, const Jsi_Dirent**);
    Jsi_Value *types;   /* File types to include */
    Jsi_Value *notypes; /* File types to exclude */
    Jsi_Value *func;    /* Function that returns true to keep. */
    Jsi_Value *dirfunc;    /* Function that returns true to recurse into dir. */
    Jsi_Value *dir;
    const char *prefix;
   // Jsi_Value *perms;
} GlobData;

static Jsi_OptionSpec GlobOptions[] = {
    JSI_OPT(VALUE,  GlobData, dir,      .help="Directory"),
    JSI_OPT(INT,    GlobData, maxDepth, .help="Max depth to recurse to"),
    JSI_OPT(FUNC,   GlobData, dirfunc,  .help="Returns true to recurse into dir"),
    JSI_OPT(FUNC,   GlobData, func,     .help="Function that returns true to keep"),
    JSI_OPT(INT,    GlobData, max,      .help="Max results to return"),
    JSI_OPT(VALUE,  GlobData, notypes,  .help="File types to exclude, one or more of chars 'fdlpsbc' for file, directory, link, etc"),
//  JSI_OPT(STRING, GlobData, perms,    .help="File permissions"),
    JSI_OPT(STRKEY, GlobData, prefix,   .help="String prefix to add to each file in list"),
    JSI_OPT(BOOL,   GlobData, recurse,  .help="Recurse into directories"),
    JSI_OPT(BOOL,   GlobData, tails,    .help="Return only the tails"),
    JSI_OPT(VALUE,  GlobData, types,    .help="File types to include, one or more of chars 'fdlpsbc' for file, directory, link, etc"),
    JSI_OPT_END(GlobData)
};

static int SubGlobDirectory(Jsi_Interp *interp, Jsi_Obj *obj, char *zPattern, Jsi_Value *
pattern,  int isreg, char *path, const char *prefix, GlobData *opts, int deep)
{
    struct dirent **namelist;
    int i, n, rc = JSI_OK, flags = opts->flags;
    Jsi_DString tStr = {};
    Jsi_Value *rpPath = Jsi_ValueNew1(interp);
    Jsi_ValueMakeStringDup(interp, &rpPath, path);
    
    if ((n=Jsi_Scandir(interp, rpPath, &namelist, 0, 0)) < 0) {
        Jsi_LogError("bad directory");
        Jsi_DecrRefCount(interp, rpPath);
        return JSI_ERROR;
    }

    for (i=0; i<n && rc == JSI_OK; i++)
    {
        int ftyp;
        const char *z = namelist[i]->d_name;
        if (*z == '.') {
            if (!(flags&JSI_FILE_TYPE_HIDDEN))
                continue;
            else if ((z[1] == 0 || (z[1] == '.' && z[2] == 0)))
                continue;
        }
#ifdef __WIN32
        /* HACK in scandir(): if is directory then inode set to 1. */
        ftyp = (namelist[i]->d_ino? DT_DIR : DT_REG);
#else
        ftyp = namelist[i]->d_type;
#endif
        if (ftyp == DT_DIR) {
            if (opts->recurse && (opts->maxDepth<=0 || (deep+1) <= opts->maxDepth)) {
                Jsi_DString sStr = {}, pStr = {};
                Jsi_DSAppend(&sStr, path, "/", z, NULL);
                if (opts->tails==0)
                    Jsi_DSAppend(&pStr, prefix?prefix:"", prefix?"/":"", z, NULL);
                if (opts->dirfunc) {
                    if (!Jsi_FunctionInvokeBool(interp, opts->dirfunc,
                        Jsi_ValueNewStringDup(interp, Jsi_DSValue(&pStr))))
                        continue;
                }       
                rc = SubGlobDirectory(interp, obj, zPattern, pattern,
                    isreg, Jsi_DSValue(&sStr), Jsi_DSValue(&pStr), opts, deep+1);
                Jsi_DSFree(&sStr);
                Jsi_DSFree(&pStr);
                if (rc != JSI_OK)
                    goto done;
            }
            if (opts->types==0 && opts->notypes && (!(flags&JSI_FILE_TYPE_DIRS)))
                continue;
        } else {
            if (!(flags&JSI_FILE_TYPE_FILES))
                continue;
        }
        if (opts->types) {
            const char *cp = Jsi_ValueString(interp, opts->types, 0);
            int mat = 0;
            while (cp && *cp && !mat) {
                mat = 0;
                switch (*cp) {
                    case 'd': mat = (ftyp == DT_DIR); break;
                    case 'f': mat = (ftyp == DT_REG); break;
#ifndef __WIN32
                    case 'p': mat = (ftyp == DT_FIFO); break;
                    case 's': mat = (ftyp == DT_SOCK); break;
                    case 'l': mat = (ftyp == DT_LNK); break;
                    case 'c': mat = (ftyp == DT_CHR); break;
                    case 'b': mat = (ftyp == DT_BLK); break;
#endif
                }
                cp++;
            }
            if (!mat)
                continue;
        }
        if (opts->notypes) {
            const char *cp = Jsi_ValueString(interp, opts->notypes, 0);
            int mat = 0;
            while (cp && *cp && !mat) {
                mat = 0;
                switch (*cp) {
                    case 'd': mat = (ftyp == DT_DIR); break;
                    case 'f': mat = (ftyp == DT_REG); break;
#ifndef __WIN32
                    case 'p': mat = (ftyp == DT_FIFO); break;
                    case 's': mat = (ftyp == DT_SOCK); break;
                    case 'l': mat = (ftyp == DT_LNK); break;
                    case 'c': mat = (ftyp == DT_CHR); break;
                    case 'b': mat = (ftyp == DT_BLK); break;
#endif
                }
                cp++;
            }
            if (mat)
                continue;
        }

        if (isreg) {
           int ismat;
            Jsi_RegExpMatch(interp, pattern, z, &ismat, NULL);
            if (!ismat)
                continue;
        } else if (zPattern != NULL && Jsi_GlobMatch(zPattern, z, 0) == 0)
            continue;
        if (prefix) {
            Jsi_DSInit(&tStr);
            Jsi_DSAppend(&tStr, prefix, "/", z, NULL);
            z = Jsi_DSValue(&tStr);
        }
        if (opts->func) {
            if (!Jsi_FunctionInvokeBool(interp, opts->func,
                Jsi_ValueNewStringDup(interp, z)))
                continue;
        }       
        rc = Jsi_ObjArrayAdd(interp, obj, Jsi_ValueNewStringDup(interp, z));
        if (prefix)
            Jsi_DSFree(&tStr);
        if (opts->max>0 && obj->arrCnt >= opts->max)
            break;
            
    }

done:
    if (rpPath)
        Jsi_DecrRefCount(interp, rpPath);
    if (namelist) {
        while (--n >= 0)
            Jsi_Free(namelist[n]);
        Jsi_Free(namelist);
    }
    return rc;     
}
#define FN_glob JSI_INFO("\
With no arguments (or null) returns all files/directories in current directory. \
If first argument is a pattern (either a glob or regexp) just files are returned. \
If second argument is a string, it denotes the directory to search in. \
If second argument is a function, this function is called with each path. \
Otherwise second arugment is a set of options.")
static int FileGlobCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int fo = 1, isOpts = 0, rc = JSI_OK;
    Jsi_Value *pat = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *dir = NULL;
    Jsi_Value *tvPtr = Jsi_ValueNew1(interp);
    Jsi_Value *pvPtr = Jsi_ValueNew1(interp);
    GlobData Data;
    Jsi_Obj *obj;
    int len, isreg = 0;
    char *zPattern = NULL, *pathPtr;

    memset(&Data, 0, sizeof(Data));
    Data.flags = JSI_FILE_TYPE_FILES;

    if (pat == NULL || Jsi_ValueIsNull(interp, pat)) {
        Jsi_ValueMakeStringKey(interp, &pvPtr, "*");
        pat = pvPtr;
        Data.flags = JSI_FILE_TYPE_DIRS|JSI_FILE_TYPE_FILES;
    } else if (Jsi_ValueIsString(interp, pat)) {
        char *cpp, *pstr = Jsi_ValueString(interp, pat, NULL);
        if (pstr && ((cpp=strrchr(pstr, '/')))) {
            Jsi_DString dStr = {};
            Jsi_DSAppendLen(&dStr, pstr, (cpp-pstr));
            Jsi_ValueMakeStringKey(interp,  &tvPtr, Jsi_DSValue(&dStr));
            dir = tvPtr;
            
            Jsi_DSSetLength(&dStr, 0);
            Jsi_DSAppend(&dStr, cpp+1, NULL);
            Jsi_ValueMakeStringKey(interp,  &pvPtr, Jsi_DSValue(&dStr));
            pat = pvPtr;
            Jsi_DSFree(&dStr);
        }
    }
    if (arg)
        switch (arg->vt) {
        case JSI_VT_NULL: break;
        case JSI_VT_STRING: dir = arg; break;
        case JSI_VT_OBJECT:
        {
            Jsi_Obj *obj = arg->d.obj;
            switch (obj->ot) {
                case JSI_VT_STRING: dir = arg; break;
                case JSI_VT_OBJECT:
                    if (Jsi_ValueIsFunction(interp, arg)) {
                        Data.func = arg;
                        break;
                    } else if (!obj->isarrlist) {
                        isOpts = 1;
                        break;
                    }
                default: fo = 0;
            }
            if (fo) break;
        }
        default:
            Jsi_LogError("expected string, function or options for arg2");
            rc = JSI_ERROR;
            goto done;
    }
    if (isOpts && Jsi_OptionsProcess(interp, GlobOptions, arg, &Data, 0) < 0) {
        rc = JSI_ERROR;
        goto done;
    }
    if (Data.dir) {
        if (dir) {
            Jsi_LogError("multiple dirs specified");
            rc = JSI_ERROR;
            goto done;
        }
        dir = Data.dir;
    }
    if (dir == NULL) {
        Jsi_ValueMakeStringKey(interp, &tvPtr, ".");
        dir = tvPtr;
    } else {
        char *dcp = Jsi_ValueString(interp, dir, NULL);
        if (dcp && *dcp == '~') {
            dcp = Jsi_FileRealpath(interp, dir, NULL);
            if (dcp) {
                Jsi_ValueMakeString(interp, &tvPtr, dcp);
                dir = tvPtr;
            }
        }
    }
    if (interp->isSafe && Jsi_InterpAccess(interp, dir, 0) != JSI_OK) {
        Jsi_LogError("read access denied");
        rc = JSI_ERROR;
        goto done;
    }

    if (!(isreg=Jsi_ValueIsObjType(interp, pat, JSI_OT_REGEXP)))
        zPattern = Jsi_ValueString(interp, pat, &len);
    pathPtr = Jsi_ValueString(interp, dir, &len);
    if (!pathPtr) { rc = JSI_ERROR; goto done; }
    if (len && pathPtr[len-1] == '/')
        len--;
    obj = Jsi_ObjNew(interp);
    Jsi_ValueMakeArrayObject(interp, ret, obj);
    rc = SubGlobDirectory(interp, obj, zPattern, pat, isreg, pathPtr, Data.prefix, &Data, 0);
    if (rc != JSI_OK)
        Jsi_ValueMakeUndef(interp, ret);   
done:
    if (pvPtr)
        Jsi_DecrRefCount(interp, pvPtr);
    if (tvPtr)
        Jsi_DecrRefCount(interp, tvPtr);
    if (isOpts)
        Jsi_OptionsFree(interp, GlobOptions, &Data, 0);
    return rc;
}

static Jsi_CmdSpec fileCmds[] = {
    { "atime",      FileAtimeCmd,       1,  1, "file:string",  .help="Return file Jsi_Access time", .retType=(uint)JSI_TT_NUMBER },
    { "chdir",      FileChdirCmd,       1,  1, "file:string",  .help="Change current directory" },
    { "chmod",      FileChmodCmd,       2,  2, "file:string, mode:string",  .help="Set file permissions" },
    { "copy",       FileCopyCmd,        2,  3, "src:string, dest:string, force:boolean=false",  .help="Copy a file to destination", .info=FN_copy },
    { "dirname",    FileDirnameCmd,     1,  1, "file:string",  .help="Return directory path", .retType=(uint)JSI_TT_STRING },
    { "executable", FileExecutableCmd,  1,  1, "file:string",  .help="Return true if file is executable", .retType=(uint)JSI_TT_BOOL },
    { "exists",     FileExistsCmd,      1,  1, "file:string",  .help="Return true if file exists", .retType=(uint)JSI_TT_BOOL },
    { "extension",  FileExtensionCmd,   1,  1, "file:string",  .help="Return file extension", .retType=(uint)JSI_TT_STRING },
    { "join",       FileJoinCmd,        2,  2, "path:string, path:string",  .help="Join two file realpaths, or just second if an absolute path", .retType=(uint)JSI_TT_STRING },
    { "isdir",      FileIsdirCmd,       1,  1, "file:string",  .help="Return true if file is a directory", .retType=(uint)JSI_TT_BOOL },
    { "isfile",     FileIsfileCmd,      1,  1, "file:string",  .help="Return true if file is a normal file", .retType=(uint)JSI_TT_BOOL },
    { "glob",       FileGlobCmd,        0,  2, "pattern:regexp|string|null='*', options:string|boolean|object=void", .help="Return list of files in dir with optional pattern match", .opts=GlobOptions, .info=FN_glob, .retType=(uint)JSI_TT_ARRAY },
    { "link",       FileLinkCmd,        2,  3, "src:string, dest:string, ishard:boolean=false",  .help="Link a file", .info=FN_link },
    { "lstat",      FileLstatCmd,       1,  1, "file:string",  .help="Return status info for file", .retType=(uint)JSI_TT_OBJECT },
    { "mkdir",      FileMkdirCmd,       1,  1, "file:string",  .help="Create a directory" },
    { "mtime",      FileMtimeCmd,       1,  1, "file:string",  .help="Return file modified time", .retType=(uint)JSI_TT_NUMBER },
    { "owned",      FileOwnedCmd,       1,  1, "file:string",  .help="Return true if file is owned by user", .retType=(uint)JSI_TT_BOOL },
    { "pwd",        FilePwdCmd,         0,  0, "",  .help="Return current directory", .retType=(uint)JSI_TT_STRING },
    { "remove",     FileRemoveCmd,      1,  2, "file:string, force:boolean=false",  .help="Delete a file or direcotry" },
    { "rename",     FileRenameCmd,      2,  3, "src:string, dest:string, force:boolean=false",  .help="Rename a file, with possible overwrite" },
    { "read",       FileReadCmd,        1,  2, "file:string, mode:string='rb'",  .help="Read a file", .retType=(uint)JSI_TT_STRING },
    { "readable",   FileReadableCmd,    1,  1, "file:string",  .help="Return true if file is readable", .retType=(uint)JSI_TT_BOOL },
    { "readlink",   FileReadlinkCmd,    1,  1, "file:string",  .help="Read file link destination", .retType=(uint)JSI_TT_STRING },
    { "realpath",   FileRealpathCmd,    1,  1, "file:string",  .help="Return absolute file name minus .., ./ etc.", .retType=(uint)JSI_TT_STRING },
    { "rootname",   FileRootnameCmd,    1,  1, "file:string",  .help="Return file name minus extension", .retType=(uint)JSI_TT_STRING },
    { "size",       FileSizeCmd,        1,  1, "file:string",  .help="Return size for file", .retType=(uint)JSI_TT_NULL },
    { "stat",       FileStatCmd,        1,  1, "file:string",  .help="Return status info for file", .retType=(uint)JSI_TT_OBJECT },
    { "tail",       FileTailCmd,        1,  1, "file:string",  .help="Return file name minus dirname", .retType=(uint)JSI_TT_STRING },
    { "tempfile",   FileTempfileCmd,    1,  1, "file:string",  .help="Create a temp file", .retType=(uint)JSI_TT_ANY },
    { "truncate",   FileTruncateCmd,    2,  2, "file:string, size:number",  .help="Truncate file" },
    { "type",       FileTypeCmd,        1,  1, "file:string",  .help="Return type of file", .retType=(uint)JSI_TT_STRING },
    { "write",      FileWriteCmd,       2,  3, "file:string, str:string, mode:string='wb+'",  .help="Write a file", .retType=(uint)JSI_TT_NUMBER },
    { "writable",   FileWritableCmd,    1,  1, "file:string",  .help="Return true if file is writable", .retType=(uint)JSI_TT_BOOL },
    { NULL, .help="Commands for accessing the filesystem" }
};

int jsi_FileCmdsInit(Jsi_Interp *interp)
{
    Jsi_CommandCreateSpecs(interp, "File",   fileCmds,   NULL, JSI_CMDSPEC_NOTOBJ);
    return JSI_OK;
}
#endif
#endif
