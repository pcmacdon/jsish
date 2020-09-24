#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#if JSI__FILESYS==1
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

#ifndef S_ISUID
#define S_ISUID 0004000
#define S_ISGID 0002000
#define S_ISVTX 0001000
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define GSVal(s) GetStringVal(interp, s)
static const char *GetStringVal(Jsi_Interp *interp, Jsi_Value *s) {
    const char *cp = Jsi_ValueString(interp, s, 0);
    return cp ? cp : "";
}

#define SAFEACCESS(fname, writ, create)  \
if (interp->isSafe && (Jsi_InterpAccess(interp, fname, (!writ ? JSI_INTACCESS_READ : JSI_INTACCESS_WRITE)) != JSI_OK \
        || (create && Jsi_InterpAccess(interp, fname, JSI_INTACCESS_CREATE) != JSI_OK))) \
        return Jsi_LogError("%s access denied by safe interp: %s", writ?"write":"read", GSVal(fname));

static char* getFileTypeCh(int mode, char smode[])
{
    char c = '-';
    if (S_ISREG(mode))
        c = '-';
    else if (S_ISDIR(mode))
        c = 'd';
#ifdef S_ISLNK
    else if (S_ISLNK(mode))
        c = 'l';
#endif
#ifdef S_ISCHR
    else if (S_ISCHR(mode))
        c = 'c';
#endif
#ifdef S_ISBLK
    else if (S_ISBLK(mode))
        c = 'b';
#endif
#ifdef S_ISFIFO
    else if (S_ISFIFO(mode))
        c = 'p';
#endif
#ifdef S_ISSOCK
    else if (S_ISSOCK(mode))
        c = 's';
#endif
    int i = 0;
    smode[i++] = c;
    smode[i++] =  ((mode & S_IRUSR)?'r':'-');
    smode[i++] =  ((mode & S_IWUSR)?'w':'-');
    smode[i++] =  ((mode & S_ISUID)?((mode & S_IXUSR)?'s':'S'): ((mode & S_IXUSR)?'x':'-'));
    smode[i++] =  ((mode & S_IRGRP)?'r':'-');
    smode[i++] =  ((mode & S_IWGRP)?'w':'-');
    smode[i++] =  ((mode & S_ISGID)?((mode & S_IXGRP)?'s':'S'): ((mode & S_IXGRP)?'x':'-'));
    smode[i++] =  ((mode & S_IROTH)?'r':'-');
    smode[i++] =  ((mode & S_IWOTH)?'w':'-');
    smode[i++] =  ((mode & S_ISVTX)?((mode & S_IXOTH)?'t':'T'): ((mode & S_IXOTH)?'x':'-'));
    smode[i] = 0;
    return smode;
}

Jsi_RC jsi_FileStatCmd(Jsi_Interp *interp, Jsi_Value *fnam, Jsi_Value **ret, int flags)
{
    int rc;
    Jsi_StatBuf st;
    SAFEACCESS(fnam, 0, 1)
    int islstat = flags&1;
    int isshort = flags&2;
    if (islstat)
        rc = Jsi_Lstat(interp, fnam, &st);
    else
        rc = Jsi_Stat(interp, fnam, &st);
    if (rc != 0) 
        return Jsi_LogError("file access failed: %s", GSVal(fnam));
   /* Single object containing result members. */
    Jsi_Value *vres;
    Jsi_Obj  *ores;
    Jsi_Value *nnv;
    vres = Jsi_ValueMakeObject(interp, NULL, ores = Jsi_ObjNew(interp));
    Jsi_IncrRefCount(interp, vres);
#define MKDBL(nam,val) \
    nnv = Jsi_ValueMakeNumber(interp, NULL, (Jsi_Number)val); \
    Jsi_ObjInsert(interp, ores, nam, nnv, 0);
    
    MKDBL("mtime",st.st_mtime); MKDBL("size",st.st_size);
    MKDBL("uid",st.st_uid); MKDBL("gid",st.st_gid);
    char smode[30];
    getFileTypeCh(st.st_mode, smode);
    Jsi_Value *nv = Jsi_ValueNewStringDup(interp, smode);
    Jsi_ObjInsert(interp, ores, "perms", nv, 0);
    if (!isshort) {
        MKDBL("mode",st.st_mode);
        MKDBL("dev",st.st_dev); MKDBL("ino",st.st_ino); 
        MKDBL("nlink",st.st_nlink); MKDBL("rdev",st.st_rdev);
#ifndef __WIN32
        MKDBL("blksize",st.st_blksize); MKDBL("blocks",st.st_blocks);
#endif
        MKDBL("ctime",st.st_ctime); MKDBL("atime",st.st_atime);
    }
    Jsi_ValueDup2(interp, ret, vres);
    Jsi_DecrRefCount(interp, vres);
    return JSI_OK;
}


static Jsi_RC FileStatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return jsi_FileStatCmd(interp, Jsi_ValueArrayIndex(interp, args, 0), ret, 0);
}

static Jsi_RC FileLstatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    return jsi_FileStatCmd(interp, Jsi_ValueArrayIndex(interp, args, 0), ret, 1);
}

enum { FSS_Exists, FSS_Atime, FSS_Mtime, FSS_Writable, FSS_Readable, FSS_Executable, FSS_Perms, 
FSS_Owned, FSS_Isdir, FSS_Isfile };

static Jsi_RC _FileSubstat(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int sub)
{
    Jsi_Value *fnam = Jsi_ValueArrayIndex(interp, args, 0);
    int rc;
    Jsi_StatBuf st = {}, lst = {};
    char smode[30];
    st.st_uid = -1;
    rc = Jsi_Stat(interp, fnam, &st) | Jsi_Lstat(interp, fnam, &lst);
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
        case FSS_Perms: 
            getFileTypeCh((int)st.st_mode, smode);
            Jsi_ValueMakeStringKey(interp, ret, smode); break;
        case FSS_Owned:
#ifndef __WIN32
            Jsi_ValueMakeBool(interp, ret, rc == 0 && geteuid() == st.st_uid);
#endif
            break;
        case FSS_Isdir: Jsi_ValueMakeBool(interp, ret, rc == 0 && S_ISDIR(st.st_mode));  break;
        case FSS_Isfile: Jsi_ValueMakeBool(interp, ret, rc == 0 && S_ISREG(st.st_mode));  break;
#ifndef __cplusplus
        default:
            assert(0);
#endif
    }
    return JSI_OK;
}
#define MAKE_FSS_SUB(nam) \
static Jsi_RC File##nam##Cmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, \
    Jsi_Value **ret, Jsi_Func *funcPtr) \
{\
    return _FileSubstat(interp, args, _this, ret, funcPtr, FSS_##nam);\
}
MAKE_FSS_SUB(Exists) MAKE_FSS_SUB(Atime) MAKE_FSS_SUB(Writable) MAKE_FSS_SUB(Readable)
MAKE_FSS_SUB(Executable) MAKE_FSS_SUB(Perms) MAKE_FSS_SUB(Owned)
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
        return -1;
    }

    /* First time just try to make the dir */
    goto first;

    while (ok--) {
        /* Must have failed the first time, so recursively make the parent and try again */
        {
            char *slash = Jsi_Strrchr(path, '/');

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

static Jsi_RC FilePwdCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_DString dStr = {};
    Jsi_ValueMakeStringDup(interp, ret, Jsi_GetCwd(interp, &dStr));
    Jsi_DSFree(&dStr);
    return JSI_OK;
}

static Jsi_RC FileChdirCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    if (!path) 
        return Jsi_LogError("expected string");
    int rc = Jsi_Chdir(interp, path);
    if (rc != 0) 
        return Jsi_LogError("can't change to directory \"%s\": %s", GSVal(path), strerror(errno));    
    return JSI_OK;
}

static Jsi_RC FileMkdirCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->isSafe && interp->safeMode == jsi_safe_Lockdown)
        return Jsi_LogError("disabled by safe mode");
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 1);
    char *spath =  Jsi_ValueString(interp, path,0);
    int rc, force = 0; 

    if (!spath) 
        return Jsi_LogError("expected string");
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) 
        return Jsi_LogError("expected boolean");
    SAFEACCESS(path, 1, 1)
    if (vf)
        force = vf->d.val;
    if (force==0)
        rc = MKDIR_DEFAULT(spath);
    else {
        Jsi_Value *npath = Jsi_ValueNewStringDup(interp, spath);
        rc = mkdir_all(interp, npath);
        Jsi_ValueFree(interp, npath);
    }
    if (rc != 0) 
        return Jsi_LogError("can't create directory \"%s\": %s", spath, strerror(errno));    
    return JSI_OK;
}

static Jsi_RC jsi_SAFEACCESS(Jsi_Interp *interp, Jsi_Value *fname, bool write)
{
    SAFEACCESS(fname, write, 1);
    return JSI_OK;
}

static Jsi_RC FileTempfileCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char *filename;
#ifndef __WIN32
    Jsi_Value *vt = Jsi_ValueArrayIndex(interp, args, 0);
    const char *tp, *templ = "/tmp/jsiXXXXXX";

    if (vt && (tp = Jsi_ValueString(interp, vt, NULL))) {
        templ = tp;
    }
    filename = Jsi_Strdup(templ);
    if (Jsi_InterpSafe(interp)) {
        Jsi_Value *fname = Jsi_ValueNewStringConst(interp, templ, -1);
        Jsi_IncrRefCount(interp, fname);
        Jsi_RC rc = jsi_SAFEACCESS(interp, fname, 1);
        Jsi_DecrRefCount(interp, fname);
        if (rc != JSI_OK)
            return rc;
    }
    int fd = mkstemp(filename);
    if (fd < 0)
        goto fail;
    close(fd);
#else
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
    char name[MAX_PATH];

    if (!GetTempPath(MAX_PATH, name) || !GetTempFileName(name, "JSI", 0, name)) 
        return Jsi_LogError("failed to get temp file");
    if (Jsi_InterpSafe(interp)) {
        Jsi_Value *fname = Jsi_ValueNewStringConst(interp, name, -1);
        Jsi_IncrRefCount(interp, fname);
        Jsi_RC rc = jsi_SAFEACCESS(interp, fname, 1);
        Jsi_DecrRefCount(interp, fname);
        if (rc != JSI_OK)
            return rc;
    }
    filename = Jsi_Strdup(name);
#endif
    Jsi_ValueMakeString(interp, ret, filename);
    return JSI_OK;
    
#ifndef __WIN32
fail:
    Jsi_LogError("Failed to create tempfile");
    Jsi_Free(filename);
    return JSI_ERROR;
#endif
}

static Jsi_RC FileTruncateCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{   
    if (interp->isSafe && interp->safeMode == jsi_safe_Lockdown)
        return Jsi_LogError("disabled by safe mode");
    Jsi_RC rc;
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *sizv = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Number siz;
    if (Jsi_GetNumberFromValue(interp, sizv, &siz) != JSI_OK)
        return JSI_ERROR;
    SAFEACCESS(fname, 1, 0)
    Jsi_Channel ch = Jsi_Open(interp, fname, "rb+");
    if (!ch)
        return JSI_ERROR;
    rc = (Jsi_Truncate(interp, ch, (unsigned int)siz) == 0 ? JSI_OK : JSI_ERROR);
    Jsi_Close(interp, ch);
    return rc;
}

static Jsi_RC FileMknodCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->isSafe && interp->safeMode == jsi_safe_Lockdown)
        return Jsi_LogError("disabled by safe mode");
#ifdef __WIN32
    return Jsi_LogError("mknod unsupported");
#else
    Jsi_Number m, d;
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    const char *nam = Jsi_ValueString(interp, fname, NULL);
    if (Jsi_GetNumberFromValue(interp, Jsi_ValueArrayIndex(interp, args, 1), &m) != JSI_OK
        || Jsi_GetNumberFromValue(interp, Jsi_ValueArrayIndex(interp, args, 2), &d) != JSI_OK)
        return Jsi_LogError("expected numbers for arg 2 & 3");
    SAFEACCESS(fname, 1, 1)
    mode_t mode = (mode_t)m;
    dev_t dev = (dev_t)m;
    if (mknod(nam, mode, dev))
        return Jsi_LogError("mknod failed: %s", strerror(errno));
    return JSI_OK;
#endif
}

static Jsi_RC FileChmodCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->isSafe && interp->safeMode == jsi_safe_Lockdown)
        return Jsi_LogError("disabled by safe mode");
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *modv = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Number fmod;
    if (Jsi_GetNumberFromValue(interp, modv, &fmod) != JSI_OK)
        return JSI_ERROR;
    SAFEACCESS(fname, 1, 0)
    return (Jsi_Chmod(interp, fname, (unsigned int)fmod) == 0 ? JSI_OK : JSI_ERROR);
}

Jsi_RC Jsi_FileRead(Jsi_Interp *interp, Jsi_Value *name, Jsi_DString *dStr) {
    Jsi_RC rc = JSI_ERROR;
    Jsi_Channel chan = Jsi_Open(interp, name, "rb");
    int n, sum = 0;
    if (!chan)
        return rc;
    char buf[JSI_BUFSIZ];
    while (sum < MAX_LOOP_COUNT && (n = Jsi_Read(interp, chan, buf, sizeof(buf))) > 0) {
        Jsi_DSAppendLen(dStr, buf, n);
        sum += n;
    }
    Jsi_Close(interp, chan);
    return JSI_OK;
}

static Jsi_RC FileReadCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr) /* TODO limit. */
{   
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vmode = Jsi_ValueArrayIndex(interp, args, 1);
    const char *mode = (vmode ? Jsi_ValueString(interp, vmode, NULL) : NULL);
    SAFEACCESS(fname, 0, 0)

    Jsi_Channel chan = Jsi_Open(interp, fname, (mode ? mode : "rb"));
    int n, sum = 0;
    if (!chan) 
        return Jsi_LogError("failed open for read: %s", GSVal(fname));
    Jsi_DString dStr = {};
    char buf[JSI_BUFSIZ];
    while (sum < MAX_LOOP_COUNT && (n = Jsi_Read(interp, chan, buf, sizeof(buf))) > 0) {
        Jsi_DSAppendLen(&dStr, buf, n);
        sum += n;
    }
    Jsi_Close(interp, chan);
    Jsi_ValueMakeDStringObject(interp, ret, &dStr);
    return JSI_OK;
}


static Jsi_RC FileWriteCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{   
    if (interp->isSafe && interp->safeMode == jsi_safe_Lockdown)
        return Jsi_LogError("disabled by safe mode");
    Jsi_Value *fname = Jsi_ValueArrayIndex(interp, args, 0);
    const char *data;
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *vmode = Jsi_ValueArrayIndex(interp, args, 2);
    const char *mode = (vmode ? Jsi_ValueString(interp, vmode, NULL) : NULL);
    Jsi_Channel chan;
    int n, len, cnt = 0, sum = 0;
    SAFEACCESS(fname, 1, 1)
    if (!(data = Jsi_ValueGetStringLen(interp, v, &len))) {
        return JSI_ERROR;
    }
    chan = Jsi_Open(interp, fname, (mode ? mode : "wb+"));
    if (!chan) 
        return Jsi_LogError("failed open for write: %s", GSVal(fname));
    while (cnt < MAX_LOOP_COUNT && len > 0 && (n = Jsi_Write(interp, chan, data, len)) > 0) {
        len -= n;
        sum += n;
        cnt++;
    }
    Jsi_Close(interp, chan);
    /* TODO: handle nulls. */
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)sum);
    return JSI_OK;
}

static Jsi_RC FileRenameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{   
    if (interp->isSafe && interp->safeMode == jsi_safe_Lockdown)
        return Jsi_LogError("disabled by safe mode");
    Jsi_Value *source = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *dest = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 2);
    int force = 0; 

    SAFEACCESS(source, 1, 0)
    SAFEACCESS(dest, 1, 1)
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) 
        return Jsi_LogError("expected boolean");
    if (vf)
        force = vf->d.val;
    if (force==0 && Jsi_Access(interp, dest, F_OK) == 0) 
        return Jsi_LogError("error renaming \"%s\" to \"%s\": target exists", GSVal(source), GSVal(dest));

    if (Jsi_Rename(interp, source, dest) != 0) 
        return Jsi_LogError( "error renaming \"%s\" to \"%s\": %s", GSVal(source),
            GSVal(dest), strerror(errno));
    return JSI_OK;
}

static int FileCopy(Jsi_Interp *interp, Jsi_Value* src, Jsi_Value* dest, int force) {
    if (interp->isSafe && interp->safeMode == jsi_safe_Lockdown)
        return Jsi_LogError("disabled by safe mode");
    Jsi_Channel ich = Jsi_Open(interp, src, "rb");
    if (!ich)
        return -1;
    if (force && Jsi_Access(interp, dest, F_OK) == 0) {
        Jsi_Remove(interp, dest, 0);
    }
    Jsi_Channel och = Jsi_Open(interp, dest, "wb+");
    if (!och)
        return -1;
    while (1) {
        char buf[JSI_BUFSIZ];
        int n;
        n = Jsi_Read(interp, ich, buf, JSI_BUFSIZ);
        if (n<=0)
            break;
        if (Jsi_Write(interp, och, buf, n) != n) {
            Jsi_Close(interp, ich);
            Jsi_Close(interp, och);
            return -1;
        }
    }
    /* TODO: set perms. */
#ifndef __WIN32
    Jsi_StatBuf sb;
    Jsi_Stat(interp, src, &sb);
    Jsi_Chmod(interp, dest, sb.st_mode);
#endif
    Jsi_Close(interp, ich);
    Jsi_Close(interp, och);
    return 0;
}

#define FN_copy JSI_INFO("\
Directories are not handled.\n\
The third argument if given is a boolean force value \
which if true allows overwrite of an existing file. ")
static Jsi_RC FileCopyCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *source = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *dest = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 2);
    int force = 0; 
    SAFEACCESS(source, 0, 0)
    SAFEACCESS(dest, 1, 1)
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) 
        return Jsi_LogError("expected boolean");
    if (vf)
        force = vf->d.val;
    if (force==0 && Jsi_Access(interp, dest, F_OK) == 0) 
        return Jsi_LogError("error copying \"%s\" to \"%s\": target exists", GSVal(source), GSVal(dest));

    if (FileCopy(interp, source, dest, force) != 0) 
        return Jsi_LogError( "error copying \"%s\" to \"%s\": %s", GSVal(source),
            GSVal(dest), strerror(errno));
    return JSI_OK;
}

#define FN_link JSI_INFO("\
The second argument is the destination file to be created. "\
"If a third bool argument is true, a hard link is created.")
static Jsi_RC FileLinkCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *source = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *dest = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 2);
    int hard = 0; 
    SAFEACCESS(source, 1, 0)
    SAFEACCESS(dest, 1, 1)
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) 
        return Jsi_LogError("expected boolean");
    if (vf)
        hard = vf->d.val;

    if (Jsi_Link(interp, source, dest, hard) != 0) 
        return Jsi_LogError( "error linking \"%s\" to \"%s\": %s", GSVal(source),
            GSVal(dest), strerror(errno));
    return JSI_OK;
}

/* TODO: switch to MatchesInDir */
static Jsi_RC RmdirAll(Jsi_Interp *interp, Jsi_Value *path, int force)
{
    DIR *dir;
    struct dirent *entry;
    char spath[PATH_MAX];
    Jsi_RC erc = JSI_OK;
    char *dirname = Jsi_ValueString(interp, path, 0);
    if (!dirname) 
        return Jsi_LogError("expected string");

    /* TODO: change to Jsi_Scandir */
    dir = opendir(dirname);
    if (dir == NULL) {
        if (force)
            return JSI_OK;
        return Jsi_LogError("opening directory: %s", dirname);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (Jsi_Strcmp(entry->d_name, ".") && Jsi_Strcmp(entry->d_name, "..")) {
            snprintf(spath, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
            Jsi_Value *tpPtr = Jsi_ValueNew1(interp);
            Jsi_ValueMakeStringDup(interp, &tpPtr, spath);
#ifndef __WIN32
            if (entry->d_type == DT_DIR) {
                Jsi_RC rc = RmdirAll(interp, tpPtr, force);
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

static Jsi_RC FileRemoveCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (interp->isSafe && interp->safeMode == jsi_safe_Lockdown)
        return Jsi_LogError("disabled by safe mode");
    Jsi_Value *source = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *vf = Jsi_ValueArrayIndex(interp, args, 1);
    const char *spath = GSVal(source);
    int rc, force = 0; 
    SAFEACCESS(source, 1, 0)

    Jsi_StatBuf st;
    if (vf && !Jsi_ValueIsBoolean(interp, vf)) 
        return Jsi_LogError("expected boolean");
    if (vf)
        force = vf->d.val;
    rc = Jsi_Stat(interp, source, &st);
    if (Jsi_Strcmp(spath,"/")==0)
        return JSI_ERROR;
    if (rc != 0) {
        if (force)
            return JSI_OK;
        return Jsi_LogError("error deleting \"%s\": target not found", spath);
    }

    if (Jsi_Remove(interp, source, force) != 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (force)
                return JSI_OK;
            return Jsi_LogError("error deleting \"%s\"", spath);
        }
        if (force==0) 
            return Jsi_LogError("error deleting \"%s\": directory not empty", spath);
        return RmdirAll(interp, source, force);
    }
    return JSI_OK;
}

static Jsi_RC FileReadlinkCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    SAFEACCESS(path, 0, 0)

    char *linkValue = (char*)Jsi_Malloc(MAXPATHLEN + 1);

    int linkLength = Jsi_Readlink(interp, path, linkValue, MAXPATHLEN);

    if (linkLength == -1) {
        Jsi_Free(linkValue);
        return Jsi_LogError("couldn't readlink \"%s\": %s", GSVal(path), strerror(errno));
    }
    linkValue[linkLength] = 0;
    Jsi_ValueMakeString(interp, ret, linkValue);
    return JSI_OK;
}

static Jsi_RC FileIsRelativeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = Jsi_ValueString(interp, Jsi_ValueArrayIndex(interp, args, 0), NULL);
    if (path == NULL) 
        return Jsi_LogError("expected string");
    bool isRel = 1;
#if defined(__MINGW32__) || defined(_MSC_VER)
    if ((path[0] && path[1] == ':') || (path[0] == '\\'))
        isRel = 0;
    else
#endif
        isRel = (path[0] != '/');
    Jsi_ValueMakeBool(interp, ret, isRel);
    return JSI_OK;
}

static Jsi_RC FileDirnameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *val = Jsi_ValueArrayIndex(interp, args, 0);
    const char *path = Jsi_ValueString(interp, val, NULL);
    if (!path) 
        return Jsi_LogError("expected string");
    const char *p = Jsi_Strrchr(path, '/');

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
        int olen = -1;
        char *ostr = jsi_SubstrDup(path, -1, 0, p-path+1, &olen);
        Jsi_ValueMakeBlob(interp, ret, (uchar*)ostr, olen);
    }
#endif
    else {
        int olen = -1;
        char *ostr = jsi_SubstrDup(path, -1, 0, p-path, &olen);
        Jsi_ValueMakeBlob(interp, ret, (uchar*)ostr, olen);
    }
    return JSI_OK;
}

static Jsi_RC FileRootnameCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = Jsi_ValueString(interp, Jsi_ValueArrayIndex(interp, args, 0), NULL);
    if (path == NULL) 
        return Jsi_LogError("expected string");
    const char *lastSlash = Jsi_Strrchr(path, '/');
    const char *p = Jsi_Strrchr(path, '.');

    if (p == NULL || (lastSlash != NULL && lastSlash > p)) {
        Jsi_ValueMakeStringDup(interp, ret, path);
    }
    else {
        int olen = -1;
        char *ostr = jsi_SubstrDup(path, -1, 0, p-path, &olen);
        Jsi_ValueMakeBlob(interp, ret, (uchar*)ostr, olen);
    }
    return JSI_OK;
}
static Jsi_RC FileExtensionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = Jsi_ValueString(interp, Jsi_ValueArrayIndex(interp, args, 0), NULL);
    if (path == NULL) 
        return Jsi_LogError("expected string");

    const char *lastSlash = Jsi_Strrchr(path, '/');
    const char *p = Jsi_Strrchr(path, '.');

    if (p == NULL || (lastSlash != NULL && lastSlash >= p)) {
        p = "";
    }
    Jsi_ValueMakeStringDup(interp, ret, p);
    return JSI_OK;

}
static Jsi_RC FileTailCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *path = Jsi_ValueString(interp, Jsi_ValueArrayIndex(interp, args, 0), NULL);
    if (path == NULL) 
        return Jsi_LogError("expected string");
    const char *lastSlash = Jsi_Strrchr(path, '/');

    if (lastSlash) {
        Jsi_ValueMakeStringDup(interp, ret, lastSlash+1);
    }
    else {
        Jsi_ValueMakeStringDup(interp, ret, path);
    }
    return JSI_OK;
}

static Jsi_RC FileRealpathCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    char *newname = Jsi_FileRealpath(interp, path, NULL);
    if (newname)
        Jsi_ValueMakeString(interp, ret, (char*)newname);
    return JSI_OK;

}

static Jsi_RC FileJoinCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *p2 = Jsi_ValueArrayIndex(interp, args, 1);
    const char *path1 = Jsi_ValueString(interp, Jsi_ValueArrayIndex(interp, args, 0), NULL);
    const char *path2 = Jsi_ValueString(interp, p2, NULL);
    if (path1 == NULL || path2 == NULL) 
        return Jsi_LogError("expected string");

    char *newname;
    if (*path2 == '/' || *path2 == '~')
        newname = Jsi_FileRealpath(interp, p2, NULL);
    else {
        Jsi_DString dStr = {};
        Jsi_DSAppend(&dStr, path1, "/", path2, NULL);
        Jsi_Value *tpPtr = Jsi_ValueNew1(interp);
        Jsi_ValueFromDS(interp, &dStr, &tpPtr);
        newname = Jsi_FileRealpath(interp, tpPtr, NULL);
        Jsi_DecrRefCount(interp, tpPtr);
    }
    if (newname ==  NULL)
        return JSI_ERROR;
    Jsi_ValueMakeString(interp, ret, (char*)newname);
    return JSI_OK;
}

static Jsi_RC FileSizeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *path = Jsi_ValueArrayIndex(interp, args, 0);
    int rc;
    Jsi_StatBuf st;
    SAFEACCESS(path, 0, 0)
    if (Jsi_ValueArrayIndex(interp, args, 1))
        rc = Jsi_Lstat(interp, path, &st);
    else
        rc = Jsi_Stat(interp, path, &st);
    if (rc != 0) 
        return Jsi_LogError("file access failed: %s", GSVal(path));
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)st.st_size);
    return JSI_OK;
}

typedef struct {
    int limit;            /* Max number of results to return. */
    bool recurse;        /* Recurse into directories. */
    bool tails;          /* Return tail of path. */
    int maxDepth;       /* For recursive */
    int flags;
    bool retCount;
    bool retInfo;
    int cnt;            /* Actual count. */
    int discardCnt;
    int maxDiscard;
    const char *types;   /* File types to include */
    const char *noTypes; /* File types to exclude */
    Jsi_Value *filter;    /* Function that returns true to keep. */
    Jsi_Value *dirFilter; /* Function that returns true to recurse into dir. */
    Jsi_Value *dir;
    const char *dirStr;
    int dirLen;
    const char *prefix;
} GlobData;

static Jsi_OptionSpec GlobOptions[] = {
    JSI_OPT(STRING, GlobData, dir,      .help="The start directory: this path will not be prepended to results"),
    JSI_OPT(INT,    GlobData, maxDepth, .help="Maximum directory depth to recurse into"),
    JSI_OPT(INT,    GlobData, maxDiscard,.help="Maximum number of items to discard before giving up"),
    JSI_OPT(FUNC,   GlobData, dirFilter,.help="Filter function for directories, returning false to discard", .flags=0, .custom=0, .data=(void*)"dir:string"),
    JSI_OPT(FUNC,   GlobData, filter,   .help="Filter function to call with each file, returning false to discard", .flags=0, .custom=0, .data=(void*)"file:string"),
    JSI_OPT(INT,    GlobData, limit,    .help="The maximum number of results to return/count: -1 is unlimited (Interp.maxArrayList)"),
    JSI_OPT(STRKEY, GlobData, noTypes,  .help="Filter files to exclude these \"types\""),
    JSI_OPT(STRKEY, GlobData, prefix,   .help="String prefix to prepend to each file in result list"),
    JSI_OPT(BOOL,   GlobData, recurse,  .help="Recurse into sub-directories"),
    JSI_OPT(BOOL,   GlobData, retCount, .help="Return only the count of matches"),
    JSI_OPT(BOOL,   GlobData, retInfo,  .help="Return file info: size, uid, gid, mode, name, and path"),
    JSI_OPT(BOOL,   GlobData, tails,    .help="Returned only tail of path"),
    JSI_OPT(STRKEY, GlobData, types,    .help="Filter files to include type: one or more of chars 'fdlpsbc' for file, directory, link, etc"),
    JSI_OPT_END(GlobData, .help="Glob options")
};

static Jsi_RC SubGlobsDirectory(Jsi_Interp *interp, Jsi_Obj* obj, Jsi_Value *reg,
    const char *zPattern, const char* path, GlobData *opts, int deep,
    int cnt, Jsi_Value* _this)
{
    if (cnt>interp->maxIncDepth || !path)
        return Jsi_LogError("runaway File.globs");
    struct dirent **namelist;
    char pbuf[PATH_MAX];
    Jsi_RC rc = JSI_OK;
    int i, n, flags = opts->flags, slen;
    bool bres = 0;
    Jsi_DString tStr = {}, pStr = {}, eStr = {}, zStr = {};
    Jsi_Value *rpPath = Jsi_ValueNew1(interp);
    const char *mid = NULL;
    const char *ssp, *sspp, *esp=NULL, *star, *spath = path;
    if (opts->recurse || reg) {
        Jsi_ValueMakeStringDup(interp, &rpPath, (spath&&*spath?spath:"."));
    } else {
        if (!*spath || (*spath=='*' && !spath[1]))
            spath = "./*";
        slen = Jsi_Strlen(spath);
        star = Jsi_Strchr(spath, '*');
        if (!star) {
            ssp=Jsi_Strrchr(spath, '/');
            if (!ssp) {
                zPattern = spath;
                spath = ".";
            } else {
                zPattern = ssp+1;
                spath=Jsi_DSAppendLen(&pStr, spath, ssp-spath);
            }
        } else {
            if ((ssp=Jsi_Strchr(star, '/'))) {
                mid = Jsi_DSAppend(&eStr, ssp, NULL);
                esp = ssp-1;
                /*if (sspp[1]=='/' && (star == spath || star[-1]=='/'))
                    spath=Jsi_DSAppendLen(&tStr, spath, sspp-spath);
                else
                    spath=Jsi_DSAppend(&tStr, ".");*/
            } else
                esp = spath+slen-1;
                
            ssp = star;
            while (ssp>spath && *--ssp!='/');
            sspp = (*ssp=='/'?ssp+1:ssp);
            zPattern=Jsi_DSAppendLen(&zStr, sspp, esp-sspp+1);
            int len = (ssp-spath);
            if (len==0)
                spath = "";
            else {
                if (len>0 && spath[len]=='/') len--;
                spath=Jsi_DSAppendLen(&pStr, spath, len+1);
            }
        }
        Jsi_ValueMakeStringDup(interp, &rpPath, (*spath?spath:"."));
        if (!*path || (*path=='*' && !path[1]))
            spath = "";
    }
    /*const char *dcp = Jsi_FileRealpath(interp, rpPath, NULL);
    if (dcp) {
        Jsi_ValueMakeString(interp, &rpPath, dcp);
    }*/
    if ((n=Jsi_Scandir(interp, rpPath, &namelist, 0, 0)) < 0) {
        if (opts->recurse && deep) return JSI_OK;
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
        if (ftyp == DT_LNK) {
            if (opts->noTypes && (!(flags&JSI_FILE_TYPE_LINK)))
                continue;
            snprintf(pbuf, sizeof(pbuf), "%s%s%s", spath, (spath[0]?"/":""),  z);
            Jsi_StatBuf stat;
            Jsi_Value *vpath = Jsi_ValueNewStringConst(interp, pbuf, -1);
            Jsi_IncrRefCount(interp, vpath);
            int sc = Jsi_Stat(interp, vpath, &stat);
            Jsi_DecrRefCount(interp, vpath);
            if (sc) continue;
            if (S_ISDIR(stat.st_mode))
                ftyp = DT_DIR;
        }
        if (ftyp != DT_DIR) {
            if (!(flags&JSI_FILE_TYPE_FILES) || mid)
                continue;
        } else {
            if (mid) {
                if (zPattern != NULL && Jsi_GlobMatch(zPattern, z, 0) == 0)
                    continue;
                Jsi_DString sStr = {};
                Jsi_DSAppend(&sStr, spath, "/", z, mid, NULL);
                rc = SubGlobsDirectory(interp, obj, NULL, NULL, Jsi_DSValue(&sStr), opts, deep, cnt+1, _this);
                Jsi_DSFree(&sStr);
                continue;
            }
            int maxd = opts->maxDepth;
            if (maxd<=0) maxd = interp->maxIncDepth;
            if (opts->recurse && ((deep+1) <= maxd)) {
                const char *zz;
                Jsi_DString sStr = {};
                Jsi_DSAppend(&sStr, spath, (spath[0]?"/":""),  z, NULL);
                if (opts->dirFilter && Jsi_ValueIsFunction(interp, opts->dirFilter)) {
                    bres=Jsi_FunctionInvokeBool(interp, opts->dirFilter,
                        Jsi_ValueNewStringDup(interp, Jsi_DSValue(&sStr)), _this);
                    if (Jsi_InterpGone(interp)) {
                        rc = JSI_ERROR;
                        goto done;
                    }
                    if (!bres)
                        continue;
                    else if (bres!=0 && bres!=1)
                        rc = JSI_ERROR;
                }
                if (opts->types && Jsi_Strchr(opts->types, 'd'))
                    goto dumpit;
                zz = Jsi_DSValue(&sStr);
                rc = SubGlobsDirectory(interp, obj, reg, zPattern, zz, opts, deep+1, cnt+1, _this);
                Jsi_DSFree(&sStr);
                if (opts->limit>0 && opts->cnt >= opts->limit)
                    goto done;
                if (opts->maxDiscard>0 && opts->discardCnt >= opts->maxDiscard)
                    goto done;
                if (rc != JSI_OK)
                    goto done;
            }
            if (opts->types==0 && opts->noTypes && (!(flags&JSI_FILE_TYPE_DIRS)))
                continue;
        }
        // TODO: sanity check types/noTypes.
        if (opts->types) {
            const char *cp = opts->types;
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
        if (opts->noTypes) {
            const char *cp = opts->noTypes;
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

        if (reg) {
           int ismat;
            Jsi_RegExpMatch(interp, reg, z, &ismat, NULL);
            if (!ismat)
                continue;
        } else if (zPattern != NULL && Jsi_GlobMatch(zPattern, z, 0) == 0)
            continue;
        if (opts->filter && Jsi_ValueIsFunction(interp, opts->filter)) {
            Jsi_Value *nval = Jsi_ValueNewStringDup(interp, z);
            Jsi_IncrRefCount(interp, nval);
            bres=Jsi_FunctionInvokeBool(interp, opts->filter, nval, _this);
            if (Jsi_InterpGone(interp)) {
                rc = JSI_ERROR;
                goto done;
            }
            Jsi_DecrRefCount(interp, nval);
            if (!bres) {
                opts->discardCnt++;
                continue;
            } else if (bres!=0 && bres!=1)
                rc = JSI_ERROR;
        }
dumpit:
        opts->cnt++;
        if (!opts->retCount) {
            Jsi_DSSetLength(&tStr, 0);
            if (opts->prefix)
                Jsi_DSAppend(&tStr, opts->prefix, NULL);
            if (!opts->tails)
                Jsi_DSAppend(&tStr, spath, (spath[0]?"/":""), NULL);
            Jsi_DSAppend(&tStr, z, NULL);
            z = Jsi_DSValue(&tStr);
            if (!opts->tails && opts->dirLen && Jsi_Strlen(z)>=(uint)opts->dirLen) {
                z += opts->dirLen;
                if (z[0] == '/') z++;
            }
            Jsi_Value *nv;
            if (!opts->retInfo)
                nv = Jsi_ValueNewStringDup(interp, z);
            else {
                Jsi_Value *info = Jsi_ValueNew1(interp);
                snprintf(pbuf, sizeof(pbuf), "%s%s%s", spath, (spath[0]?"/":""),  z);
                Jsi_Value *vpath = Jsi_ValueNewStringConst(interp, pbuf, -1);
                Jsi_IncrRefCount(interp, vpath);
                rc = jsi_FileStatCmd(interp, vpath, &info, 2);
                Jsi_DecrRefCount(interp, vpath);
                if (rc != JSI_OK) {
                    Jsi_DecrRefCount(interp, info);
                    break;
                }
                nv = Jsi_ValueNewStringDup(interp, z);
                Jsi_ObjInsert(interp, info->d.obj, "name", nv, 0);
                if (opts->recurse)
                    Jsi_ObjInsert(interp, info->d.obj, "path", (spath[0]?Jsi_ValueNewStringDup(interp, pbuf):nv), 0);
                nv = info;
            }
            rc = Jsi_ObjArrayAdd(interp, obj, nv);
            if (opts->retInfo)
                Jsi_DecrRefCount(interp, nv);
        }
        if (opts->limit>0 && opts->cnt >= opts->limit)
            break;
        if (opts->maxDiscard>0 && opts->discardCnt >= opts->maxDiscard)
            break;
    }

done:
    Jsi_DSFree(&tStr); Jsi_DSFree(&pStr); Jsi_DSFree(&eStr); Jsi_DSFree(&zStr);
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
With no arguments (or null) returns all files/directories in current directory.\n\
The first argument can be a pattern (either a glob or regexp) of the files to return.\n\
When the second argument is a function, it is called with each path, and filter on false.\n\
Otherwise second argument must be a set of options.")
static Jsi_RC FileGlobsCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int fo = 1, isOpts = 0;
    Jsi_RC rc = JSI_OK;
    Jsi_Value *pat = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *arg = Jsi_ValueArrayIndex(interp, args, 1);
    GlobData Data = {};
    Jsi_Obj *obj = NULL;
    //int len;
    //char *zPattern = NULL;
    const char *dcp, *zPattern = NULL;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    
    Data.flags = JSI_FILE_TYPE_FILES;

    if (arg)
        switch (arg->vt) {
        case JSI_VT_NULL: break;
        case JSI_VT_OBJECT:
        {
            Jsi_Obj *sobj = arg->d.obj;
            switch (sobj->ot) {
                case JSI_OT_FUNCTION:
                    Data.filter = arg;
                    break;
                case JSI_OT_OBJECT:
                    if (!sobj->isarrlist) {
                        isOpts = 1;
                        break;
                    }
                default: fo = 0;
            }
            if (fo) break;
        }
        default:
            rc = Jsi_LogError("arg2 must be a function, object or null");
            goto done;
    }
    if (isOpts && Jsi_OptionsProcess(interp, GlobOptions, &Data, arg, 0) < 0) {
        rc = JSI_ERROR;
        goto done;
    }
    if (!Data.limit)
        Data.limit = interp->maxArrayList;
    if (Data.dir) {
        dcp = Data.dirStr = Jsi_ValueString(interp, Data.dir, &Data.dirLen);
        if (*dcp == '~') {
            dcp = Jsi_FileRealpath(interp, Data.dir, NULL);
            if (!dcp)
                dcp = Data.dirStr;
            else
                Data.dirLen = Jsi_Strlen(dcp);
        }
        Jsi_DSAppend(&dStr, dcp, (*dcp && dcp[Jsi_Strlen(dcp)-1]!='/')?"/":"", NULL);
    }
    if (Data.retCount) {
        if (Data.retInfo) {
            rc = Jsi_LogError("Can not use both retCount and retInfo");
            goto done;
        }
    } else {
        obj = Jsi_ObjNew(interp);
        Jsi_ValueMakeArrayObject(interp, ret, obj);
    }
    if (pat == NULL || Jsi_ValueIsNull(interp, pat))
        dcp = "*";
    else if (Jsi_ValueIsObjType(interp, pat, JSI_OT_REGEXP))
        rc = SubGlobsDirectory(interp, obj, pat, NULL, dcp, &Data, 0, 0, _this);
    else {
        dcp = Jsi_ValueString(interp, pat, NULL);
        if (!dcp) {
            rc = Jsi_LogError("expected string, regex or null");
            goto done;
        }
    }
    if (!*dcp)
        dcp = "*";
    if (Data.recurse) {
        if (Jsi_Strchr(dcp, '/')) {
            rc = Jsi_LogError("patern may can not contain '/' with recurse");
            goto done;
        }
        if (Data.dir) {
            zPattern = dcp;
            dcp = Jsi_ValueString(interp, Data.dir, NULL);
        }
    } else
        dcp = Jsi_DSAppend(&dStr, dcp, NULL);
        
    rc = SubGlobsDirectory(interp, obj, NULL, zPattern, dcp, &Data, 0, 0, _this);
    if (rc != JSI_OK)
        Jsi_ValueMakeUndef(interp, ret);   
    else if (Data.retCount)
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)Data.cnt);

done:
    Jsi_DSFree(&dStr);
    if (isOpts)
        Jsi_OptionsFree(interp, GlobOptions, &Data, 0);
    return rc;
}

static Jsi_CmdSpec fileCmds[] = {
    { "atime",      FileAtimeCmd,       1,  1, "file:string",  .help="Return file Jsi_Access time", .retType=(uint)JSI_TT_NUMBER },
    { "chdir",      FileChdirCmd,       1,  1, "file:string",  .help="Change current directory" },
    { "chmod",      FileChmodCmd,       2,  2, "file:string, mode:number",  .help="Set file permissions" },
    { "copy",       FileCopyCmd,        2,  3, "src:string, dest:string, force:boolean=false",  .help="Copy a file to destination", .retType=0, .flags=0, .info=FN_copy },
    { "dirname",    FileDirnameCmd,     1,  1, "file:string",  .help="Return directory path", .retType=(uint)JSI_TT_STRING },
    { "executable", FileExecutableCmd,  1,  1, "file:string",  .help="Return true if file is executable", .retType=(uint)JSI_TT_BOOLEAN },
    { "exists",     FileExistsCmd,      1,  1, "file:string",  .help="Return true if file exists", .retType=(uint)JSI_TT_BOOLEAN },
    { "extension",  FileExtensionCmd,   1,  1, "file:string",  .help="Return file extension", .retType=(uint)JSI_TT_STRING },
    { "join",       FileJoinCmd,        2,  2, "path1:string, path2:string",  .help="Join two file realpaths, or just second if an absolute path", .retType=(uint)JSI_TT_STRING },
    { "isdir",      FileIsdirCmd,       1,  1, "file:string",  .help="Return true if file is a directory", .retType=(uint)JSI_TT_BOOLEAN },
    { "isfile",     FileIsfileCmd,      1,  1, "file:string",  .help="Return true if file is a normal file", .retType=(uint)JSI_TT_BOOLEAN },
    { "isrelative", FileIsRelativeCmd,  1,  1, "file:string",  .help="Return true if file path is relative", .retType=(uint)JSI_TT_BOOLEAN },
    { "glob",       FileGlobsCmd,        0,  2, "pattern:regexp|string|null='*', options:function|object|null=void", .help="Return list of files in dir with optional pattern match", .retType=(uint)JSI_TT_ARRAY, .flags=0, .info=FN_glob, .opts=GlobOptions },
    { "link",       FileLinkCmd,        2,  3, "src:string, dest:string, ishard:boolean=false",  .help="Link a file", .retType=0, .flags=0, .info=FN_link },
    { "lstat",      FileLstatCmd,       1,  1, "file:string",  .help="Return status info for file", .retType=(uint)JSI_TT_OBJECT },
    { "mkdir",      FileMkdirCmd,       1,  2, "file:string,force:boolean=false",  .help="Create a directory: force creates subdirs" },
    { "mknod",      FileMknodCmd,       3,  3, "file:string, mode:number, dev:number", .help="Create unix device file using mknod"  },
    { "mtime",      FileMtimeCmd,       1,  1, "file:string",  .help="Return file modified time", .retType=(uint)JSI_TT_NUMBER },
    { "owned",      FileOwnedCmd,       1,  1, "file:string",  .help="Return true if file is owned by user", .retType=(uint)JSI_TT_BOOLEAN },
    { "perms",      FilePermsCmd,       1,  1, "file:string",  .help="Return perms string", .retType=(uint)JSI_TT_STRING },
    { "pwd",        FilePwdCmd,         0,  0, "",  .help="Return current directory", .retType=(uint)JSI_TT_STRING },
    { "remove",     FileRemoveCmd,      1,  2, "file:string, force:boolean=false",  .help="Delete a file or direcotry" },
    { "rename",     FileRenameCmd,      2,  3, "src:string, dest:string, force:boolean=false",  .help="Rename a file, with possible overwrite" },
    { "read",       FileReadCmd,        1,  2, "file:string, mode:string='rb'",  .help="Read a file", .retType=(uint)JSI_TT_STRING },
    { "readable",   FileReadableCmd,    1,  1, "file:string",  .help="Return true if file is readable", .retType=(uint)JSI_TT_BOOLEAN },
    { "readlink",   FileReadlinkCmd,    1,  1, "file:string",  .help="Read file link destination", .retType=(uint)JSI_TT_STRING },
    { "realpath",   FileRealpathCmd,    1,  1, "file:string",  .help="Return absolute file name minus .., ./ etc", .retType=(uint)JSI_TT_STRING },
    { "rootname",   FileRootnameCmd,    1,  1, "file:string",  .help="Return file name minus extension", .retType=(uint)JSI_TT_STRING },
    { "size",       FileSizeCmd,        1,  1, "file:string",  .help="Return size for file", .retType=(uint)JSI_TT_NUMBER },
    { "stat",       FileStatCmd,        1,  1, "file:string",  .help="Return status info for file", .retType=(uint)JSI_TT_OBJECT },
    { "tail",       FileTailCmd,        1,  1, "file:string",  .help="Return file name minus dirname", .retType=(uint)JSI_TT_STRING },
    { "tempfile",   FileTempfileCmd,    1,  1, "file:string",  .help="Create a temp file", .retType=(uint)JSI_TT_ANY },
    { "truncate",   FileTruncateCmd,    2,  2, "file:string, size:number",  .help="Truncate file" },
    { "write",      FileWriteCmd,       2,  3, "file:string, str:string, mode:string='wb+'",  .help="Write a file", .retType=(uint)JSI_TT_NUMBER },
    { "writable",   FileWritableCmd,    1,  1, "file:string",  .help="Return true if file is writable", .retType=(uint)JSI_TT_BOOLEAN },
    { NULL, 0,0,0,0, .help="Commands for accessing the filesystem" }
};

Jsi_RC jsi_InitFileCmds(Jsi_Interp *interp, int release)
{
    if (!release)
        Jsi_CommandCreateSpecs(interp, "File",   fileCmds,   NULL, 0);
    return JSI_OK;
}
#endif
#endif
