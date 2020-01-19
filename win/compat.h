#if defined(_WIN32) || defined(WIN32)
#ifndef JSI_WIN32COMPAT_H
#define JSI_WIN32COMPAT_H

typedef unsigned int uint;
/* TODO: bring in external regex ..
typedef struct { int n; } regex_t;
typedef struct { int n; } regmatch_t; */

enum {
    DT_UNKNOWN = 0, DT_FIFO = 1, DT_CHR = 2, DT_DIR = 4,
    DT_BLK = 6, DT_REG = 8, DT_LNK = 10, DT_SOCK = 12, DT_WHT = 14
};


/* Compatibility for Windows (mingw and msvc, not cygwin */

#include <time.h>
#include <dirent.h>
char * strptime(const char *buf, const char *fmt, struct tm *tm);

#define MAXNAMLEN FILENAME_MAX 
int scandir( const char *dirname, struct dirent ***namelist, int (*select)(const struct dirent *), 
     int (*compar)( const struct dirent **, const struct dirent ** ) 
);
int alphasort( const struct dirent **d1, const struct dirent **d2 ); 
int istrcmp( const char *s1, const char *s2 ); 

#define HAVE_DLOPEN
void *dlopen(const char *path, int mode);
int dlclose(void *handle);
void *dlsym(void *handle, const char *symbol);
char *dlerror(void);

extern time_t internal_timegm(struct tm *t);

/* MS CRT always uses three digits after 'e' */
#define JSI_SPRINTF_DOUBLE_NEEDS_FIX

#ifdef _MSC_VER
/* These are msvc vs gcc */

#if _MSC_VER >= 1000
    #pragma warning(disable:4146)
#endif

#include <limits.h>
#define jsi_wide _int64
#ifndef LLONG_MAX
    #define LLONG_MAX    9223372036854775807I64
#endif
#ifndef LLONG_MIN
    #define LLONG_MIN    (-LLONG_MAX - 1I64)
#endif
#define JSI_WIDE_MIN LLONG_MIN
#define JSI_WIDE_MAX LLONG_MAX
#define JSI_WIDE_MODIFIER "I64d"
#define strcasecmp _stricmp
#define strtoull _strtoui64
#define snprintf _snprintf

#include <io.h>

struct timeval {
    long tv_sec;
    long tv_usec;
};

int gettimeofday(struct timeval *tv, void *unused);

#define HAVE_OPENDIR
struct dirent {
    char *d_name;
};

typedef struct DIR {
    long                handle; /* -1 for failed rewind */
    struct _finddata_t  info;
    struct dirent       result; /* d_name null iff first time */
    char                *name;  /* null-terminated char string */
} DIR;

DIR *opendir(const char *name);
int closedir(DIR *dir);
struct dirent *readdir(DIR *dir);
#endif /* _MSC_VER */

#endif


#endif /* WIN32 */
