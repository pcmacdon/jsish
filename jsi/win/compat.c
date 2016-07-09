#ifdef __WIN32
#ifndef JSI_AMALGAMATION
#include "../jsiInt.h"
#endif

#ifndef STRICT
#define STRICT
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#define HAVE_DLOPEN_COMPAT
#if defined(HAVE_DLOPEN_COMPAT)
void *dlopen(const char *path, int mode)
{
    mode=mode;

    return (void *)LoadLibraryA(path);
}

int dlclose(void *handle)
{
    FreeLibrary((HANDLE)handle);
    return 0;
}

void *dlsym(void *handle, const char *symbol)
{
    return GetProcAddress((HMODULE)handle, symbol);
}

char *dlerror(void)
{
    static char msg[121];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                   LANG_NEUTRAL, msg, sizeof(msg) - 1, NULL);
    return msg;
}
#endif

#ifdef _MSC_VER

#include <sys/timeb.h>

/* POSIX gettimeofday() compatibility for WIN32 */
int gettimeofday(struct timeval *tv, void *unused)
{
    struct _timeb tb;

    _ftime(&tb);
    tv->tv_sec = tb.time;
    tv->tv_usec = tb.millitm * 1000;

    return 0;
}

/* Posix dirent.h compatiblity layer for WIN32.
 * Copyright Kevlin Henney, 1997, 2003. All rights reserved.
 * Copyright Salvatore Sanfilippo ,2005.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that this copyright and permissions notice appear in all copies and
 * derivatives.
 *
 * This software is supplied "as is" without express or implied warranty.
 * This software was modified by Salvatore Sanfilippo for the Jsi Interpreter.
 */

DIR *opendir(const char *name)
{
    DIR *dir = 0;

    if (name && name[0]) {
        size_t base_length = strlen(name);
        const char *all =       /* search pattern must end with suitable wildcard */
            strchr("/\\", name[base_length - 1]) ? "*" : "/*";

        if ((dir = (DIR *) malloc(sizeof *dir)) != 0 &&
            (dir->name = (char *)malloc(base_length + strlen(all) + 1)) != 0) {
            strcat(strcpy(dir->name, name), all);

            if ((dir->handle = (long)_findfirst(dir->name, &dir->info)) != -1)
                dir->result.d_name = 0;
            else {              /* rollback */
                free(dir->name);
                free(dir);
                dir = 0;
            }
        }
        else {                  /* rollback */
            free(dir);
            dir = 0;
            errno = ENOMEM;
        }
    }
    else {
        errno = EINVAL;
    }
    return dir;
}

int closedir(DIR * dir)
{
    int result = -1;

    if (dir) {
        if (dir->handle != -1)
            result = _findclose(dir->handle);
        free(dir->name);
        free(dir);
    }
    if (result == -1)           /* map all errors to EBADF */
        errno = EBADF;
    return result;
}

struct dirent *readdir(DIR * dir)
{
    struct dirent *result = 0;

    if (dir && dir->handle != -1) {
        if (!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1) {
            result = &dir->result;
            result->d_name = dir->info.name;
        }
    }
    else {
        errno = EBADF;
    }
    return result;
}

#endif

int scandir(const char *dirname,
            struct dirent ***namelist,
            int (*doselect)(const struct dirent *),
            int (*compar)(const struct dirent **,
                          const struct dirent **))
{
    WIN32_FIND_DATA wfd;
    HANDLE hf;
    struct dirent **plist, **newlist;
    struct dirent d;
    int numentries = 0;
    int allocentries = 255;
    int i;
    char path[FILENAME_MAX];
    i = strlen(dirname);
    if (i > sizeof path - 5)
        return -1;
    strcpy(path, dirname);
    if (i>0 && dirname[i-1]!='\\' && dirname[i-1]!='/')
        strcat(path, "\\");
    strcat(path, "*.*");
    hf = FindFirstFile(path, &wfd);
    if (hf == INVALID_HANDLE_VALUE)
        return -1;
    plist = malloc(sizeof *plist * allocentries);
    if (plist==NULL)
    {
        FindClose(hf);
        return -1;
    }
    do
    {
        if (numentries==allocentries)
        {
            allocentries *= 2;
            newlist = realloc(plist, sizeof *plist * allocentries);
            if (newlist==NULL)
            {
                for (i=0; i<numentries; i++)
                    free(plist[i]);
                free(plist);
                FindClose(hf);
                return -1;
            }
            plist = newlist;
        }
        strncpy(d.d_name, wfd.cFileName, sizeof d.d_name);
        /* HACK. if is directory, set inode to 1. */
        d.d_ino =( (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?1:0);
        d.d_namlen = strlen(wfd.cFileName);
        d.d_reclen = sizeof d;
        if (doselect==NULL || doselect(&d))
        {
            plist[numentries] = malloc(sizeof d);
            if (plist[numentries]==NULL)
            {
                for (i=0; i<numentries; i++)
                    free(plist[i]);
                free(plist);
                FindClose(hf);
                return -1;
            };
            memcpy(plist[numentries], &d, sizeof d);
            numentries++;
        }
    }
    while (FindNextFile(hf, &wfd));
    FindClose(hf);
    if (numentries==0)
    {
        free(plist);
        *namelist = NULL;
    }
    else
    {
        newlist = realloc(plist, sizeof *plist * numentries);
        if (newlist!=NULL)
            plist = newlist;
        if (compar!=NULL)
            qsort(plist, numentries, sizeof *plist, (void*)compar);
        *namelist = plist;
    }
    return numentries;
}

int istrcmp(const char *s1, const char *s2)
{
    int d;
    for (;;)
    {
        d = tolower(*s1) - tolower(*s2);
        if (d!=0 || *s1=='\0' || *s2=='\0')
            return d;
        s1++;
        s2++;
    }
}

int alphasort(const struct dirent **d1,
              const struct dirent **d2)
{
    return istrcmp((*d1)->d_name, (*d2)->d_name);
}
#endif
