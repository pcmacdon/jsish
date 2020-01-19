/* Exec in a chrooted subdirectory, optionally as "nobody:nogroup".  MIT Copyright 2019, Peter MacDonald.
   Build with:
      gcc -g -o uchroot uchroot.c && sudo chown root.root uchroot && sudo chmod u+s uchroot
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>

#include <sys/time.h>
#include <sys/resource.h>
extern char **environ;

static int lvals[] = {
    RLIMIT_CORE, RLIMIT_DATA, RLIMIT_FSIZE, RLIMIT_STACK, RLIMIT_CPU, RLIMIT_NPROC, RLIMIT_AS 
};
#define ULIMIT_CHARS "cdfstuv"

static const char* uhelp = "Usage: uchroot [-h -v -nobody -chdir DIR -ulimits LIMS -env ENV]  rootdir prog [args...]\n\n"
"LIMS is a list \"X:N,X:N,...\" where X is one of:\n"
"  c     The maximum size of core files created\n"
"  d     The maximum size of a process's data segment\n"
"  f     The maximum size of files written by the shell and its children\n"
"  s     The maximum stack size\n"
"  t     The maximum amount of cpu time in seconds\n"
"  u     The maximum number of processes available to a single user\n"
"  v     The maximum amount of virtual memory available to the shell and, on some systems, to its children";

static char **envptr = NULL;
static const char *rnams = ULIMIT_CHARS;
#define NUMLVALS (sizeof(lvals)/sizeof(int))
static struct rlimit pres[NUMLVALS] = {};
static int limCnt = 0, test = 0, verb = 0;


int usage(int code, char *msg) {
    fprintf(stderr, "%s%s%s%s\n", (msg && msg[0]?"ERROR: ":""), msg, (msg && msg[0]?"\n":""), uhelp);
    exit(code);
    return(code);
}


void resetlimits() {
    int li;
    for (li=0; rnams[li] && li<limCnt; li++)
        setrlimit(lvals[li], pres+li);
    puts("reset limits");
}

int setlimits(const char *lim) {
    const char *ep = strchr(lim,',');
    char *vep, *lcp;
    unsigned long val;
    struct rlimit r = {};
    int li, lin;
    // First preserve current limits.
    limCnt = 0;
    for (li=0; rnams[li]; li++, limCnt++) {
        if (getrlimit(lvals[li], pres+li))
            return usage(2, "getrlimit failed");
    }
    while (lim) {
        if (!*lim || lim[1] != ':')
            return usage(2, "invalid ulimit");
        val = strtoul(lim+2, &vep, 0);
        if (verb)
            printf("VAL: %c = %lu\n", lim[0], val);
        lcp = strchr(rnams, lim[0]);
        if (!lcp)
            return usage(2, "ulimit char not one of: " ULIMIT_CHARS);
        li = (lcp-rnams);
        lin = lvals[li];
        r = pres[li];
        r.rlim_cur = r.rlim_max = val;
        
        printf("VAL: %c = %lu\n", lim[0], r.rlim_cur);
        if (setrlimit(lin, &r))
            return usage(2, "setrlimit failed");
        lim = ep;
        if (lim)
            ep = strchr(lim,',');
    }
    return 0;
}

void dupenv(char *env) 
{
    int n = 1, m = 0, evlen = strlen(env);
    char *sp = env, *ep = env, *tenv;
    while (*sp && (ep=strchr(sp,';'))) {
        n++;
        sp = ep+1;
    }
    envptr = (char**)malloc((n+2)*sizeof(char**) + evlen+10);
    tenv = (char*)(envptr+n+2);
    strcpy(tenv, env);
    sp = ep = tenv;
    while (m<n) {
        envptr[m] = sp;
        ep=strchr(sp,';');
        if (ep) { *ep = 0; break; }
        sp = ep+1;
        m++;
    }
        
}

int main(int argc, char**argv) {
    int i, nobody = 0;
    const char *cdir = NULL, *limits = NULL;
    char *env = NULL;
    if (argc==2 && !strcmp("-h", argv[1]))
        return usage(0, "");
    for (i=1; i<(argc-1) && argv[i][0] == '-'; i+=2) {
        if (!strcmp("-chdir", argv[i]))
            cdir = argv[i+1];
        else if (!strcmp("-env", argv[i]))
            env = argv[i+1];
        else if (!strcmp("-ulimits", argv[i]))
            limits = argv[i+1];
        else if (!strcmp("-h", argv[i])) {
            return usage(0, "Help for uchroot");
            i--;
        } else if (!strcmp("-v", argv[i])) {
            verb = 1;
            i--;
        } else if (!strcmp("-nobody", argv[i])) {
            nobody = 1;
            i--;
        } else if (!strcmp("-test", argv[i])) {
            test = verb = 1;
            i--;
        } else
            return usage(3, "unknown option");
    }
    if ((argc-i)<2)
        return usage(2,"too few args");
    const char *rootdir = argv[i];
    uid_t oldUid = getgid(), newUid = oldUid;
    gid_t oldGid = getuid(), newGid = oldGid;
    
    if (nobody) {
        struct passwd *pw = getpwnam("nobody");
        struct group *gr = getgrnam("nogroup");
        if (!pw || !gr)
            return usage(4, "failed to get nobody/nogroup");
    }
    
    if (access(rootdir, X_OK)) {
        perror("dir access failed");
        return 6;
    }
    if (geteuid() && getegid() && !test)
        return usage(5, "binary not suid?");
    if (chdir(rootdir)) {
        perror("chdir failed");
        return 5;
    }
    if (cdir && chdir(cdir)) {
        perror("chdir failed");
        return 5;
    }
    struct passwd *pwu = getpwuid(getuid());
    if (!pwu)
        return usage(11, "failed get userid");
    char buf[PATH_MAX];
    buf[0] = 0;
    if (!getcwd(buf, sizeof(buf))) {
        perror("getcwd failed");
        return 9;
    }
    if (strncmp(pwu->pw_dir, buf, strlen(pwu->pw_dir)))
        return usage(12, "must be in home dir");

    if (!test && chroot(buf)) {
        perror("chroot failed");
        return 5;
    }
        
    setegid(newGid);
    seteuid(newUid);
    
    if (getegid() != newGid || geteuid() != newUid)
        return usage(7, "failed to set uid/gid");
    
    if (limits && setlimits(limits)) {
        resetlimits();
        return(1);
    }
    envptr = environ;
    if (env)
        dupenv(env);
    if (execve(argv[i+1], &argv[i+1], envptr) != 0) {
        perror("failed to exec");
        resetlimits();
        return(1);
    }
    resetlimits();
    exit(1);
}
