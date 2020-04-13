#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#include <errno.h>
#include <sys/time.h>


#if (defined(JSI__READLINE) && JSI__READLINE==1)
#define JSI_HAS_READLINE 1
//#define USE_GNU_READLINE
#ifdef USE_GNU_READLINE
# include <readline/readline.h>
# include <readline/history.h>

# define jsi_sh_add_history(X) add_history(X)
# define jsi_sh_read_history(X) read_history(X)
# define jsi_sh_write_history(X) write_history(X)
# define jsi_sh_stifle_history(X) stifle_history(X)
# define jsi_sh_readline(X) readline(X)
#else
#ifndef JSI_AMALGAMATION
#include "linenoise.h"
#endif
# define jsi_sh_add_history(X) linenoiseHistoryAdd(X)
# define jsi_sh_read_history(X) linenoiseHistoryLoad(X)
# define jsi_sh_write_history(X) linenoiseHistorySave(X)
# define jsi_sh_stifle_history(X) linenoiseHistorySetMaxLen(X)
# define jsi_sh_readline(X) linenoise(X)
#endif
#else

# define jsi_sh_read_history(X)
# define jsi_sh_write_history(X)
# define jsi_sh_stifle_history(X)

# define JSI_SH_USE_LOCAL_GETLINE 1
#endif
#include <math.h>

#ifdef Jsi_Free
#undef Jsi_Free
#undef Jsi_Malloc
#undef Jsi_Calloc
#undef Jsi_Realloc
#endif
void* Jsi_Realloc(void *m,unsigned int size) {
    void *v = realloc(m,size);
    Assert(v);
    return v;
}
void* Jsi_Malloc(unsigned int size) {
    void *v = malloc(size);
    Assert(v);
    return v;
}
void* Jsi_Calloc(unsigned int n,unsigned int size) {
    void *v = calloc(n,size);
    Assert(v);
    return v;
}
void  Jsi_Free(void *n) { Assert(n); free(n); }

#if JSI__SANITIZE
#define Jsi_Malloc(sz) malloc(sz)
#define Jsi_Calloc(nm, sz) calloc(nm,sz)
#define Jsi_Realloc(ptr, sz) realloc(ptr,sz)
#define Jsi_Free(ptr) free(ptr)
#endif

static const char *jsi_LogCodes[] = { "bug", "error", "warn", "info", "unused", "parse", "test", "debug", "trace", 0 };
jsi_IntData jsiIntData = {};

#ifdef JSI_LITE_ONLY
Jsi_RC Jsi_LogMsg(Jsi_Interp *interp, uint code, const char *format,...) {
    va_list va;
    va_start (va, format);
    const char *mt = (code <= JSI__LOGLAST ? jsi_LogCodes[code] : "");
    fputs(mt, stderr);
    vfprintf(stderr, format, va);
    fputs("\n", stderr);
    va_end(va);
    return JSI_ERROR;
}

#else // JSI_LITE_ONLY

int jsi_fatalexit = JSI_LOG_BUG;
const char *jsi_GetCurFile(Jsi_Interp *interp)
{
    const char *curFile = NULL;
    if (!interp)
        return NULL;
    if (interp->inParse)
        curFile = interp->curFile;
    else
        curFile = (interp->curIp && interp->curIp->fname? interp->curIp->fname:interp->curFile);
    if (!curFile) curFile = interp->framePtr->fileName;
    if (!curFile) curFile = "";
    return curFile;
}
extern void jsi_TypeMismatch(Jsi_Interp* interp)
{
    interp->typeMismatchCnt++;
    if (interp->typeWarnMax<=0)
        return;
    if (interp->typeMismatchCnt>=interp->typeWarnMax) {
        memset(&interp->typeCheck, 0, sizeof(interp->typeCheck));
        Jsi_LogWarn("Max warnings exceeded %d: typeCheck disabled", interp->typeWarnMax);
    }
}

static bool jsi_LogEnabled(Jsi_Interp *interp, uint code) {
    if (!interp->activeFunc) return 0;
    Jsi_CmdSpec *cs = interp->activeFunc->cmdSpec;
    if (!cs)
        return 0;
    if (interp->activeFunc->parentSpec)
        cs = interp->activeFunc->parentSpec;
    int cofs = (code - JSI_LOG_TEST);
    int ac = (cs->flags & (JSI_CMD_LOG_TEST<<cofs));
    return (ac)!=0;
}

static void (*logHook)(const char *buf, va_list va) = NULL;

// Format message: always returns JSI_ERROR.
Jsi_RC Jsi_LogMsg(Jsi_Interp *interp, uint code, const char *format,...) {
    if (Jsi_InterpGone(interp))
        return JSI_ERROR;
    va_list va;
    va_start (va, format);
    char pbuf[JSI_BUFSIZ/8] = "";
    char buf[JSI_BUFSIZ/2];
    const char *term = "", *pterm=pbuf;
    static char lastMsg[JSI_BUFSIZ/2] = "";
    static int lastCnt = 0;
    static Jsi_Interp *LastInterp = NULL;
    Jsi_Interp *lastInterp = LastInterp;
    const char *emsg = buf, *mt;
    int islog, line = 0, lofs = 0, noDups=0;
    bool isHelp = (format[0]=='.' && !Jsi_Strncmp(format, "...", 3));
    Jsi_OptionSpec *oep = interp->parseMsgSpec;
    const char *pps = "", *curFile = "";
    char *ss = interp->lastPushStr;
    
    if (interp==NULL)
        interp = jsiIntData.mainInterp;
    LastInterp = interp;
    if (lastInterp != interp)
        noDups = 1;
    
    /* Filter out try/catch (TODO: and non-syntax errors??). */
    if (interp == NULL) {
//nullInterp:
        if (logHook)
            (*logHook)(format, va);
        else {
            vfprintf(stderr, format, va);
            fputc('\n', stderr);
        }
        va_end(va);
        return JSI_ERROR;
    }
    curFile = jsi_GetCurFile(interp);
    switch (code) {
        case JSI_LOG_INFO:  if (!interp->logOpts.Info) goto bail; break;
        case JSI_LOG_WARN:  if (!interp->logOpts.Warn) goto bail; break;
        case JSI_LOG_DEBUG: if (!interp->logOpts.Debug && !jsi_LogEnabled(interp, code)) goto bail; break;
        case JSI_LOG_TRACE: if (!interp->logOpts.Trace && !jsi_LogEnabled(interp, code)) goto bail; break;
        case JSI_LOG_TEST:  if (!interp->logOpts.Test && !jsi_LogEnabled(interp, code)) goto bail; break;
        case JSI_LOG_PARSE: break; //if (!interp->parent) goto nullInterp; break;
        case JSI_LOG_ERROR: {
            if (!interp->logOpts.Error) goto bail;
            if ((interp->framePtr->tryDepth - interp->framePtr->withDepth)>0 && interp->inParse<=0 
                && (!interp->tryList || !(interp->tryList->inCatch|interp->tryList->inFinal))) { 
                /* Should only do the first or traceback? */
                if (!interp->errMsgBuf[0]) {
                    vsnprintf(interp->errMsgBuf, sizeof(interp->errMsgBuf), format, va);
                    //interp->errMsgBuf[sizeof(interp->errMsgBuf)-1] = 0;
                    interp->errFile =  jsi_GetCurFile(interp);
                    interp->errLine = (interp->curIp?interp->curIp->Line:0);
                    emsg = interp->errMsgBuf;
                }
                goto done;
            }
            interp->logErrorCnt++;
            break;
        }
    }
    mt = (code <= JSI__LOGLAST ? jsi_LogCodes[code] : "");
    if (isHelp) mt = "help";
    assert((JSI__LOGLAST+2) == (sizeof(jsi_LogCodes)/sizeof(jsi_LogCodes[0])));
    if (!Jsi_Strchr(format,'\n')) term = "\n";
    if (interp->strict && interp->lastParseOpt)
        ss = (char*)Jsi_ValueToString(interp, interp->lastParseOpt, NULL);
    if (code != JSI_LOG_INFO && code < JSI_LOG_TEST && interp && ss && ss[0]) {
        char psbuf[JSI_BUFSIZ/6];
        if (Jsi_Strchr(ss,'%')) {
            char *s = ss, *sd = psbuf;
            int plen=0, llen = sizeof(psbuf)-2;
            while (*s && plen<llen) {
                if (*s == '%')
                    sd[plen++] = '%';
                sd[plen++] = *s;
                s++;
            }
            sd[plen] = 0;
            ss = psbuf;
        }
        while (*ss && isspace(*ss))
            ss++;
        if (*ss && !isHelp)
            snprintf(pbuf, sizeof(pbuf), "    (at or near \"%s\")\n", ss);
    }
    pbuf[sizeof(pbuf)-1] = 0;
    if (interp->inParse && interp->parseLine) {
        line = interp->parseLine->first_line;
        lofs = interp->parseLine->first_column;
    } else if (interp->inParse && interp->parsePs) {
        line = interp->parsePs->lexer->cur_line;
        lofs = interp->parsePs->lexer->cur_char;
    } else if (interp->curIp) {
        if (interp->callerErr && interp->framePtr && interp->framePtr->parent) {
            jsi_Frame *fptr = interp->framePtr->parent;
            line = fptr->line;
            lofs = 0;
            curFile = fptr->fileName;
        } else {
            line = interp->curIp->Line;
            lofs = interp->curIp->Lofs;
            if (line<=0)
                line = interp->framePtr->line;
        }
    }
    islog = (interp->parent && interp->debugOpts.msgCallback && code != JSI_LOG_BUG);
    Jsi_DString pStr;
    Jsi_DSInit(&pStr);
    if (oep) {
        if (oep->id != JSI_OPTION_CUSTOM || !oep->custom)
            pps = Jsi_DSPrintf(&pStr, "for option \"%s\": ", oep->name);
        else {
            Jsi_OptionCustom* cust = Jsi_OptionCustomBuiltin(oep->custom);
            pps = Jsi_DSPrintf(&pStr, "for %s option \"%s\": ", (cust?cust->name:""), oep->name);
        }
    }
    char *cpt;
    if (curFile && interp->logOpts.ftail && (cpt =Jsi_Strrchr(curFile, '/')) && cpt[1])
        curFile = cpt+1;
    if (curFile && curFile[0] && Jsi_Strchr(curFile,'%')==0 && !islog) {
        if (!interp->subOpts.logColNums)
            snprintf(buf, sizeof(buf), "%s:%d: %s: %s%s%s%s",  curFile, line, mt, pps, format, pterm, term);
        else
            snprintf(buf, sizeof(buf), "%s:%d.%d: %s: %s%s%s%s",  curFile, line, lofs, pps, mt,format, pterm, term);
    } else {
        snprintf(buf, sizeof(buf), "%s: %s%s%s%s", mt, pps, format, pterm, term);
    }
    Jsi_DSFree(&pStr);
    buf[sizeof(buf)-1]=0;

    if (logHook)
        (*logHook)(buf, va);
    else if (interp->subOpts.logAllowDups)
        vfprintf(stderr, buf, va);
    else {
        char buf1[JSI_BUFSIZ/2];
        vsnprintf(buf1, sizeof(buf1), buf, va);
        if (!isHelp && !noDups) {
            if (buf1[0] && lastCnt && Jsi_Strcmp(buf1, lastMsg)==0) {
                lastCnt++;
                goto done;
            } else if (lastMsg[0] && lastCnt>1 ) {
                fprintf(stderr, "REPEAT: Last msg repeated %d times...\"\n" ,lastCnt);
            }
            if (buf1[0] == 0 || (buf1[0] == '.' && buf1[1] == 0))
                goto done;
        }
        lastCnt = 1;
        Jsi_Strcpy(lastMsg, buf1);
        if (!islog)
            Jsi_Puts(interp, jsi_Stderr, buf1, -1);
            //fputs(buf1, stderr);
        else {
            Jsi_DString jStr={}, kStr={};
            Jsi_DSPrintf(&kStr, "[%s, \"%s\", \"%s\", %d, %d ]",
                Jsi_JSONQuote(interp, buf1, -1, &jStr), mt, curFile, line, lofs);
            if (Jsi_FunctionInvokeJSON(interp->parent, interp->debugOpts.msgCallback, Jsi_DSValue(&kStr), NULL) != JSI_OK)
                code = 1;
            Jsi_DSFree(&jStr);
            Jsi_DSFree(&kStr);
        }
    }
done:
    va_end(va);
    if (interp->debugOpts.hook) {
        static int inhook = 0;
        if (!inhook) {
            inhook = 1;
            (*interp->debugOpts.hook)(interp, curFile, interp->curIp?interp->curIp->Line:0, interp->level, interp->curFunction, "DEBUG", NULL, emsg);
        }
        inhook = 0;
    }
    if ((code & jsi_fatalexit) && !interp->opts.no_exit)
        jsi_DoExit(interp, 1);
    return (code==JSI_LOG_ERROR?JSI_ERROR:JSI_OK);
bail:
    va_end(va);
    return JSI_OK;
}

const char* Jsi_KeyAdd(Jsi_Interp *interp, const char *str)
{
    Jsi_MapEntry *hPtr;
    bool isNew;
    hPtr = Jsi_MapEntryNew(interp->strKeyTbl, str, &isNew);
    assert(hPtr) ;
    return (const char*)Jsi_MapKeyGet(hPtr, 0);
}

const char* Jsi_KeyLookup(Jsi_Interp *interp, const char *str)
{
    Jsi_MapEntry *hPtr;
    hPtr = Jsi_MapEntryFind(interp->strKeyTbl, str);
    if (!hPtr) {
        return NULL;
    }
    return (const char*)Jsi_MapKeyGet(hPtr, 0);
}


Jsi_Value *Jsi_VarLookup(Jsi_Interp *interp, const char *varname)
{
    Jsi_Value *v;
    v = Jsi_ValueObjLookup(interp, interp->framePtr->incsc, (char*)varname, 0);
    if (!v)
        v = jsi_ScopeChainObjLookupUni(interp->framePtr->ingsc, (char*)varname);
    return v;
}

static char *FindEndB(char *cp) {
    
    if (*cp == '\"'||*cp == '\'') {
        char endc = *cp;
        cp++;
        while (*cp && *cp != endc) {
            if (*cp == '\\' && cp[1]) cp++;
            cp++;
        }
        if (*cp == endc)
            cp++;
        if (*cp != ']')
            return NULL;
        return cp;
    } else
        return Jsi_Strchr(cp, ']');
}


/* Lookup "name" within object "inObj", ie.  "inObj.name"  */
Jsi_Value *Jsi_NameLookup2(Jsi_Interp *interp, const char *name, const char *inObj)
{
    Jsi_Value *v;
    if (!name)
        return NULL;
    if (!inObj)
        return Jsi_VarLookup(interp, name);
    v = Jsi_VarLookup(interp, inObj);
    if (!v)
        return NULL;
    if (Jsi_ValueIsArray(interp, v)) {
        int n;
        if (!isdigit(name[0]))
            return NULL;
        if (Jsi_GetInt(interp, name, &n, 0) != JSI_OK)
            return NULL;
        if (n>=0 && n<(int)v->d.obj->arrCnt)
            return v->d.obj->arr[n];
        return NULL;
    }
    if (v->vt != JSI_VT_OBJECT || (v->d.obj->ot != JSI_OT_OBJECT && v->d.obj->ot != JSI_OT_FUNCTION))
        return NULL;
    return Jsi_ValueObjLookup(interp, v, name, 0);
}

/* Lookup a name, eg.  "a[b].c  a.b.c  a[b][c]  a.b[c]  a["b"].c  a[1].c  */
Jsi_Value *Jsi_NameLookup(Jsi_Interp *interp, const char *name)
{
    uint cnt = 0, len, isq;
    char *nam = (char*)name, *cp, *cp2, *ocp, *kstr;
    //DECL_VALINIT(tv);
    DECL_VALINIT(nv);
    DECL_VALINIT(key);
    Jsi_Value *v = NULL, *nvPtr = &nv;
    Jsi_Value *kPtr = &key; // Note: a string key so no reset needed.
    Jsi_DString dStr = {};
    cp2 = Jsi_Strchr(nam,'[');
    cp = Jsi_Strchr(nam, '.');
    if (cp2 && (cp==0 || cp2<cp))
        cp = cp2;
    if (!cp)
        return Jsi_VarLookup(interp, nam);
    //fprintf(stderr, "NAM: %s\n", nam);
    Jsi_DSSetLength(&dStr, 0);
    Jsi_DSAppendLen(&dStr, nam, cp-nam);
    v = Jsi_VarLookup(interp, Jsi_DSValue(&dStr));
    if (!v)
        goto bail;
    while (v && cnt++ < 1000) {
        ocp = cp;
        nam = cp+1;
        isq = 0;
        if (*cp == '[') {
            cp = FindEndB(cp+1); /* handle [] in strings. */
            if (!cp) goto bail;
            len = cp-nam;
            cp++;
            if (len>=2 && ((nam[0] == '\"' && nam[len-1] == '\"') || (nam[0] == '\'' && nam[len-1] == '\''))) {
                nam += 1;
                len -= 2;
                isq = 1;
            }
        } else if (*cp == '.') {
            cp2 = Jsi_Strchr(nam,'[');
            cp = Jsi_Strchr(nam, '.');
            if (cp2 && (cp==0 || cp2<cp))
                cp = cp2;
            len = (cp ? (uint)(cp-nam) : Jsi_Strlen(nam));
        } else {
            goto bail;
        }
        Jsi_DSSetLength(&dStr, 0);
        Jsi_DSAppendLen(&dStr, nam, len);
        kstr = Jsi_DSValue(&dStr);
        if (*ocp == '[' && isq == 0 && isdigit(kstr[0]) && Jsi_ValueIsArray(interp, v)) {
            int nn;
            if (Jsi_GetInt(interp, kstr, &nn, 0) != JSI_OK)
                goto bail;
            v = Jsi_ValueArrayIndex(interp, v, nn);
            if (!v)
                goto bail;
        } else if (*ocp == '[' && isq == 0) {
            Jsi_Value *kv = Jsi_VarLookup(interp, kstr);
            if (!kv)
                goto bail;
            v = jsi_ValueSubscript(interp, v, kv, &nvPtr);
            goto keyon;
        } else {
            Jsi_ValueMakeStringKey(interp, &kPtr, kstr);
            v = jsi_ValueSubscript(interp, v, kPtr, &nvPtr);
keyon:
            if (!v)
                goto bail;
        }
        if (cp == 0 || *cp == 0) break;
    }
    //Jsi_ValueReset(interp, &ret);
    Jsi_DSFree(&dStr);
    if (v && v == nvPtr) {
        v = Jsi_ValueNew(interp);
        //Jsi_ValueMove(interp, v, &tv);
#ifdef JSI_MEM_DEBUG
        memcpy(v, &nv, sizeof(nv)-sizeof(nv.VD));
        v->VD.label3 = nv.VD.func;
        if (interp->memDebug>1)
            v->VD.label2 = Jsi_KeyAdd(interp, name);
#else
        *v = nv;
#endif
    }
    return v;
bail:
    Jsi_DSFree(&dStr);
    return NULL;
}

Jsi_Value *jsi_GlobalContext(Jsi_Interp *interp)
{
    return interp->csc;
}

typedef struct {
    Jsi_DString *dStr;
    int quote; /* Set to JSI_OUTPUT_JSON, etc*/
    int depth;
} objwalker;

bool Jsi_StrIsAlnum(const char *cp)
{
    if (!cp || !*cp) return 0;
    while (*cp)
        if (isalnum(*cp) || *cp == '_')
            cp++;
        else
            return 0;
    return 1;
}

/* Modify Sql to append bind fields for Object */
Jsi_RC Jsi_SqlObjBinds(Jsi_Interp* interp, Jsi_DString* zStr, const char *varName, bool addTypes, bool addDefaults, bool nullDefaults) {
    Jsi_Value *v, *vnPtr = Jsi_VarLookup(interp, varName);
    if (!vnPtr || !Jsi_ValueIsObjType(interp, vnPtr, JSI_OT_OBJECT))
        return Jsi_LogError("varName must be an Object: %s", varName);
    char *cp, *zSql = Jsi_DSValue(zStr);
    int create = !Jsi_Strncasecmp(zSql,"create",6);
    int insert = !Jsi_Strncasecmp(zSql,"insert",6);
    if (!create && !insert) return JSI_OK;
    const char *cPtr = Jsi_Strstr(zSql, " %s");
    if (!cPtr) cPtr = Jsi_Strstr(zSql, "\t%s");
    if (!cPtr)
        return Jsi_LogError("Object varName must contain a ' %%s': %s", varName);
    Jsi_DString sStr = {}, vStr = {}, jStr = {};
    Jsi_DSAppendLen(&sStr, zSql, cPtr?(cPtr-zSql):-1);
    Jsi_DSAppend(&sStr, " (", NULL);

    Jsi_IterObj *io = Jsi_IterObjNew(interp, NULL);
    Jsi_IterGetKeys(interp, vnPtr, io, 0);
    uint i;
    const char *pre = "", *kstr;
    if (!create)
        Jsi_DSAppend(&vStr, " VALUES(", NULL);
    for (i=0; i<io->count; i++) {
        kstr = io->keys[i];
        const char *qs = "", *qe = "";
        if (!Jsi_StrIsAlnum(kstr) || Jsi_IsReserved(interp, kstr, 1)) {
            qe = qs = "'";
        }
        Jsi_DSAppend(&sStr, pre, qs, kstr, qe, NULL);
        if (create) {
            const char *typ = NULL, *dflt=(nullDefaults?"NULL":NULL);
            if (addTypes && ((v = Jsi_ValueObjLookup(interp, vnPtr, kstr, 1)))) {
                if (Jsi_ValueIsBoolean(interp, v)) {
                    typ = "BOOLEAN";
                    if (!nullDefaults && addDefaults) {
                        bool bv = 0;
                        Jsi_ValueGetBoolean(interp, v, &bv);
                        dflt = (bv?"1":"0");
                    }
                } else if (Jsi_ValueIsNumber(interp, v)) {
                    typ = "NUMERIC";
                    if (!Jsi_Strcmp(kstr,"rowid"))
                        typ = "INTEGER PRIMARY KEY";
                    else if (!nullDefaults && addDefaults) {
                        Jsi_Number nv = 0;
                        Jsi_DSFree(&jStr);
                        Jsi_ValueGetNumber(interp, v, &nv);
                        dflt = Jsi_DSPrintf(&jStr, "%" JSI_NUMGFMT, nv);
                    }
                } else if (Jsi_ValueIsArray(interp, v) || Jsi_ValueIsObjType(interp, v, JSI_OT_OBJECT)) {
                    typ = "CHARJSON";
                    if (!nullDefaults && addDefaults) {
                        Jsi_DSFree(&jStr);
                        Jsi_DSAppend(&jStr, "'", NULL);
                        Jsi_ValueGetDString(interp, v, &jStr, JSI_OUTPUT_JSON|JSI_JSON_STRICT);
                        Jsi_DSAppend(&jStr, "'", NULL);
                        dflt = Jsi_DSValue(&jStr);
                    }
                } else {
                    typ = "TEXT";
                    if (!nullDefaults && addDefaults) {
                        if ((cp=Jsi_ValueString(interp, v, NULL))) {
                            Jsi_DSFree(&jStr);
                            dflt = Jsi_DSAppend(&jStr, "'", cp, "'", NULL);
                        } else
                        dflt = "NULL";
                    }
                }
            }
            if (typ)
                Jsi_DSAppend(&sStr, " ", typ, (dflt?" DEFAULT ":""), dflt, NULL);
        } else {
            Jsi_DSAppend(&vStr, pre, "$", varName, "(", kstr, ")", NULL);
        }
        pre = ",";
    }
    if (!create)
        Jsi_DSAppend(&vStr, ")", NULL);
    Jsi_IterObjFree(io);
    Jsi_DSAppend(&sStr, ") ", Jsi_DSValue(&vStr), cPtr+3, NULL);
    Jsi_DSFree(zStr);
    Jsi_DSAppend(zStr, Jsi_DSValue(&sStr), NULL);
    Jsi_DSFree(&sStr); Jsi_DSFree(&vStr); Jsi_DSFree(&jStr);
    return JSI_OK;
}

char *jsi_TrimStr(char *str) {
    while (isspace(*str)) str++;
    if (!str) return str;
    int len = Jsi_Strlen(str);
    while (--len>=0 && isspace(str[len]))
        str[len] = 0;
    return str;
}

static Jsi_RC jsiValueGetString(Jsi_Interp *interp, Jsi_Value* v, Jsi_DString *dStr, objwalker *owPtr);

static Jsi_RC _object_get_callback(Jsi_Tree *tree, Jsi_TreeEntry *hPtr, void *data)
{
    Jsi_Value *v;
    objwalker *ow = (objwalker *)data;
    Jsi_DString *dStr = ow->dStr;
    int len;
    char *str;
    if ((hPtr->f.bits.dontenum))
        return JSI_OK;
    v =(Jsi_Value*) Jsi_TreeValueGet(hPtr);
    if ((ow->quote&JSI_OUTPUT_JSON) && v && v->vt == JSI_VT_UNDEF)
        return JSI_OK;
    str = (char*)Jsi_TreeKeyGet(hPtr);
    char *cp = Jsi_DSValue(dStr);
    len = Jsi_DSLength(dStr);
    if (len>=2 && (cp[len-2] != '{' || cp[len-1] == '}'))
        Jsi_DSAppend(dStr, ", ", NULL);
    if (((ow->quote&JSI_OUTPUT_JSON) == 0 || (ow->quote&JSI_JSON_STRICT) == 0) && Jsi_StrIsAlnum(str)
        && !Jsi_HashEntryFind(tree->opts.interp->lexkeyTbl, str))
        Jsi_DSAppend(dStr, str, NULL);
    else
        /* JSON/spaces, etc requires quoting the name. */
        Jsi_DSAppend(dStr, "\"", str, "\"", NULL);
    Jsi_DSAppend(dStr, ":", NULL);
    ow->depth++;
    Jsi_RC rc = jsiValueGetString(tree->opts.interp, v, dStr, ow);
    ow->depth--;
    return rc;
}

/* Format value into dStr.  Toplevel caller does init/free. */
static Jsi_RC jsiValueGetString(Jsi_Interp *interp, Jsi_Value* v, Jsi_DString *dStr, objwalker *owPtr)
{
    char buf[JSI_MAX_NUMBER_STRING], *str;
    Jsi_DString eStr;
    Jsi_DSInit(&eStr);
    if (interp->maxDepth>0 && owPtr->depth > interp->maxDepth)
        return Jsi_LogError("recursive ToString");
    int quote = owPtr->quote;
    int isjson = owPtr->quote&JSI_OUTPUT_JSON;
    Jsi_Number num;
    switch(v->vt) {
        case JSI_VT_UNDEF:
            Jsi_DSAppend(dStr, "undefined", NULL);
            return JSI_OK;
        case JSI_VT_NULL:
            Jsi_DSAppend(dStr, "null", NULL);
            return JSI_OK;
        case JSI_VT_VARIABLE:
            Jsi_DSAppend(dStr, "variable", NULL);
            return JSI_OK;
        case JSI_VT_BOOL:
            Jsi_DSAppend(dStr, (v->d.val ? "true":"false"), NULL);
            return JSI_OK;
        case JSI_VT_NUMBER:
            num = v->d.num;
outnum:
            if (isjson && !Jsi_NumberIsNormal(num)) {
                Jsi_DSAppend(dStr, "null", NULL);
            } else if (Jsi_NumberIsInteger(num)) {
                Jsi_NumberItoA10((Jsi_Wide)num, buf, sizeof(buf));
                Jsi_DSAppend(dStr, buf, NULL);
            } else if (Jsi_NumberIsWide(num)) {
                snprintf(buf, sizeof(buf), "%" PRId64, (Jsi_Wide)num);
                Jsi_DSAppend(dStr, buf, NULL);
            } else if (Jsi_NumberIsNormal(num) || Jsi_NumberIsSubnormal(num)) {
                Jsi_NumberDtoA(interp, num, buf, sizeof(buf), 0);
                Jsi_DSAppend(dStr, buf, NULL);
            } else if (Jsi_NumberIsNaN(num)) {
                Jsi_DSAppend(dStr, "NaN", NULL);
            } else {
                int s = Jsi_NumberIsInfinity(num);
                if (s > 0) Jsi_DSAppend(dStr, "+Infinity", NULL);
                else if (s < 0) Jsi_DSAppend(dStr, "-Infinity", NULL);
                else Jsi_LogBug("Ieee function problem: %d", fpclassify(num));
            }
            return JSI_OK;
        case JSI_VT_STRING:
            str = v->d.s.str;
outstr:
            if (!quote) {
                Jsi_DSAppend(dStr, str, NULL);
                return JSI_OK;
            }
            Jsi_DSAppend(dStr,"\"", NULL);
            while (*str) {
                if ((*str == '\'' && (!isjson)) || *str == '\\'|| *str == '\"'|| (*str == '\n'
                    && (!(owPtr->quote&JSI_OUTPUT_NEWLINES)))
                    || *str == '\r' || *str == '\t' || *str == '\f' || *str == '\b'  ) {
                    char pcp[2];
                    *pcp = *str;
                    pcp[1] = 0;
                    Jsi_DSAppendLen(dStr,"\\", 1);
                    switch (*str) {
                        case '\r': *pcp = 'r'; break;
                        case '\n': *pcp = 'n'; break;
                        case '\t': *pcp = 't'; break;
                        case '\f': *pcp = 'f'; break;
                        case '\b': *pcp = 'b'; break;
                    }
                    Jsi_DSAppendLen(dStr,pcp, 1);
                } else if (isprint(*str) || !isjson)
                    Jsi_DSAppendLen(dStr,str, 1);
                else {
                    char ubuf[10];
                    int l = Jsi_UtfEncode(str, ubuf);
                    Jsi_DSAppend(dStr,ubuf, NULL);
                    str += l-1;
                }
                str++;
            }
            Jsi_DSAppend(dStr,"\"", NULL);
            Jsi_DSFree(&eStr);
            return JSI_OK;
        case JSI_VT_OBJECT: {
            Jsi_Obj *o = v->d.obj;
            switch(o->ot) {
                case JSI_OT_BOOL:
                    Jsi_DSAppend(dStr, (o->d.val ? "true":"false"), NULL);
                    return JSI_OK;
                case JSI_OT_NUMBER:
                    num = o->d.num;
                    goto outnum;
                    return JSI_OK;
                case JSI_OT_STRING:
                    str = o->d.s.str;
                    goto outstr;
                case JSI_OT_FUNCTION:
                    Jsi_FuncObjToString(interp, o->d.fobj->func, &eStr, 3 | ((owPtr->depth==0 && owPtr->quote)?8:0));
                    str = Jsi_DSValue(&eStr);
                    goto outstr;
                case JSI_OT_REGEXP:
                    str = o->d.robj->pattern;
                    goto outstr;
                case JSI_OT_USEROBJ:
                    jsi_UserObjToName(interp, o->d.uobj, &eStr);
                    str = Jsi_DSValue(&eStr);
                    goto outstr;
                case JSI_OT_ITER:
                    Jsi_DSAppend(dStr, (isjson?"null":"*ITER*"), NULL);
                    return JSI_OK;
                default:
                    break;
            }
                        
            if (o->isarrlist)
            {
                Jsi_Value *nv;
                int i, len = o->arrCnt;
                
                if (!o->arr)
                    len = Jsi_ValueGetLength(interp, v);
                Jsi_DSAppend(dStr,"[",len?" ":"", NULL);
                for (i = 0; i < len; ++i) {
                    nv = Jsi_ValueArrayIndex(interp, v, i);
                    if (i) Jsi_DSAppend(dStr,", ", NULL);
                    owPtr->depth++;
                    if (nv) {
                        if (jsiValueGetString(interp, nv, dStr, owPtr) != JSI_OK) {
                            owPtr->depth--;
                            return JSI_ERROR;
                        }
                    }
                    else Jsi_DSAppend(dStr, "undefined", NULL);
                    owPtr->depth--;
                }
                Jsi_DSAppend(dStr,len?" ":"","]", NULL);
            } else {
                int len = Jsi_TreeSize(o->tree);
                Jsi_DSAppend(dStr,"{",len?" ":"", NULL);
                owPtr->depth++;
                Jsi_TreeWalk(o->tree, _object_get_callback, owPtr, 0);
                owPtr->depth--;
                Jsi_DSAppend(dStr,len?" ":"","}", NULL);
            }
            return JSI_OK;
        }
#ifndef __cplusplus
        default:
            Jsi_LogBug("Unexpected value type: %d", v->vt);
#endif
    }
    return JSI_OK;
}

/* Format value into dStr.  Toplevel caller does init/free. */
const char* Jsi_ValueGetDString(Jsi_Interp *interp, Jsi_Value* v, Jsi_DString *dStr, int quote)
{
    objwalker ow;
    ow.quote = quote;
    ow.depth = 0;
    ow.dStr = dStr;
    jsiValueGetString(interp, v, dStr, &ow);
    return Jsi_DSValue(dStr);
}

char* jsi_KeyFind(Jsi_Interp *interp, const char *str, int nocreate, int *isKey)
{
    Jsi_MapEntry *hPtr;
    if (isKey) *isKey = 0;
    if (!nocreate) {
        *isKey = 1;
         if (isKey) *isKey = 1;
        return (char*)Jsi_KeyAdd(interp, str);
    }
    hPtr = Jsi_MapEntryFind(interp->strKeyTbl, str);
    if (!hPtr) {
        return Jsi_Strdup(str);;
    }
    if (isKey) *isKey = 1;
    *isKey = 1;
    return (char*)Jsi_MapKeyGet(hPtr, 0);
}

bool jsi_StrIsBalanced(char *str) {
    int cnt = 0, quote = 0;
    char *cp = str;
    while (*cp) {
        switch (*cp) {
        case '\\':
            cp++;
            break;
        case '{': case '(': case '[':
            cnt++;
            break;
        case '\'': case '\"':
            quote++;
            break;
        case '}': case ')': case ']':
            cnt--;
            break;
        }
        if (*cp == 0)
            break;
        cp++;
    }
    return ((quote%2) == 0 && cnt <= 0);
}

static char *get_inputline(Jsi_Interp *interp, int istty, const char *prompt)
{
    char *res;
#ifdef JSI_HAS_READLINE
    if (istty && interp->subOpts.noReadline==0) {
        res = jsi_sh_readline(prompt);
        if (res && *res) jsi_sh_add_history(res);
        return res;
    }
#endif
    int done = 0;
    char bbuf[JSI_BUFSIZ];
    Jsi_DString dStr = {};
    if (istty)
        fputs(prompt, stdout);
    fflush(stdout);
    while (!done) { /* Read a line. */
        bbuf[0] = 0;
        if (fgets(bbuf, sizeof(bbuf), stdin) == NULL)
            return NULL;
        Jsi_DSAppend(&dStr, bbuf, NULL);
        if (Jsi_Strlen(bbuf) < (sizeof(bbuf)-1) || bbuf[sizeof(bbuf)-1] == '\n')
            break;
    }
    res = Jsi_Strdup(Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return res;
}

static Jsi_Interp* jsi_interactiveInterp = NULL;
#ifdef JSI_HAS_READLINE
static Jsi_Value *completeValues = NULL;

#ifdef USE_GNU_READLINE
static int jsiRlStart = 0;

static char *jsiRlCmdMatches(const char *text, int state) {
    static int idx, len;
    const char *name;
    Jsi_Interp* interp = jsi_interactiveInterp;
    if (completeValues == NULL || !Jsi_ValueIsArray(interp, completeValues))
        return NULL;
    Jsi_Value **arr = completeValues->d.obj->arr;
    int aLen = completeValues->d.obj->arrCnt;

    if (!state)
    {
        idx = 0;
        len = Jsi_Strlen(text)-jsiRlStart;
    }
    while (idx<aLen)
    {
        name = Jsi_ValueString(interp, arr[idx], NULL);
        if (!name) name = "";
        idx++;
        if (Jsi_Strncmp(name, text+jsiRlStart, len) == 0)
            return (Jsi_Strdup(name));
    }
    return NULL;
}

static char **jsiRlGetMatches(const char *cstr, int start, int end) {
    char **matches = NULL;
    char *str = rl_line_buffer;
    jsiRlStart = start;
    if (1 || start == 0 || !completeValues) {
        int rc;
        Jsi_Interp* interp = jsi_interactiveInterp;
        if (!completeValues)
            completeValues = Jsi_ValueNew1(interp);
        Jsi_Value *func = interp->onComplete;
        if (func == NULL || !Jsi_ValueIsFunction(interp, func))
            func = Jsi_NameLookup(interp, "Info.completions");
        if (func && Jsi_ValueIsFunction(interp, func)) {
            Jsi_Value *items[3] = {};
            items[0] = Jsi_ValueNewStringDup(interp, str);
            items[1] = Jsi_ValueNewNumber(interp, (Jsi_Number)start);
            items[2] = Jsi_ValueNewNumber(interp, (Jsi_Number)end);;
            Jsi_Value *args = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 3, 0));
            Jsi_IncrRefCount(interp, args);
            rc = Jsi_FunctionInvoke(interp, func, args, &completeValues, interp->csc);
            Jsi_DecrRefCount(interp, args);
            if (rc != JSI_OK)
                fprintf(stderr, "bad completion: %s %d %d\n", str?str:"", start, end);
        }
        matches = rl_completion_matches(str, jsiRlCmdMatches);
    }
    return matches;
}
#else
static const char *jsiFilePreCmds[] = {
    "File.", "source", "load", "new Channel", "new Sqlite", NULL
};

char *jsiLNhints(const char *buf, int *color, int *bold) {
    int i, len = Jsi_Strlen(buf);
    for (i=0; jsiFilePreCmds[i]; i++)
        if (!Jsi_Strncmp(buf, jsiFilePreCmds[i], Jsi_Strlen(jsiFilePreCmds[i]))) break;
    if (jsiFilePreCmds[i]) {
        const char *ce = buf+len-1, *cp = "('<file>";
        if ((*ce =='\'' || *ce =='\"') && buf[len-2]=='(') cp+=2;
        else if (*ce=='(') cp++;
        else return NULL;
        
        *color = 35;
        *bold = 0;
        return (char*)cp;
    }
    return NULL;
}

static void jsiLNGetMatches(const char *str, linenoiseCompletions *lc) {
    char buf[JSI_BUFSIZ], pre[JSI_BUFSIZ], hpre[6] = {};
    const char *cp, *fnam = "Info.completions";
    int i = 0, len;
    int rc, isfile = 0, start = 0, end = Jsi_Strlen(str);
    Jsi_Interp* interp = jsi_interactiveInterp;
    if (!Jsi_Strncmp(str, "help ", 5)) {
        Jsi_Strcpy(hpre, "help ");
        str += 5;
        end -= 5;
    }
    if (end<=0) return;
    Jsi_Strncpy(buf, str, sizeof(buf)-1);
    buf[sizeof(buf)-1] = 0;
    pre[0] = 0;
    if (end<=3 && !Jsi_Strncmp(str, "help", end)) {
        linenoiseAddCompletion(lc, "help");
        return;
    }
    if (!completeValues)
        completeValues = Jsi_ValueNew1(interp);
    Jsi_Value *func = interp->onComplete;
    if (func == NULL || !Jsi_ValueIsFunction(interp, func)) {
        for (i=0; jsiFilePreCmds[i]; i++)
            if (!Jsi_Strncmp(buf, jsiFilePreCmds[i], Jsi_Strlen(jsiFilePreCmds[i]))) break;
        if (jsiFilePreCmds[i] && ((cp=Jsi_Strrchr(buf, '(')) && (cp[1]=='\"' || cp[1]=='\''))) {
            Jsi_Strcpy(pre, buf);
            pre[cp-buf+2] = 0;
            snprintf(buf, sizeof(buf), "%s*%s", cp+2, (buf[0]=='s'?".js*":""));
            isfile = 1;
            fnam = "File.glob";
        }
    }
    func = Jsi_NameLookup(interp, fnam);
    if (func && Jsi_ValueIsFunction(interp, func)) {
        //printf("PATTERN: %s\n", str);
        Jsi_Value *items[3] = {};;
        i = 0;
        items[i++] = Jsi_ValueNewStringDup(interp, buf);
        if (!isfile) {
            items[i++] = Jsi_ValueNewNumber(interp, (Jsi_Number)start);
            items[i++] = Jsi_ValueNewNumber(interp, (Jsi_Number)end);
        }
        Jsi_Value *args = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, i, 0));
        Jsi_IncrRefCount(interp, args);
        rc = Jsi_FunctionInvoke(interp, func, args, &completeValues, interp->csc);
        Jsi_DecrRefCount(interp, args);
        if (rc != JSI_OK) {
            fprintf(stderr, "bad completion: %s %d %d\n", str?str:"", start, end);
            return;
        }
        const char *name;
        Jsi_Interp* interp = jsi_interactiveInterp;
        if (completeValues == NULL || !Jsi_ValueIsArray(interp, completeValues))
            return;
        Jsi_Value **arr = completeValues->d.obj->arr;
        int aLen = completeValues->d.obj->arrCnt;
        i = 0;
        while (i<aLen)
        {
            name = Jsi_ValueString(interp, arr[i], &len);
            if (name) {
                if (!pre[0] && !hpre[0])
                    linenoiseAddCompletion(lc, name);
                else {
                    snprintf(buf, sizeof(buf), "%s%s%s", hpre, pre, name);
                    linenoiseAddCompletion(lc, buf);
                }
            }
            i++;
        }
    }
}
#endif
#endif
#if JSI__SIGNAL
#include <signal.h> //  our new library 
#endif

#if JSI__SIGNAL
static void jsi_InteractiveSignal(int sig){
    if (jsi_interactiveInterp)
        jsi_interactiveInterp->interrupted = 1;
}
#endif
 
/* Collect and execute code from stdin.  The first byte of flags are passed to Jsi_ValueGetDString(). */
Jsi_RC Jsi_Interactive(Jsi_Interp* interp, int flags) 
{
    Jsi_RC rc = JSI_OK;
    int done = 0, len, quote = (flags & 0xff), istty = 1, chkHelp=0, hasHelp=0;
    const char *prompt = interp->subOpts.prompt;
    char *buf;
    if (jsi_interactiveInterp) 
        return Jsi_LogError("multiple interactive not supported");
#if JSI__SIGNAL
  signal(SIGINT, jsi_InteractiveSignal); 
#endif
    interp->typeCheck.parse = interp->typeCheck.run = interp->typeCheck.all = 1;
    interp->strict = 1;
    interp->isInteractive = 1;
    jsi_interactiveInterp = interp;
    interp->subOpts.istty = 1;
    interp->subOpts.logAllowDups = 1;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
#ifndef __WIN32
    istty = isatty(fileno(stdin));
#else
    istty = _isatty(_fileno(stdin));
#endif
#ifdef JSI_HAS_READLINE
    Jsi_DString dHist = {}, sHist = {};
    char *hist = NULL;
#ifdef USE_GNU_READLINE
    rl_attempted_completion_function = jsiRlGetMatches;
#else
    linenoiseSetCompletionCallback(jsiLNGetMatches);
    linenoiseSetHintsCallback(jsiLNhints);
#endif
    if(interp->subOpts.noReadline == 0 && !interp->parent && !(interp->isSafe && interp->safeMode==jsi_safe_Lockdown))
    {
        const char *hfile = (interp->historyFile ? interp->historyFile : "~/.jsish_history");
        hist = Jsi_NormalPath(interp, hfile, &dHist);
        if (hist)
            jsi_sh_read_history(hist);
    }
#endif
    interp->level++;
    if (!interp->iskips)
        puts("Jsish interactive: see 'help [cmd]' or 'history'.  \\ cancels > input."
#if JSI__SIGNAL
        "  ctrl-c aborts running script."
#endif
        );
    while (done==0 && interp->exited==0) {
        buf = get_inputline(interp, istty, (prompt?prompt:"$ "));
        if (buf) {
            if (buf[0] == '\\' && !buf[1]) {
                 Jsi_DSSetLength(&dStr, 0);
                 prompt = interp->subOpts.prompt;
                 fprintf(stderr, "abandoned input");
            } else
                Jsi_DSAppend(&dStr, buf, NULL);
            free(buf);
        } else {
            done = 1;
        }
        len = Jsi_DSLength(&dStr);
        if (done && len == 0)
            break;
        if (!len) continue;
        Jsi_DSAppendLen(&dStr, " ", 1); // Allow for added space.
        buf = Jsi_DSValue(&dStr);
        if (done == 0 && (!jsi_StrIsBalanced(buf))) {
            prompt = interp->subOpts.prompt2;
            continue;
        }
        prompt = interp->subOpts.prompt;
        while ((len = Jsi_Strlen(buf))>0 && (isspace(buf[len-1])))
            buf[len-1] = 0;
        if (buf[0] == 0) {
            Jsi_DSSetLength(&dStr, 0);
            continue;
        }
        bool wantHelp = 0;
        if (interp->onEval == NULL) {
            /* Convenience: add semicolon to "var" statements (required by parser). */
#ifdef JSI_HAS_READLINE
            if (!Jsi_Strncmp(buf, "history", 7) && buf[7] == 0) {
                fputs(Jsi_DSValue(&sHist), stdout);
                Jsi_DSSetLength(&dStr, 0);
                continue;
            }
#endif
            if (!Jsi_Strncmp(buf, "help", 4) && (buf[4] == 0 || isspace(buf[4]))) {
                if (!chkHelp++)
                    hasHelp = (Jsi_PkgRequire(interp, "Help", 0)>=0);
                if (hasHelp) {
                    wantHelp = 1;
                    char tbuf[BUFSIZ];
                    snprintf(tbuf, sizeof(tbuf), "return runModule('Help', '%s'.trim().split(null));", buf+4);
                    rc = Jsi_EvalString(interp, tbuf, JSI_RETURN);
                }
            }
            if (!wantHelp) {
                if (!Jsi_Strncmp(buf,"var ", 4) && Jsi_Strchr(buf, '\n')==NULL && Jsi_Strchr(buf, ';')==NULL)
                    Jsi_Strcpy(buf+Jsi_Strlen(buf), ";"); // Added space above so strcat ok.
                rc = Jsi_EvalString(interp, buf, JSI_EVAL_RETURN);
                prompt = interp->subOpts.prompt;
#ifdef JSI_HAS_READLINE
                if (rc == JSI_OK)
                    Jsi_DSAppend(&sHist, buf, "\n", NULL);
#endif
            }
        }
        else
        {
            Jsi_Value *func = interp->onEval;
            if (func && Jsi_ValueIsFunction(interp, func)) {
                Jsi_Value *items[1] = {};
                items[0] = Jsi_ValueNewStringDup(interp, buf);
                Jsi_Value *args = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 1, 0));
                Jsi_IncrRefCount(interp, args);
                rc = Jsi_FunctionInvoke(interp, func, args, &interp->retValue, interp->csc);
                Jsi_DecrRefCount(interp, args);
                if (rc != JSI_OK)
                    fprintf(stderr, "bad eval");
            }
        }
        if (interp->exited)
            break;
        jsi_interactiveInterp->interrupted = 0;
        if (rc == JSI_OK) {
             if (interp->retValue->vt != JSI_VT_UNDEF || interp->subOpts.outUndef) {
                Jsi_DString eStr = {};
                fputs(Jsi_ValueGetDString(interp, interp->retValue, &eStr, hasHelp?0:quote), stdout);
                Jsi_DSFree(&eStr);
                fputs("\n", stdout);
             }
        } else if (!interp->exited && !wantHelp) {
            fputs("ERROR\n", stderr);
        }
        Jsi_DSSetLength(&dStr, 0);
        len = 0;
    }
    interp->level--;
#ifdef JSI_HAS_READLINE
    if (hist && !interp->isSafe) {
        jsi_sh_stifle_history(100);
        jsi_sh_write_history(hist);
    }
    Jsi_DSFree(&dHist);
    Jsi_DSFree(&sHist);
#endif
    Jsi_DSFree(&dStr);
    if (interp->retValue) {
        Jsi_DecrRefCount(interp, interp->retValue);
        interp->retValue = NULL;
    }
    if (interp->exited && interp->level <= 0)
    {
        rc = JSI_EXIT;
        Jsi_InterpDelete(interp);
    }
    jsi_interactiveInterp = NULL;
    return rc;
}

Jsi_RC Jsi_ThisDataSet(Jsi_Interp *interp, Jsi_Value *_this, void *value)
{
    bool isNew;
    Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->thisTbl, _this, &isNew);
    if (!hPtr)
        return JSI_ERROR;
    Jsi_HashValueSet(hPtr, value);
    return JSI_OK;
}

void *Jsi_ThisDataGet(Jsi_Interp *interp, Jsi_Value *_this)
{
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryFind(interp->thisTbl, _this);
    if (!hPtr)
        return NULL;
    return Jsi_HashValueGet(hPtr);
}

Jsi_RC Jsi_PrototypeDefine(Jsi_Interp *interp, const char *key, Jsi_Value *value)
{
    bool isNew;
    Jsi_HashEntry *hPtr = Jsi_HashEntryNew(interp->protoTbl, key, &isNew);
    if (!hPtr)
        return JSI_ERROR;
    Jsi_HashValueSet(hPtr, value);
    return JSI_OK;
}

void *Jsi_PrototypeGet(Jsi_Interp *interp, const char *key)
{
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->protoTbl, key);
    if (!hPtr)
        return NULL;
    return Jsi_HashValueGet(hPtr);
}

Jsi_RC Jsi_PrototypeObjSet(Jsi_Interp *interp, const char *key, Jsi_Obj *obj)
{
    Jsi_Value *val;
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->protoTbl, key);
    if (!hPtr)
        return JSI_ERROR;
    val = (Jsi_Value *)Jsi_HashValueGet(hPtr);
    obj->__proto__ = val;
    return JSI_OK;
}

const char *Jsi_ObjTypeStr(Jsi_Interp *interp, Jsi_Obj *o)
{
     switch (o->ot) {
        case JSI_OT_BOOL: return "boolean"; break;
        case JSI_OT_FUNCTION: return "function"; break;
        case JSI_OT_NUMBER: return "number"; break;
        case JSI_OT_STRING: return "string"; break;  
        case JSI_OT_REGEXP: return "regexp"; break;  
        case JSI_OT_ITER: return "iter"; break;  
        case JSI_OT_OBJECT: if (!o->isarrlist) return "object";
        case JSI_OT_ARRAY: return "array"; break;  
        case JSI_OT_USEROBJ:
            if (o->__proto__) {
                Jsi_HashEntry *hPtr;
                Jsi_HashSearch search;
                            
                for (hPtr = Jsi_HashSearchFirst(interp->thisTbl,&search); hPtr != NULL;
                    hPtr = Jsi_HashSearchNext(&search))
                    if (Jsi_HashValueGet(hPtr) == o->__proto__)
                        return (char*)Jsi_HashKeyGet(hPtr);
            }
            
            return "userobj";
            break;
            //return Jsi_ObjGetType(interp, v->d.obj);
        default:
            break;
     }
     return "";
}

extern Jsi_otype Jsi_ObjTypeGet(Jsi_Obj *obj)
{
    return obj->ot;
}

const char *Jsi_ValueTypeStr(Jsi_Interp *interp, Jsi_Value *v)
{
    switch (v->vt) {
        case JSI_VT_BOOL: return "boolean"; break;
        case JSI_VT_UNDEF: return "undefined"; break;
        case JSI_VT_NULL: return "null"; break;
        case JSI_VT_NUMBER: return "number"; break;
        case JSI_VT_STRING: return "string"; break;  
        case JSI_VT_VARIABLE: return "variable"; break;  
        case JSI_VT_OBJECT: return Jsi_ObjTypeStr(interp, v->d.obj);
    }
    return "";
}

/* For user defined object  "name", invokes "new"  with "arg" + "opts".  Returns userobj data pointer for C use. */
void *Jsi_CommandNewObj(Jsi_Interp *interp, const char *name, const char *arg1, const char *opts, const char *var) {
    char buf[JSI_BUFSIZ];
    if (arg1)
        snprintf(buf, sizeof(buf), "%s%snew %s('%s', %s);", var?var:"", var?"=":"return ", name, arg1, opts?opts:"null");
    else
        snprintf(buf, sizeof(buf), "%s%snew %s(%s);", var?var:"", var?"=":"return ", name, opts?opts:"null");
    int rc = Jsi_EvalString(interp, buf, 0);
    if (rc != JSI_OK)
        return NULL;
    Jsi_Value *vObj = interp->retValue;
    if (var)
        vObj = Jsi_NameLookup(interp, var);
    if (!vObj)
        return NULL;
    return Jsi_UserObjGetData(interp, vObj, NULL);
}

#endif

// List

Jsi_List *Jsi_ListNew(Jsi_Interp *interp, Jsi_Wide flags, Jsi_HashDeleteProc *freeProc)
{
    Jsi_List *list = (Jsi_List *)Jsi_Calloc(1, sizeof(Jsi_List));
    list->sig = JSI_SIG_LIST;
    list->opts.flags = flags;
    list->opts.freeHashProc = freeProc;
    list->opts.interp = interp;
    list->opts.mapType = JSI_MAP_LIST;
    list->opts.keyType = (Jsi_Key_Type)-1;
    return list;
}

Jsi_RC Jsi_ListConf(Jsi_List *listPtr, Jsi_MapOpts *opts, bool set)
{
    if (set) {
        listPtr->opts = *opts;
    } else {
        *opts = listPtr->opts;
    }
    return JSI_OK;
}

void Jsi_ListDelete(Jsi_List *list) {
    Jsi_ListClear(list);
    free(list);
}

void Jsi_ListClear(Jsi_List *list) {
    Jsi_ListEntry *l;
    while (list->head) {
        l = list->head;
        list->head = list->head->next;
        l->next = l->prev = NULL;
        if (list->opts.freeListProc && l->value)
            (list->opts.freeListProc)(list->opts.interp, l, l->value);
        Jsi_ListEntryDelete(l);
    }
    list->numEntries = 0;
}
 
Jsi_ListEntry* Jsi_ListPush(Jsi_List *list, Jsi_ListEntry *item, Jsi_ListEntry *before)
{
    Assert(item && list);
    if (item->list && (item->list->head == item || item->prev || item->next)) {
        Assert(list->opts.freeListProc == item->list->opts.freeListProc);
        Jsi_ListPop(item->list, item);
    }
        
    if (!item->list)
         item->list = list;
    else if (list != item->list) {
        Assert(list->opts.freeListProc == item->list->opts.freeListProc);
        item->list = list;
    }
    if (!list->head) {
        list->head = list->tail = item;
    } else if (item == list->head) {
        assert(0);
    } else if (before == NULL) {
        item->prev = list->tail;
        list->tail->next = item;
        list->tail = item;
    } else if (before == list->head) {
        item->next = list->head;
        list->head->prev = item;
        list->head = item;
    } else {
        item->next = before;
        item->prev = before->prev;
        before->prev->next = item;
        before->prev = item;
    }
    list->numEntries++;
    item->list = list;
    return item;
}
 
Jsi_ListEntry* Jsi_ListPop(Jsi_List *list, Jsi_ListEntry *item)
{
    Assert(item && list->head && list->tail && item->list);
    SIGASSERT(list, LIST);
    SIGASSERT(item, LISTENTRY);
    if (item == list->head) {
        if (list->head == list->tail)
            list->head = list->tail = NULL;
        else
            list->head = list->head->next;
    }
    else if (item == list->tail) {
        list->tail = list->tail->prev;
        list->tail->next = NULL;
    } else {
        item->prev->next = item->next;
        if (item->next)
            item->next->prev = item->prev;
    }
    list->numEntries--;
    item->next = item->prev = NULL;
    return item;
}

Jsi_ListEntry *Jsi_ListEntryNew(Jsi_List* list, const void *value, Jsi_ListEntry *before) {
    SIGASSERT(list, LIST);
    Jsi_ListEntry *l = (Jsi_ListEntry*)Jsi_Calloc(1, sizeof(Jsi_ListEntry));
    l->sig = JSI_SIG_LISTENTRY;
    l->typ = JSI_MAP_LIST;
    l->list = list;
    l->value = (void*)value;
    Jsi_ListPush(list, l, before);
    return l;
}

int Jsi_ListEntryDelete(Jsi_ListEntry *l) {
    SIGASSERT(l, LISTENTRY);
    if (l->next || l->prev)
        Jsi_ListPop(l->list, l);
    Jsi_Free(l);
    return 1;
}

Jsi_ListEntry* Jsi_ListSearchFirst (Jsi_List *list, Jsi_ListSearch *searchPtr, int flags)
{
    SIGASSERT(list, LIST);
    searchPtr->flags = flags;
    Jsi_ListEntry *lptr;
    if (flags & JSI_LIST_REVERSE) {
        lptr = Jsi_ListGetBack(list);
        searchPtr->nextEntryPtr = (lptr?Jsi_ListEntryPrev(lptr):NULL);
    } else {
        lptr = Jsi_ListGetFront(list);
        searchPtr->nextEntryPtr = (lptr?Jsi_ListEntryNext(lptr):NULL);
    }
    return lptr;
}

Jsi_ListEntry* Jsi_ListSearchNext (Jsi_ListSearch *searchPtr)
{
    Jsi_ListEntry *lptr = searchPtr->nextEntryPtr;
    searchPtr->nextEntryPtr = (lptr?(searchPtr->flags & JSI_LIST_REVERSE ? Jsi_ListEntryPrev(lptr): Jsi_ListEntryNext(lptr)):NULL);
    return lptr;
}


uint Jsi_ListSize(Jsi_List *list) {
    SIGASSERT(list, LIST);
    return list->numEntries;
}

void* Jsi_ListValueGet(Jsi_ListEntry *l) {
    SIGASSERT(l, LISTENTRY);
    return l?l->value:NULL;
}
void Jsi_ListValueSet(Jsi_ListEntry *l, const void *value) {
    SIGASSERTV(l, LISTENTRY);
    l->value = (void*)value;
}


// Map

Jsi_Map* Jsi_MapNew (Jsi_Interp *interp, Jsi_Map_Type listType, Jsi_Key_Type keyType, Jsi_MapDeleteProc *freeProc)
{
    Jsi_Map *lPtr, lval = {.sig=JSI_SIG_MAP};
    lval.typ = listType;
    switch (listType) {
        case JSI_MAP_HASH: lval.v.hash = Jsi_HashNew(interp, keyType, (Jsi_HashDeleteProc*)freeProc); break;
        case JSI_MAP_TREE: lval.v.tree = Jsi_TreeNew(interp, keyType, (Jsi_TreeDeleteProc*)freeProc); break;
        case JSI_MAP_LIST: lval.v.list = Jsi_ListNew(interp, keyType, (Jsi_HashDeleteProc*)freeProc); break;
        default: return NULL;
    }
    if (!lval.v.hash) return NULL;
    lPtr = (Jsi_Map*)Jsi_Malloc(sizeof(*lPtr));
    *lPtr = lval;
    return lPtr;
}

Jsi_RC Jsi_MapConf(Jsi_Map *mapPtr, Jsi_MapOpts *opts, bool set)
{
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: return Jsi_HashConf(mapPtr->v.hash, opts, set);
        case JSI_MAP_TREE: return Jsi_TreeConf(mapPtr->v.tree, opts, set);
        case JSI_MAP_LIST: return Jsi_ListConf(mapPtr->v.list, opts, set);
        case JSI_MAP_NONE: break;
    }
    return JSI_ERROR;
}

void Jsi_MapClear (Jsi_Map *mapPtr) {
    SIGASSERTV(mapPtr, MAP);
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: Jsi_HashClear(mapPtr->v.hash); break;
        case JSI_MAP_TREE: Jsi_TreeClear(mapPtr->v.tree); break;
        case JSI_MAP_LIST: Jsi_ListClear(mapPtr->v.list); break;
        default: return;
    }
}

void Jsi_MapDelete (Jsi_Map *mapPtr) {
    SIGASSERTV(mapPtr, MAP);
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: Jsi_HashDelete(mapPtr->v.hash); break;
        case JSI_MAP_TREE: Jsi_TreeDelete(mapPtr->v.tree); break;
        case JSI_MAP_LIST: Jsi_ListDelete(mapPtr->v.list); break;
        default: return;
    }
    Jsi_Free(mapPtr);
}
Jsi_MapEntry* Jsi_MapSet(Jsi_Map *mapPtr, const void *key, const void *value){
    SIGASSERT(mapPtr, MAP);
    Jsi_MapEntry* mptr = NULL;
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: mptr = (Jsi_MapEntry*)Jsi_HashSet(mapPtr->v.hash, (void*)key, (void*)value); break;
        case JSI_MAP_TREE: mptr = (Jsi_MapEntry*)Jsi_TreeSet(mapPtr->v.tree, (void*)key, (void*)value); break;
        case JSI_MAP_LIST: {
            mptr = Jsi_MapEntryNew(mapPtr, key, NULL);
            Jsi_MapValueSet(mptr, (void*)value);
            break;
        }
        case JSI_MAP_NONE: break;
    }
    return mptr;
}
void* Jsi_MapGet(Jsi_Map *mapPtr, const void *key, int flags){
    SIGASSERT(mapPtr, MAP);
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: return Jsi_HashGet(mapPtr->v.hash, (void*)key, flags);
        case JSI_MAP_TREE: return Jsi_TreeGet(mapPtr->v.tree, (void*)key, flags);
        case JSI_MAP_LIST: {
            Jsi_ListEntry* lptr = (key == NULL? Jsi_ListGetFront(mapPtr->v.list) : Jsi_ListGetBack(mapPtr->v.list));
            if (lptr)
                return Jsi_ListValueGet(lptr);
            break;
        }
        case JSI_MAP_NONE: break;
    }
    return NULL;
}
bool Jsi_MapUnset(Jsi_Map *mapPtr, const void *key){
    SIGASSERT(mapPtr, MAP);
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: return Jsi_HashUnset(mapPtr->v.hash, (void*)key);
        case JSI_MAP_TREE: return Jsi_TreeUnset(mapPtr->v.tree, (void*)key);
        case JSI_MAP_LIST: {
            /*Jsi_ListEntry* lptr = (key == NULL? Jsi_ListGetFront(mapPtr->v.list) : Jsi_ListGetBack(mapPtr->v.list));
            if (lptr)
                return Jsi_ListUnset(lptr);*/
            break;
        }
        case JSI_MAP_NONE: break;
    }
    return false;
}
static int jsi_GetListType(Jsi_MapEntry *h) {
    Jsi_HashEntry *hPtr =(Jsi_HashEntry *)h;
    return hPtr->typ;
}
void* Jsi_MapKeyGet(Jsi_MapEntry *h, int flags){
    switch (jsi_GetListType(h)) {
        case JSI_MAP_HASH: return Jsi_HashKeyGet((Jsi_HashEntry*)h);
        case JSI_MAP_TREE: return Jsi_TreeKeyGet((Jsi_TreeEntry*)h);
        case JSI_MAP_LIST: break;
        case JSI_MAP_NONE: break;
    }
    return NULL;
}
#ifndef JSI_LITE_ONLY
Jsi_RC Jsi_MapKeysDump(Jsi_Interp *interp, Jsi_Map *mapPtr, Jsi_Value **ret, int flags){
    SIGASSERT(mapPtr, MAP);
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: return Jsi_HashKeysDump(interp, mapPtr->v.hash, ret, flags);
        case JSI_MAP_TREE: return Jsi_TreeKeysDump(interp, mapPtr->v.tree, ret, flags);
        case JSI_MAP_LIST: break; // TODO: dump numbers?
        case JSI_MAP_NONE: break;
    }
    return JSI_ERROR;
}
#endif
void* Jsi_MapValueGet(Jsi_MapEntry *h){
    switch (jsi_GetListType(h)) {
        case JSI_MAP_HASH: return Jsi_HashValueGet((Jsi_HashEntry*)h);
        case JSI_MAP_TREE: return Jsi_TreeValueGet((Jsi_TreeEntry*)h);
        case JSI_MAP_LIST: return Jsi_ListValueGet((Jsi_ListEntry*)h);
        case JSI_MAP_NONE: break;
    }
    return NULL;
}
void Jsi_MapValueSet(Jsi_MapEntry *h, const void *value){
    switch (jsi_GetListType(h)) {
        case JSI_MAP_HASH: return Jsi_HashValueSet((Jsi_HashEntry*)h, (void*)value);
        case JSI_MAP_TREE: return Jsi_TreeValueSet((Jsi_TreeEntry*)h, (void*)value);
        case JSI_MAP_LIST: return Jsi_ListValueSet((Jsi_ListEntry*)h, (void*)value);
        case JSI_MAP_NONE: break;
    }
}
Jsi_MapEntry* Jsi_MapEntryFind (Jsi_Map *mapPtr, const void *key){
    SIGASSERT(mapPtr, MAP);
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: return (Jsi_MapEntry*)Jsi_HashEntryFind(mapPtr->v.hash, key);
        case JSI_MAP_TREE: return (Jsi_MapEntry*)Jsi_TreeEntryFind(mapPtr->v.tree, key);
        case JSI_MAP_LIST:
            return (Jsi_MapEntry*) (key == NULL? Jsi_ListGetFront(mapPtr->v.list) : Jsi_ListGetBack(mapPtr->v.list));
        case JSI_MAP_NONE: break;
    }
    return NULL;
}
Jsi_MapEntry* Jsi_MapEntryNew (Jsi_Map *mapPtr, const void *key, bool *isNew){
    SIGASSERT(mapPtr, MAP);
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: return (Jsi_MapEntry*)Jsi_HashEntryNew(mapPtr->v.hash, key, isNew);
        case JSI_MAP_TREE: return (Jsi_MapEntry*)Jsi_TreeEntryNew(mapPtr->v.tree, key, isNew);
        case JSI_MAP_LIST: {
            Jsi_ListEntry *lptr = Jsi_ListEntryNew(mapPtr->v.list, NULL, (key?mapPtr->v.list->head:NULL));
            if (isNew) *isNew = 1;
            return (Jsi_MapEntry*)lptr;
        }
        break;
        case JSI_MAP_NONE: break;
    }
    return NULL;
}
int Jsi_MapEntryDelete (Jsi_MapEntry *entryPtr){
    switch (jsi_GetListType(entryPtr)) {
        case JSI_MAP_HASH: return Jsi_HashEntryDelete((Jsi_HashEntry*)entryPtr);
        case JSI_MAP_TREE: return Jsi_TreeEntryDelete((Jsi_TreeEntry*)entryPtr);
        case JSI_MAP_LIST: {
            Jsi_ListEntry *lptr = (Jsi_ListEntry*)entryPtr;
            Jsi_ListPop(lptr->list, lptr);
            Jsi_ListEntryDelete(lptr);
            return 1;
        }
    }
    return JSI_OK;
}
Jsi_MapEntry* Jsi_MapSearchFirst (Jsi_Map *mapPtr, Jsi_MapSearch *searchPtr, int flags){
    SIGASSERT(mapPtr, MAP);
    searchPtr->typ = mapPtr->typ;
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: return (Jsi_MapEntry*)Jsi_HashSearchFirst(mapPtr->v.hash, &searchPtr->v.hash);
        case JSI_MAP_TREE: return (Jsi_MapEntry*)Jsi_TreeSearchFirst(mapPtr->v.tree, &searchPtr->v.tree, flags, NULL);
        case JSI_MAP_LIST: return (Jsi_MapEntry*)Jsi_ListSearchFirst(mapPtr->v.list, &searchPtr->v.list, flags);
        case JSI_MAP_NONE: break;
    }
    return NULL;
}
Jsi_MapEntry* Jsi_MapSearchNext (Jsi_MapSearch *searchPtr){
    switch (searchPtr->typ) {
        case JSI_MAP_HASH: return (Jsi_MapEntry*)Jsi_HashSearchNext(&searchPtr->v.hash);
        case JSI_MAP_TREE: return (Jsi_MapEntry*)Jsi_TreeSearchNext(&searchPtr->v.tree);
        case JSI_MAP_LIST: return (Jsi_MapEntry*)Jsi_ListSearchNext(&searchPtr->v.list);
        case JSI_MAP_NONE: break;
    }
    return NULL;
}
void Jsi_MapSearchDone (Jsi_MapSearch *searchPtr){
    switch (searchPtr->typ) {
        case JSI_MAP_HASH: break;
        case JSI_MAP_TREE: Jsi_TreeSearchDone(&searchPtr->v.tree); break;
        case JSI_MAP_LIST: break;
        case JSI_MAP_NONE: break;
    }
}
uint Jsi_MapSize(Jsi_Map *mapPtr) {
    SIGASSERT(mapPtr, MAP);
    switch (mapPtr->typ) {
        case JSI_MAP_HASH: return Jsi_HashSize(mapPtr->v.hash);
        case JSI_MAP_TREE: return Jsi_TreeSize(mapPtr->v.tree);
        case JSI_MAP_LIST: return Jsi_ListSize(mapPtr->v.list);
        case JSI_MAP_NONE: break;
    }
    return -1;
}


#ifndef JSI_OMIT_THREADS

#ifdef __WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct Jsi_Mutex {
    int flags;
    int lockTimeout;
    int threadErrCnt;
#ifdef __WIN32
    CRITICAL_SECTION mtx;
#else
    pthread_mutex_t mtx;
#endif
} Jsi_Mutex;

#ifdef __WIN32
#include <windows.h>

static Jsi_RC MutexLock(Jsi_Interp *interp, Jsi_Mutex* mtx) {
    int timeout = mtx->lockTimeout;
    if (interp && timeout<0)
        timeout = interp->lockTimeout;
    if (timeout<=0)
        EnterCriticalSection(&mtx->mtx);
    else {
        int cnt = timeout;
        while (cnt-- >= 0) {
            if (TryEnterCriticalSection(&mtx->mtx))
                return JSI_OK;
            usleep(1000);
        }
        Jsi_LogError("lock timed out");
        if (interp)
            interp->threadErrCnt++;
        mtx->threadErrCnt++;
        return JSI_ERROR;
    }
    return JSI_OK;
}
static void MutexUnlock(Jsi_Mutex* mtx) { LeaveCriticalSection(&mtx->mtx); }
static void MutexInit(Jsi_Mutex *mtx) {  InitializeCriticalSection(&mtx->mtx); }

static void MutexDone(Jsi_Mutex *mtx) { DeleteCriticalSection(&mtx->mtx); }
#else /* ! __WIN32 */

#include <pthread.h>
static Jsi_RC MutexLock(Jsi_Interp *interp, Jsi_Mutex *mtx) {
    int timeout = mtx->lockTimeout;
    if (interp && timeout<0)
        timeout = interp->lockTimeout;
    if (timeout<=0)
        pthread_mutex_lock(&mtx->mtx);
    else {
        struct timespec ts;
        ts.tv_sec = timeout/1000;
        ts.tv_nsec = 1000 * (timeout%1000);
        int rc = pthread_mutex_timedlock(&mtx->mtx, &ts);
        if (rc != 0) {
            Jsi_LogError("lock timed out");
            if (interp)
                interp->threadErrCnt++;
            mtx->threadErrCnt++;
            return JSI_ERROR;
        }
    }
    return JSI_OK;
}
static void MutexUnlock(Jsi_Mutex *mtx) { pthread_mutex_unlock(&mtx->mtx); }
static void MutexInit(Jsi_Mutex *mtx) {
    pthread_mutexattr_t Attr;
    pthread_mutexattr_init(&Attr);
    if (mtx->flags & JSI_MUTEX_RECURSIVE)
        pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mtx->mtx, &Attr);
}

static void MutexDone(Jsi_Mutex *mtx) { pthread_mutex_destroy(&mtx->mtx); }
#endif /* ! __WIN32 */

Jsi_RC Jsi_MutexLock(Jsi_Interp *interp, Jsi_Mutex *mtx) { if (interp) interp->lockRefCnt++; return MutexLock(interp, mtx);}
void Jsi_MutexUnlock(Jsi_Interp *interp, Jsi_Mutex *mtx) { MutexUnlock(mtx); if (interp) interp->lockRefCnt--; }
Jsi_Mutex* Jsi_MutexNew(Jsi_Interp *interp, int timeout, int flags) {
     Jsi_Mutex *mtx = (Jsi_Mutex *)Jsi_Calloc(1,sizeof(Jsi_Mutex));
     mtx->lockTimeout = timeout;
     mtx->flags = flags;
     MutexInit(mtx);
     return mtx;
}
void Jsi_MutexDelete(Jsi_Interp *interp, Jsi_Mutex *mtx) { MutexDone(mtx); Jsi_Free(mtx);}
//void Jsi_MutexInit(Jsi_Interp *interp, Jsi_Mutex *mtx) { MutexInit(mtx); }
void* Jsi_InterpThread(Jsi_Interp *interp) { return interp->threadId; }
void* Jsi_CurrentThread(void) {
#ifdef __WIN32
    return (void*)(uintptr_t)GetCurrentThreadId();
#else
    return (void*)pthread_self();
#endif
}

#else /* ! JSI_OMIT_THREADS */
Jsi_RC Jsi_MutexLock(Jsi_Interp *interp, Jsi_Mutex *mtx) { return JSI_OK; }
void Jsi_MutexUnlock(Jsi_Interp *interp, Jsi_Mutex *mtx) { }
Jsi_Mutex* Jsi_MutexNew(Jsi_Interp *interp, int timeout, int flags) { return NULL; }
void Jsi_MutexDelete(Jsi_Interp *interp, Jsi_Mutex *mtx) { }
void* Jsi_CurrentThread(void) { return NULL; }
void* Jsi_InterpThread(Jsi_Interp *interp) { return NULL; }
#endif

Jsi_Number Jsi_Version(void) {
    Jsi_Number d = JSI_VERSION;
    return d;
}

static const char *JsiCharsetMatch(const char *pattern, int c, int flags)
{
    int inot = 0;
    int pchar;
    int match = 0;
    int nocase = 0;

    if (flags & JSI_CMP_NOCASE) {
        nocase++;
        c = toupper(c);
    }

    if (flags & JSI_CMP_CHARSET_SCAN) {
        if (*pattern == '^') {
            inot++;
            pattern++;
        }

        /* Special case. If the first char is ']', it is part of the set */
        if (*pattern == ']') {
            goto first;
        }
    }

    while (*pattern && *pattern != ']') {
        /* Exact match */
        if (pattern[0] == '\\') {
first:
            pattern += Jsi_UtfToUniCharCase(pattern, &pchar, nocase);
        }
        else {
            /* Is this a range? a-z */
            int start;
            int end;
            pattern += Jsi_UtfToUniCharCase(pattern, &start, nocase);
            if (pattern[0] == '-' && pattern[1]) {
                /* skip '-' */
                pattern += Jsi_UtfToUniChar(pattern, &pchar);
                pattern += Jsi_UtfToUniCharCase(pattern, &end, nocase);

                /* Handle reversed range too */
                if ((c >= start && c <= end) || (c >= end && c <= start)) {
                    match = 1;
                }
                continue;
            }
            pchar = start;
        }

        if (pchar == c) {
            match = 1;
        }
    }
    if (inot) {
        match = !match;
    }

    return match ? pattern : NULL;
}


/* Split on char, or whitespace if ch==0. */
static void SplitChar(const char *str, int *argcPtr,
              char ***argvPtr, char ch, Jsi_DString *dStr)
{
    char *cp, *ep, *p, **argv;
    int cnt = 1, len, i;

    len = Jsi_Strlen(str);
    cp = (char*)str;
    while (*cp) {
        if (ch)
            cp = Jsi_Strchr(cp,ch);
        else {
            while (*cp && !isspace(*cp))
                cp++;
        }
        if (cp == NULL || *cp == 0) break;
        cp++;
        cnt++;
    }
    //argv = (char**)Jsi_Calloc(1,(sizeof(char*)*(cnt+3) + sizeof(char)*(len+6)));
    Jsi_DSSetLength(dStr, (sizeof(char*)*(cnt+3) + sizeof(char)*(len+6)));
    argv = (char**)Jsi_DSValue(dStr);
    *argvPtr = argv;
    *argcPtr = cnt;
    p = (char*)&(argv[cnt+2]);
    argv[cnt+1] = p;
    Jsi_Strcpy(p, str);
    cp = p;
    i = 0;
    argv[i++] = p;
    while (*cp) {
        if (ch)
            ep = Jsi_Strchr(cp,ch);
        else {
            ep = cp;
            while (*ep && !isspace(*ep))
                ep++;
        }
        if (ep == NULL || *ep == 0) break;
        *ep = 0;
        cp = ep+1;
        argv[i++] = cp;
    }
    argv[cnt] = NULL;
}

Jsi_RC
Jsi_GetIndex( Jsi_Interp *interp, const char *str,
    const char **tablePtr, const char *msg, int flags,
    int *indexPtr)
{
  const char *msg2 = "unknown ";
  char **cp, *c;
  int cond, index = -1, slen, i, dup = 0;
  int exact = (flags & JSI_CMP_EXACT);
  int nocase = (flags & JSI_CMP_NOCASE);
  slen = Jsi_Strlen(str);
 /* if (slen==0) 
        return Jsi_LogError("empty option %s %s", msg, str);*/
  cp = (char**)tablePtr;
  i = -1;
  while (*cp != 0) {
    i++;
    c = *cp;
    if (c[0] != str[0]) { cp++; continue; }
    if (!nocase)
        cond = (exact ? Jsi_Strcmp(c,str) : Jsi_Strncmp(c,str,slen));
    else {
        cond = (exact ? Jsi_Strncasecmp(c,str, -1) : Jsi_Strncasecmp(c,str,slen));
    }
    if (cond == 0) {
      if (index<0) {
        index = i;
      } else {
        dup = 1;
        break;
      }
    }
    cp++;
  }
  if (index >= 0 && dup == 0) {
    *indexPtr = index;
    return JSI_OK;
  }
  if (exact && (dup || index<=0)) {
    if (interp != NULL) {
      msg2 = (index>=0? "unknown ":"duplicate ");
    }
    goto err;
  }
  cp = (char**)tablePtr;
  i = -1;
  dup = 0;
  index = -1;
  while (*cp != 0) {
    i++;
    c = *cp;
    if (c[0] == str[0] && Jsi_Strncmp(c,str, slen) == 0) {
      if (index<0) {
        index = i;
        if (slen == (int)Jsi_Strlen(c))
            break;
      } else {
        if (interp != NULL) {
          msg2 = "ambiguous ";
        }
        goto err;
      }
    }
    cp++;
  }
  if (index >= 0 && dup == 0) {
    *indexPtr = index;
    return JSI_OK;
  }
err:
  if (interp != NULL) {
    Jsi_DString dStr = {};
    Jsi_DSAppend(&dStr, msg2, msg, " \"", str, "\" not one of: ", NULL);
    cp = (char**)tablePtr;
    while (*cp != 0) {
      c = *cp;
      Jsi_DSAppend(&dStr, c, NULL);
      Jsi_DSAppend(&dStr, " ", NULL);
      cp++;
    }
    Jsi_LogError("%s", Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
  }
  return JSI_ERROR;
}

bool Jsi_GlobMatch(const char *pattern, const char *string, int nocase)
{
    int c;
    int pchar;
    while (*pattern) {
        switch (pattern[0]) {
            case '*':
                while (pattern[1] == '*') {
                    pattern++;
                }
                pattern++;
                if (!pattern[0]) {
                    return 1;   /* match */
                }
                while (*string) {
                    if (Jsi_GlobMatch(pattern, string, nocase))
                        return 1;       /* match */
                    string += Jsi_UtfToUniChar(string, &c);
                }
                return 0;       /* no match */

            case '?':
                string += Jsi_UtfToUniChar(string, &c);
                break;

            case '[': {
                    string += Jsi_UtfToUniChar(string, &c);
                    pattern = JsiCharsetMatch(pattern + 1, c, nocase ? JSI_CMP_NOCASE : 0);
                    if (!pattern) {
                        return 0;
                    }
                    if (!*pattern) {
                        /* Ran out of pattern (no ']') */
                        continue;
                    }
                    break;
                }
            case '\\':
                if (pattern[1]) {
                    pattern++;
                }
                /* fall through */
            default:
                string += Jsi_UtfToUniCharCase(string, &c, nocase);
                Jsi_UtfToUniCharCase(pattern, &pchar, nocase);
                if (pchar != c) {
                    return 0;
                }
                break;
        }
        pattern += Jsi_UtfToUniCharCase(pattern, &pchar, nocase);
        if (!*string) {
            while (*pattern == '*') {
                pattern++;
            }
            break;
        }
    }
    if (!*pattern && !*string) {
        return 1;
    }
    return 0;
}

Jsi_Stack* Jsi_StackNew(void)
{
    Jsi_Stack *stack = (Jsi_Stack*)Jsi_Calloc(1, sizeof(Jsi_Stack));
    return stack;
}

void Jsi_StackFree(Jsi_Stack *stack)
{
    Jsi_Free(stack->vector);
    Jsi_Free(stack);
}

int Jsi_StackSize(Jsi_Stack *stack)
{
    return stack->len;
}

void Jsi_StackPush(Jsi_Stack *stack, void *element)
{
    int neededLen = stack->len + 1;

    if (neededLen > stack->maxlen) {
        stack->maxlen = neededLen < 20 ? 20 : neededLen * 2;
        stack->vector = (void**)Jsi_Realloc(stack->vector, sizeof(void *) * stack->maxlen);
    }
    stack->vector[stack->len] = element;
    stack->len++;
}

void *Jsi_StackPop(Jsi_Stack *stack)
{
    if (stack->len == 0)
        return NULL;
    stack->len--;
    return stack->vector[stack->len];
}

void *Jsi_StackUnshift(Jsi_Stack *stack)
{
    if (stack->len == 0)
        return NULL;
    stack->len--;
    void *rc = stack->vector[0];
    memmove(stack->vector, stack->vector+1, sizeof(void*)*stack->len);
    return rc;
}

void *Jsi_StackPeek(Jsi_Stack *stack)
{
    if (stack->len == 0)
        return NULL;
    return stack->vector[stack->len - 1];
}

void *Jsi_StackHead(Jsi_Stack *stack)
{
    if (stack->len == 0)
        return NULL;
    return stack->vector[0];
}

void Jsi_StackFreeElements(Jsi_Interp *interp, Jsi_Stack *stack, Jsi_DeleteProc *freeProc)
{
    int i;
    for (i = 0; i < stack->len; i++)
        freeProc(interp, stack->vector[i]);
    stack->len = 0;
}

typedef struct {
    void *data;
    Jsi_DeleteProc *delProc;
} AssocData;

/* Split on string. */
void Jsi_SplitStr(const char *str, int *argcPtr,
              char ***argvPtr, const char *ch, Jsi_DString *dStr)
{
    char *cp, *ep, *p, **argv;
    int cnt = 1, len, i, clen;
    if (!ch)
        ch = "";
    clen = Jsi_Strlen(ch);
    if (clen<=0)
        return SplitChar(str, argcPtr, argvPtr, *ch, dStr);
    len = Jsi_Strlen(str);
    cp = (char*)str;
    while (*cp) {
        cp = Jsi_Strstr(cp,ch);
 
        if (cp == NULL || *cp == 0) break;
        cp += clen;
        cnt++;
    }
    //argv = (char**)Jsi_Calloc(1,(sizeof(char*)*(cnt+3) + sizeof(char)*(len+6)));
    Jsi_DSSetLength(dStr, (sizeof(char*)*(cnt+3) + sizeof(char)*(len+6)));
    argv = (char**)Jsi_DSValue(dStr);
    *argvPtr = argv;
    *argcPtr = cnt;
    p = (char*)&(argv[cnt+2]);
    argv[cnt+1] = p;
    Jsi_Strcpy(p, str);
    cp = p;
    i = 0;
    argv[i++] = p;
    while (*cp) {
        ep = Jsi_Strstr(cp,ch);
        if (ep == NULL || *ep == 0) break;
        *ep = 0;
        cp = ep+clen;
        argv[i++] = cp;
    }
    argv[cnt] = NULL;
}

static Jsi_RC JsiCheckConversion(const char *str, const char *endptr)
{
    if (str[0] == '\0' || str == endptr) {
        return JSI_ERROR;
    }

    if (endptr[0] != '\0') {
        while (*endptr) {
            if (!isspace(UCHAR(*endptr))) {
                return JSI_ERROR;
            }
            endptr++;
        }
    }
    return JSI_OK;
}

static int JsiNumberBase(const char *str, int *base, int *sign)
{
    int i = 0;

    *base = 10;

    while (isspace(UCHAR(str[i]))) {
        i++;
    }

    if (str[i] == '-') {
        *sign = -1;
        i++;
    }
    else {
        if (str[i] == '+') {
            i++;
        }
        *sign = 1;
    }

    if (str[i] != '0') {
        /* base 10 */
        return 0;
    }

    /* We have 0<x>, so see if we can convert it */
    switch (str[i + 1]) {
        case 'x': case 'X': *base = 16; break;
        case 'o': case 'O': *base = 8; break;
        case 'b': case 'B': *base = 2; break;
        default: return 0;
    }
    i += 2;
    /* Ensure that (e.g.) 0x-5 fails to parse */
    if (str[i] != '-' && str[i] != '+' && !isspace(UCHAR(str[i]))) {
        /* Parse according to this base */
        return i;
    }
    /* Parse as base 10 */
    return 10;
}

/* Converts a number as per strtoull(..., 0) except leading zeros do *not*
 * imply octal. Instead, decimal is assumed unless the number begins with 0x, 0o or 0b
 */
static Jsi_Wide jsi_strtoull(const char *str, char **endptr)
{
#ifdef JSI__LONG_LONG
    int sign;
    int base;
    int i = JsiNumberBase(str, &base, &sign);

    if (base != 10) {
        Jsi_Wide value = strtoull(str + i, endptr, base);
        if (endptr == NULL || *endptr != str + i) {
            return value * sign;
        }
    }

    /* Can just do a regular base-10 conversion */
    return strtoull(str, endptr, 10);
#else
    return (unsigned long)jsi_strtol(str, endptr);
#endif
}

static Jsi_Wide jsi_strtoul(const char *str, char **endptr)
{
#ifdef JSI__LONG_LONG
    int sign;
    int base;
    int i = JsiNumberBase(str, &base, &sign);

    if (base != 10) {
        Jsi_Wide value = strtoul(str + i, endptr, base);
        if (endptr == NULL || *endptr != str + i) {
            return value * sign;
        }
    }

    /* Can just do a regular base-10 conversion */
    return strtoul(str, endptr, 10);
#else
    return (unsigned long)jsi_strtol(str, endptr);
#endif
}


Jsi_RC Jsi_GetWide(Jsi_Interp* interp, const char *string, Jsi_Wide *widePtr, int base)
{
    char *endptr;

    if (base) {
        *widePtr = strtoull(string, &endptr, base);
    }
    else {
        *widePtr = jsi_strtoull(string, &endptr);
    }

    return JsiCheckConversion(string, endptr);
}

Jsi_RC Jsi_GetInt(Jsi_Interp* interp, const char *string, int *n, int base)
{
    char *endptr;
    if (base) {
        *n = strtoul(string, &endptr, base);
    }
    else {
        *n = (int)jsi_strtoul(string, &endptr);
    }
    return JsiCheckConversion(string, endptr);
}

Jsi_RC Jsi_GetDouble(Jsi_Interp* interp, const char *string, Jsi_Number *n)
{
    char *endptr;

    /* Callers can check for underflow via ERANGE */
    errno = 0;

    *n = strtod(string, &endptr);

    return JsiCheckConversion(string, endptr);
}

Jsi_RC Jsi_GetBool(Jsi_Interp* interp, const char *string, bool *n)
{
    int len = Jsi_Strlen(string);
    if (len && (Jsi_Strncasecmp(string, "true", len)==0 && len<=4)) {
        *n = 1;
        return JSI_OK;
    }
    if (len && (Jsi_Strncasecmp(string, "false", len)==0 && len<=5)) {
        *n = 0;
        return JSI_OK;
    }
    return JSI_ERROR;
}

/* Converts a hex character to its integer value */
char jsi_fromHexChar(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char jsi_toHexChar(char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

void jsi_ToHexStr(const uchar *indata, int dlen, char *out) {
    static char hex[] = "0123456789abcdef";
    int i, n=0;
    for (i=0; i<dlen; i++) {
        int c = indata[i];
        out[n++] = hex[(c>>4)&0xf];
        out[n++] = hex[c&0xf];
    }
    out[n] = 0;
}

static int jsi_FromHexStr(const char *in, uchar *outdata) {
    int n = 0;
    while (in[0] && in[1]) {
        if (!isxdigit(in[0]) || isxdigit(in[0]))
            return -1;
        outdata[n++] = jsi_fromHexChar(in[0]) << 4 | jsi_fromHexChar(in[1]);
        in+=2;
    }
    return n;
}


int Jsi_HexStr(const uchar *data, int len, Jsi_DString *dStr, bool decode) {
    int olen = (decode?(len/2+1):(len*2+1));
    Jsi_DSSetLength(dStr, olen);
    if (!decode)
        return jsi_FromHexStr((const char*)data, (uchar*)Jsi_DSValue(dStr));
    jsi_ToHexStr((const uchar*)data, len, Jsi_DSValue(dStr));
    return olen-1;
}
#ifndef JSI_LITE_ONLY

void *Jsi_InterpGetData(Jsi_Interp *interp, const char *key, Jsi_DeleteProc **proc)
{
    Jsi_HashEntry *hPtr;
    AssocData *ptr;
    hPtr = Jsi_HashEntryFind(interp->assocTbl, key);
    if (!hPtr)
        return NULL;
    ptr = (AssocData *)Jsi_HashValueGet(hPtr);
    if (!ptr)
        return NULL;
    if (proc)
        *proc = ptr->delProc;
    return ptr->data;
}
void Jsi_InterpSetData(Jsi_Interp *interp, const char *key, void *data, Jsi_DeleteProc *proc)
{
    bool isNew;
    Jsi_HashEntry *hPtr;
    AssocData *ptr;
    hPtr = Jsi_HashEntryNew(interp->assocTbl, key, &isNew);
    if (!hPtr)
        return;
    if (isNew) {
        ptr = (AssocData *)Jsi_Calloc(1,sizeof(*ptr));
        Jsi_HashValueSet(hPtr, ptr);
    } else
        ptr = (AssocData *)Jsi_HashValueGet(hPtr);
    ptr->data = data;
    ptr->delProc = proc;
}

void jsi_DelAssocData(Jsi_Interp *interp, void *data) {
    AssocData *ptr = (AssocData *)data;
    if (!ptr) return;
    if (ptr->delProc)
        ptr->delProc(interp, ptr->data);
    Jsi_Free(ptr);
}

Jsi_RC Jsi_DeleteData(Jsi_Interp* interp, void *m)
{
    Jsi_Free(m);
    return JSI_OK;
}

void Jsi_InterpFreeData(Jsi_Interp *interp, const char *key)
{
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryFind(interp->assocTbl, key);
    if (!hPtr)
        return;
    Jsi_HashEntryDelete(hPtr);
}

Jsi_RC Jsi_GetStringFromValue(Jsi_Interp* interp, Jsi_Value *value, const char **n)
{
    if (!value)
        return JSI_ERROR;
    if (value->vt == JSI_VT_STRING)
    {
        *n = (const char*)value->d.s.str;
         return JSI_OK;
    }
    if (value->vt == JSI_VT_OBJECT && value->d.obj->ot == JSI_OT_STRING) {
        *n = value->d.obj->d.s.str;
        return JSI_OK;
    }
    Jsi_LogError("invalid string");
    return JSI_ERROR;
}

Jsi_RC Jsi_GetBoolFromValue(Jsi_Interp* interp, Jsi_Value *value, bool *n)
{
    if (!value)
        return JSI_ERROR;

    if (value->vt == JSI_VT_BOOL) {
        *n = value->d.val;
        return JSI_OK;
    }
    if (value->vt == JSI_VT_OBJECT && value->d.obj->ot == JSI_OT_BOOL) {
        *n = value->d.obj->d.val;
        return JSI_OK;
    }
    Jsi_LogError("invalid bool");
    return JSI_ERROR;
}


Jsi_RC Jsi_GetNumberFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Number *n)
{
    if (!value)
        return JSI_ERROR;

    if (value->vt == JSI_VT_NUMBER) {
        *n = value->d.num;
        return JSI_OK;
    }
    if (value->vt == JSI_VT_OBJECT && value->d.obj->ot == JSI_OT_NUMBER) {
        *n = value->d.obj->d.num;
        return JSI_OK;
    }
    if (interp)
        Jsi_LogError("invalid number");
    return JSI_ERROR;
}

Jsi_RC Jsi_GetIntFromValueBase(Jsi_Interp* interp, Jsi_Value *value, int *n, int base, int flags)
{
    int noMsg = (flags & JSI_NO_ERRMSG);
    /* TODO: inefficient to convert to double then back. */
    if (!value)
        return JSI_ERROR;
    Jsi_Number d = Jsi_ValueToNumberInt(interp, value, 1);
    if (!Jsi_NumberIsFinite(d))
    {
        if (!noMsg)
            Jsi_LogError("invalid number");
        return JSI_ERROR;
    }
    Jsi_ValueReset(interp,&value);
    Jsi_ValueMakeNumber(interp, &value, d);
    *n = (int)d;
    return JSI_OK;
}

Jsi_RC Jsi_GetIntFromValue(Jsi_Interp* interp, Jsi_Value *value, int *n)
{
    if (!Jsi_ValueIsNumber(interp, value)) 
        return Jsi_LogError("invalid number");
    return Jsi_GetIntFromValueBase(interp, value, n, 0, 0);
}

Jsi_RC Jsi_GetLongFromValue(Jsi_Interp* interp, Jsi_Value *value, long *n)
{
    /* TODO: inefficient to convert to double then back. */
    if (!value)
        return JSI_ERROR;
    if (!interp->strict)
        jsi_ValueToOInt32(interp, value);
    if (!Jsi_ValueIsNumber(interp, value))
    
        return Jsi_LogError("invalid number");
    *n = (long)(value->vt == JSI_VT_NUMBER ? value->d.num : value->d.obj->d.num);
    return JSI_OK;
}

Jsi_RC Jsi_GetWideFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Wide *n)
{
    if (!value)
        return JSI_ERROR;
    if (!interp->strict)
        jsi_ValueToOInt32(interp, value);
    if (!Jsi_ValueIsNumber(interp, value))
    
        return Jsi_LogError("invalid number");
    *n = (Jsi_Wide)(value->vt == JSI_VT_NUMBER ? value->d.num : value->d.obj->d.num);
    return JSI_OK;

}

Jsi_RC Jsi_GetDoubleFromValue(Jsi_Interp* interp, Jsi_Value *value, Jsi_Number *n)
{
    if (!value)
        return JSI_ERROR;
    if (!interp->strict)
        Jsi_ValueToNumber(interp, value);
    if (!Jsi_ValueIsNumber(interp, value))
    
        return Jsi_LogError("invalid number");
    *n = (value->vt == JSI_VT_NUMBER ? value->d.num : value->d.obj->d.num);
    return JSI_OK;
}

Jsi_RC
Jsi_ValueGetIndex( Jsi_Interp *interp, Jsi_Value *valPtr,
    const char **tablePtr, const char *msg, int flags, int *indexPtr)
{
    char *val = Jsi_ValueString(interp, valPtr, NULL);
    if (val == NULL) 
        return Jsi_LogError("expected string");
    return Jsi_GetIndex(interp, val, tablePtr, msg, flags, indexPtr);
}

#endif // JSI_LITE_ONLY
