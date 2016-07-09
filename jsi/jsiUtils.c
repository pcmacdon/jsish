#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif
#include <errno.h>
#include <sys/time.h>

#ifdef HAVE_READLINE
# include <readline/readline.h>
# include <readline/history.h>
#endif

void* Jsi_Realloc(void *m,unsigned int size) {
    void *v = realloc(m,size);
    return v;
}
void* Jsi_Malloc(unsigned int size) {
    void *v = malloc(size);
    return v;
}
void* Jsi_Calloc(unsigned int n,unsigned int size) {
    void *v = calloc(n,size);
    return v;
}
void  Jsi_Free(void *n) { free(n); }

/* Get time in milliseconds since Jan 1, 1970 */
Jsi_Number Jsi_DateTime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    Jsi_Number num = ((Jsi_Number)tv.tv_sec*1000.0 + (Jsi_Number)tv.tv_usec/1000.0);
    return num;
}


#ifndef JSI_LITE_ONLY

int jsi_fatalexit = JSI_LOG_FATAL;

static void (*logHook)(const char *buf, va_list va) = NULL;

void Jsi_LogMsg(Jsi_Interp *interp, int code, const char *format,...) {
    va_list va;
    va_start (va, format);
    char pbuf[BUFSIZ/8] = "";
    char buf[BUFSIZ/2];
    const char *mt = "", *term = "", *pterm=pbuf;
    static char lastMsg[BUFSIZ/2] = "";
    static int lastCnt = 0;
    if (interp==NULL)
        interp = jsiMainInterp;

    const char *curFile = NULL;
    if (interp)
        curFile = (interp->curIp && interp->curIp->fname? interp->curIp->fname:interp->curFile);
    if (!curFile) curFile = "";
    /* Filter out try/catch (TODO: and non-syntax errors??). */
    if (code == JSI_LOG_PARSE || interp == NULL) {
        if (logHook)
            (*logHook)(format, va);
        else
            vfprintf(stderr, format, va);
        va_end(va);
        return;
    }
    if (code == JSI_LOG_ERROR && (interp->tryDepth-interp->withDepth)>0 && interp->inParse<=0 
        /*&& !interp->errMsgBuf[0]*/) { /* TODO: should only do the first or traceback? */
        vsnprintf(interp->errMsgBuf, sizeof(interp->errMsgBuf), format, va);
        interp->errFile = curFile;
        interp->errLine = interp->curIp->line;
        va_end(va);
        return;
    }
    switch (code) {
        case JSI_LOG_FATAL: mt = "fatal"; break;
        case JSI_LOG_ERROR: mt = "error"; break;
        case JSI_LOG_PARSE: mt = "parse"; break;
        case JSI_LOG_WARN:  mt = "warning"; break;
        case JSI_LOG_INFO: mt = "info"; break;
        case JSI_LOG_BUG: mt = "bug"; break;
        case JSI_LOG_TODO: mt = "todo"; break;
    } 
    if (!strchr(format,'\n')) term = "\n";
    if (interp && interp->lastPushStr && interp->lastPushStr[0]) {
        char *ss = interp->lastPushStr;
        char psbuf[BUFSIZ/6];
        if (strchr(ss,'%')) {
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
        if (*ss)
            snprintf(pbuf, sizeof(pbuf), "    (at or near \"%s\")\n", ss);
    }
    pbuf[sizeof(pbuf)-1] = 0;
    int line = 0;
    if (interp->parseLine)
        line = interp->parseLine->first_line;
    else if (interp->curIp)
        line = interp->curIp->line;
    if (interp && line>0 && curFile && strchr(curFile,'%')==0)
        snprintf(buf, sizeof(buf), "%s:%d: %s: %s%s%s",  curFile, line, mt,format, pterm, term);
    else
        snprintf(buf, sizeof(buf), "%s: %s%s%s", mt, format, pterm, term);
    buf[sizeof(buf)-1]=0;

    if (logHook)
        (*logHook)(buf, va);
    else if (interp->logAllowDups)
        vfprintf(stderr, buf, va);
    else {
        char buf1[BUFSIZ/2];
        vsnprintf(buf1, sizeof(buf1), buf, va);
        if (buf1[0] && lastCnt && strcmp(buf1, lastMsg)==0) {
            lastCnt++;
            goto done;
        } else if (lastMsg[0] && lastCnt>1 ) {
            fprintf(stderr, "REPEAT: Last msg repeated %d times...\"\n" ,lastCnt);
        }
        if (buf1[0] == 0 || (buf1[0] == '.' && buf1[1] == 0))
            goto done;
        lastCnt = 1;
        strcpy(lastMsg, buf1);
        fputs(buf1, stderr);
    }
done:
    va_end(va);
    if (code & jsi_fatalexit)
        exit(1);
}

const char* Jsi_KeyAdd(Jsi_Interp *interp, const char *str)
{
    Jsi_HashEntry *hPtr;
    int isNew;
    hPtr = Jsi_HashEntryNew(interp->strKeyTbl, str, &isNew);
    assert(hPtr) ;
    return (const char*)Jsi_HashKeyGet(hPtr);
}

const char* Jsi_KeyLookup(Jsi_Interp *interp, const char *str)
{
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryFind(interp->strKeyTbl, str);
    if (!hPtr) {
        return NULL;
    }
    return (const char*)Jsi_HashKeyGet(hPtr);
}

extern int Jsi_PkgProvide(Jsi_Interp *interp, const char *name, const char *version)
{
    return JSI_ERROR;
}

extern int Jsi_PkgRequire(Jsi_Interp *interp, const char *name, const char *version)
{
    return JSI_ERROR;
}

Jsi_Value *Jsi_VarLookup(Jsi_Interp *interp, const char *varname)
{
    Jsi_Value *v;
    v = Jsi_ValueObjLookup(interp, interp->incsc, (char*)varname, 0);
    if (!v)
        v = jsi_ScopeChainObjLookupUni(interp->ingsc, (char*)varname);
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
        return strchr(cp, ']');
}

/* Lookup a name, eg.  "a[b].c  a.b.c  a[b][c]  a.b[c]  a["b"].c  a[1].c  */
Jsi_Value *Jsi_NameLookup(Jsi_Interp *interp, const char *name)
{
    int cnt = 0, len, isq;
    char *nam = (char*)name, *cp, *cp2, *ocp, *kstr;
    DECL_VALINIT(tv);
    DECL_VALINIT(nv);
    DECL_VALINIT(key);
    Jsi_Value *v = NULL, *nvPtr = &nv;
    Jsi_Value *kPtr = &key; // Note: a string key so no reset needed.
    Jsi_DString dStr = {};
    cp2 = strchr(nam,'[');
    cp = strchr(nam, '.');
    if (cp2 && (cp==0 || cp2<cp))
        cp = cp2;
    if (!cp)
        return Jsi_VarLookup(interp, nam);
    Jsi_DSSetLength(&dStr, 0);
    Jsi_DSAppendLen(&dStr, nam, cp-nam);
    v = Jsi_VarLookup(interp, Jsi_DSValue(&dStr));
    if (!v)
        goto bail;
    while (cnt++ < 1000) {
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
            cp2 = strchr(nam,'[');
            cp = strchr(nam, '.');
            if (cp2 && (cp==0 || cp2<cp))
                cp = cp2;
            len = (cp ? cp-nam : strlen(nam));
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
            jsi_ValueSubscriptLen(interp, v, kv, &nvPtr, 1);
            goto keyon;
        } else {
            Jsi_ValueMakeStringKey(interp, &kPtr, kstr); 
            jsi_ValueSubscriptLen(interp, v, kPtr, &nvPtr, 1);
keyon:
            if (nv.vt == JSI_VT_UNDEF)
                goto bail;
            else {
                tv = nv;
                v = &tv;
            }
        }
        if (cp == 0 || *cp == 0) break;
    }
    //Jsi_ValueReset(interp, &ret);
    Jsi_DSFree(&dStr);
    if (v && v == &tv) {
        v = Jsi_ValueNew(interp);
        //Jsi_ValueMove(interp, v, &tv);
#ifdef JSI_MEM_DEBUG
        memcpy(v, &tv, sizeof(tv)-sizeof(tv.VD));
        v->VD.label3 = tv.VD.func;
#else
        *v = tv;
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

static int IsAlnum(const char *cp)
{
    while (*cp)
        if (isalnum(*cp) || *cp == '_')
            cp++;
        else
            return 0;
    return 1;
}

static void jsiValueGetString(Jsi_Interp *interp, Jsi_Value* v, Jsi_DString *dStr, objwalker *owPtr);

static int IsKeyword(Jsi_Interp *interp, char *str) {
    return (Jsi_HashEntryFind(interp->lexkeyTbl, str) != 0);
}

static int _object_get_callback(Jsi_Tree *tree, Jsi_TreeEntry *hPtr, void *data)
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
    if (len>=2 && cp[len-2] != '{')
        Jsi_DSAppend(dStr, ", ", NULL);
    if (((ow->quote&JSI_OUTPUT_JSON) == 0 || (ow->quote&JSI_JSON_STRICT) == 0) && IsAlnum(str) && !IsKeyword(tree->interp, str))
        Jsi_DSAppend(dStr, str, NULL);
    else
        /* JSON/spaces, etc requires quoting the name. */
        Jsi_DSAppend(dStr, "\"", str, "\"", NULL);
    Jsi_DSAppend(dStr, ":", NULL);
    ow->depth++;
    jsiValueGetString(tree->interp, v, dStr, ow);
    ow->depth--;
    return JSI_OK;
}

/* Format value into dStr.  Toplevel caller does init/free. */
static void jsiValueGetString(Jsi_Interp *interp, Jsi_Value* v, Jsi_DString *dStr, objwalker *owPtr)
{
    char buf[100], *str;
    if (owPtr->depth > interp->maxDepth) {
        Jsi_LogError("recursive ToString");
        return;
    }
    int quote = owPtr->quote;
    int isjson = owPtr->quote&JSI_OUTPUT_JSON;
    double num;
    switch(v->vt) {
        case JSI_VT_UNDEF:
            Jsi_DSAppend(dStr, "undefined", NULL);
            return;
        case JSI_VT_NULL:
            Jsi_DSAppend(dStr, "null", NULL);
            return;
        case JSI_VT_VARIABLE:
            Jsi_DSAppend(dStr, "variable", NULL);
            return;
        case JSI_VT_BOOL:
            Jsi_DSAppend(dStr, (v->d.val ? "true":"false"), NULL);
            return;
        case JSI_VT_NUMBER:
            num = v->d.num;
outnum:
            if (jsi_is_integer(num)) {
                sprintf(buf, "%d", (int)num);
                Jsi_DSAppend(dStr, buf, NULL);
            } else if (jsi_is_wide(num)) {
                sprintf(buf, "%Ld", (Jsi_Wide)num);
                Jsi_DSAppend(dStr, buf, NULL);
            } else if (jsi_ieee_isnormal(num)) {
                sprintf(buf, "%" JSI_NUMGFMT, num);
                Jsi_DSAppend(dStr, buf, NULL);
            } else if (jsi_ieee_isnan(num)) {
                Jsi_DSAppend(dStr, "NaN", NULL);
            } else {
                int s = jsi_ieee_infinity(num);
                if (s > 0) Jsi_DSAppend(dStr, "+Infinity", NULL);
                else if (s < 0) Jsi_DSAppend(dStr, "-Infinity", NULL);
                else Jsi_LogBug("Ieee function problem");
            }
            return;
        case JSI_VT_STRING:
            str = v->d.s.str;
outstr:
            if (!quote) {
                Jsi_DSAppend(dStr, str, NULL);
                return;
            }
            Jsi_DSAppend(dStr,"\"", NULL);
            while (*str) {
                if ((*str == '\'' && (!(isjson))) || *str == '\"' ||
                    (*str == '\n' && (!(owPtr->quote&JSI_OUTPUT_NEWLINES))) || *str == '\r' || *str == '\t' || *str == '\f' || *str == '\b'  ) {
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
                } else if (!isprint(*str))
                    /* TODO: encode */
                    if (isjson) {
                        char ubuf[10];
                        sprintf(ubuf, "\\u00%.02x", (unsigned char)*str);
                        Jsi_DSAppend(dStr,ubuf, NULL);
                        //Jsi_DSAppendLen(dStr,".", 1);
                    } else
                        Jsi_DSAppendLen(dStr,str, 1);
                    
                else
                    Jsi_DSAppendLen(dStr,str, 1);
                str++;
            }
            Jsi_DSAppend(dStr,"\"", NULL);
            return;
        case JSI_VT_OBJECT: {
            Jsi_Obj *o = v->d.obj;
            switch(o->ot) {
                case JSI_OT_BOOL:
                    Jsi_DSAppend(dStr, (o->d.val ? "true":"false"), NULL);
                    return;
                case JSI_OT_NUMBER:
                    num = o->d.num;
                    goto outnum;
                    return;
                case JSI_OT_STRING:
                    str = o->d.s.str;
                    goto outstr;
                    return;
                case JSI_OT_FUNCTION: {
                    Jsi_FuncObjToString(interp, o, dStr);
                    return;
                }
                case JSI_OT_REGEXP:
                    Jsi_DSAppend(dStr, o->d.robj->pattern, NULL);
                    return;
                case JSI_OT_USEROBJ: 
                    jsi_UserObjToName(interp, o->d.uobj, dStr);
                    return;
                case JSI_OT_ITER:
                    Jsi_DSAppend(dStr, "*ITER*", NULL);
                    return;
            }
                        
            if (o->isarrlist)
            {
                Jsi_Value *nv;
                int i, len = o->arrCnt;
                
                if (!o->arr)
                    len = Jsi_ValueGetLength(interp, v);
                Jsi_DSAppend(dStr,"[ ", NULL);
                for (i = 0; i < len; ++i) {
                    nv = Jsi_ValueArrayIndex(interp, v, i);
                    if (i) Jsi_DSAppend(dStr,", ", NULL);
                    if (nv) jsiValueGetString(interp, nv, dStr, owPtr);
                    else Jsi_DSAppend(dStr, "undefined", NULL);
                }
                Jsi_DSAppend(dStr," ]", NULL);
            } else {
                Jsi_DSAppend(dStr,"{ ", NULL);
                owPtr->depth++;
                Jsi_TreeWalk(o->tree, _object_get_callback, owPtr, 0);
                owPtr->depth--;
                Jsi_DSAppend(dStr," }", NULL);
            }
            return;
        }
        default:
            Jsi_LogBug("Unexpected value type: %d\n", v->vt);
    }
}

/* Format value into dStr.  Toplevel caller does init/free. */
const char* Jsi_ValueGetDString(Jsi_Interp *interp, Jsi_Value* v, Jsi_DString *dStr, int quote)
{
    objwalker ow;
    ow.quote = quote;
    ow.depth = 0;
    ow.dStr = dStr;
    Jsi_DSInit(dStr);
    jsiValueGetString(interp, v, dStr, &ow);
    return Jsi_DSValue(dStr);
}

void Jsi_Puts(Jsi_Interp* interp, Jsi_Value *v, int flags)
{
    int quote = (flags&JSI_OUTPUT_QUOTE);
    int iserr = (flags&JSI_OUTPUT_STDERR);
    Jsi_DString dStr = {};
    const char *cp = Jsi_ValueString(interp, v, 0);
    if (cp) {
        fprintf((iserr?stderr:stdout),"%s", cp);
        return;
    }
    Jsi_DSInit(&dStr);
    Jsi_ValueGetDString(interp, v, &dStr, quote);
    fprintf((iserr?stderr:stdout),"%s",Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return;
}

extern int yyparse(jsi_Pstate *ps);

char* jsi_KeyFind(Jsi_Interp *interp, const char *str, int nocreate, int *isKey)
{
    Jsi_HashEntry *hPtr;
    if (isKey) *isKey = 0;
    if (!nocreate) {
        *isKey = 1;
         if (isKey) *isKey = 1;
        return (char*)Jsi_KeyAdd(interp, str);
    }
    hPtr = Jsi_HashEntryFind(interp->strKeyTbl, str);
    if (!hPtr) {
        return Jsi_Strdup(str);;
    }
    if (isKey) *isKey = 1;
    *isKey = 1;
    return (char*)Jsi_HashKeyGet(hPtr);
}

static int balanced(char *str) {
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

static char *get_inputline(int istty, const char *prompt)
{
    char *res;
#ifdef HAVE_READLINE
    if (istty) {
        res = readline(prompt);
        if (res && *res) add_history(res);
        return res;
    }
#endif
    int done = 0;
    char bbuf[BUFSIZ];
    Jsi_DString dStr = {};
    if (istty)
        fputs(prompt, stdout);
    fflush(stdout);
    while (!done) { /* Read a line. */
        bbuf[0] = 0;
        if (fgets(bbuf, sizeof(bbuf), stdin) == NULL)
            return NULL;
        Jsi_DSAppend(&dStr, bbuf, NULL);
        if (strlen(bbuf) < (sizeof(bbuf)-1) || bbuf[sizeof(bbuf)-1] == '\n')
            break;
    }
    res = strdup(Jsi_DSValue(&dStr));
    Jsi_DSFree(&dStr);
    return res;
}

static Jsi_Interp* interactiveInterp = NULL;
#ifdef HAVE_READLINE
static Jsi_Value *completeValues = NULL;
static int jsiRlStart = 0;

static char *jsiRlCmdMatches(const char *text, int state) {
    static int idx, len;
    const char *name;
    Jsi_Interp* interp = interactiveInterp;
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
        Jsi_Interp* interp = interactiveInterp;
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
            //fprintf(stderr, "CCCCC: %s %d %d\n", str, start, end);
            rc = Jsi_FunctionInvoke(interp, func, args, &completeValues, interp->csc);
            if (rc != JSI_OK)
                fprintf(stderr, "bad completion");
        }
        matches = rl_completion_matches(str, jsiRlCmdMatches);
    }
    return matches;
}
#endif

/* Collect and execute code from stdin.  The first byte of flags are passed to Jsi_ValueGetDString(). */
int Jsi_Interactive(Jsi_Interp* interp, int flags) {
    int rc = 0, done = 0, len, quote = (flags & 0xff), istty = 1;
    const char *prompt = "# ";
    char *buf;
    if (interactiveInterp) {
        Jsi_LogError("multiple interactive not supported");
        return JSI_ERROR;
    }
    interactiveInterp = interp;
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
#ifndef __WIN32
    istty = isatty(fileno(stdin));
#else
    istty = _isatty(_fileno(stdin));
#endif
#ifdef HAVE_READLINE
    Jsi_DString dHist = {};
    char *hist = NULL;
    rl_attempted_completion_function = jsiRlGetMatches;
    if(interp->noreadline == 0 && !interp->parent)
    {
        hist = Jsi_NormalPath(interp, "~/.jsish_history", &dHist);
        if (hist)
            read_history(hist);
    }
#endif
    interp->level++;
    while (done==0 && interp->exited==0) {
        buf = get_inputline(istty, prompt);
        if (buf) {
          Jsi_DSAppend(&dStr, buf, NULL);
          free(buf);
        } else {
          done = 1;
        }
        len = Jsi_DSLength(&dStr);
        if (done && len == 0)
            break;
        buf = Jsi_DSValue(&dStr);
        if (done == 0 && (!balanced(buf))) {
            prompt = "> ";
            if (len<5)
                break;
            continue;
        }
        prompt = "# ";
        while ((len = Jsi_Strlen(buf))>0 && (isspace(buf[len-1])))
            buf[len-1] = 0;
        if (buf[0] == 0)
            continue;
        if (interp->onEval == NULL) {
            /* Convenience: add semicolon to "var" statements (required by parser). */
            if (strncmp(buf,"var ", 4) == 0 && strchr(buf, '\n')==NULL && strchr(buf, ';')==NULL)
                strcat(buf, ";");
                rc = Jsi_EvalString(interp, buf, JSI_EVAL_RETURN);
        }
        else
        {
            Jsi_Value *func = interp->onEval;
            if (func && Jsi_ValueIsFunction(interp, func)) {
                Jsi_Value *items[1] = {};
                items[0] = Jsi_ValueNewStringDup(interp, buf);
                Jsi_Value *args = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, 1, 0));
                rc = Jsi_FunctionInvoke(interp, func, args, &interp->retPtr, interp->csc);
                if (rc != JSI_OK)
                    fprintf(stderr, "bad eval");
            }
        }
        if (interp->exited)
            break;
        if (rc == 0) {
             if (interp->retPtr->vt != JSI_VT_UNDEF || interp->noUndef==0) {
                Jsi_DString eStr = {};
                fputs(Jsi_ValueGetDString(interp, interp->retPtr, &eStr, quote), stdout);
                Jsi_DSFree(&eStr);
                fputs("\n", stdout);
             }
        } else if (!interp->exited) {
            fputs("ERROR\n", stderr);
        }
        Jsi_DSSetLength(&dStr, 0);
        len = 0;
    }
    interp->level--;
#ifdef HAVE_READLINE
    if (hist) {
        stifle_history(100);
        write_history(hist);
    }
    Jsi_DSFree(&dHist);
#endif
    Jsi_DSFree(&dStr);
    if (interp->exited && interp->level <= 0)
    {
        rc = interp->exitCode;
        Jsi_InterpDelete(interp);
    } else if (interp == interp->mainInterp)
        interp->retPtr = NULL;
    interactiveInterp = NULL;
    return rc;
}

int Jsi_PackageProvide(Jsi_Interp *interp, const char *name, const char *version)
{
    return JSI_OK;
}
int Jsi_PackageRequire(Jsi_Interp *interp, const char *name, const char *version)
{
    return JSI_OK;
}

int Jsi_ThisDataSet(Jsi_Interp *interp, Jsi_Value *_this, void *value)
{
    int isNew;
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryNew(interp->thisTbl, _this, &isNew);
    if (!hPtr)
        return -1;
    Jsi_HashValueSet(hPtr, value);
    return isNew;
}

void *Jsi_ThisDataGet(Jsi_Interp *interp, Jsi_Value *_this)
{
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryFind(interp->thisTbl, _this);
    if (!hPtr)
        return NULL;
    return Jsi_HashValueGet(hPtr);
}

int Jsi_PrototypeDefine(Jsi_Interp *interp, const char *key, Jsi_Value *value)
{
    int isNew;
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryNew(interp->protoTbl, key, &isNew);
    if (!hPtr)
        return -1;
    Jsi_HashValueSet(hPtr, value);
    return isNew;
}

void *Jsi_PrototypeGet(Jsi_Interp *interp, const char *key)
{
    Jsi_HashEntry *hPtr;
    hPtr = Jsi_HashEntryFind(interp->protoTbl, key);
    if (!hPtr)
        return NULL;
    return Jsi_HashValueGet(hPtr);
}

int Jsi_PrototypeObjSet(Jsi_Interp *interp, const char *key, Jsi_Obj *obj)
{
    Jsi_HashEntry *hPtr;
    Jsi_Value *val;
    hPtr = Jsi_HashEntryFind(interp->protoTbl, key);
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
        case JSI_OT_ARRAY: return "array"; break;  
        case JSI_OT_OBJECT: return "object"; break;
        case JSI_OT_USEROBJ:
            if (o->__proto__) {
                Jsi_HashEntry *hPtr;
                Jsi_HashSearch search;
                            
                for (hPtr = Jsi_HashEntryFirst(interp->thisTbl,&search); hPtr != NULL;
                    hPtr = Jsi_HashEntryNext(&search))
                    if (Jsi_HashValueGet(hPtr) == o->__proto__)
                        return (char*)Jsi_HashKeyGet(hPtr);
            }
            
            return "userobj";
            break;
            //return Jsi_ObjGetType(interp, v->d.obj);
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

/* Shim to instantiate a new obj-command and return its userobj data pointer for C use. */
void *Jsi_NewCmdObj(Jsi_Interp *interp, char *name, char *arg1, char *opts, char *var) {
    char buf[BUFSIZ];
    if (arg1)
        sprintf(buf, "%s%snew %s('%s', %s);", var?var:"", var?"=":"return ", name, arg1, opts?opts:"null");
    else
        sprintf(buf, "%s%snew %s(%s);", var?var:"", var?"=":"return ", name, opts?opts:"null");
    int rc = Jsi_EvalString(interp, buf, 0);
    if (rc != JSI_OK)
        return NULL;
    Jsi_Value *vObj = interp->retPtr;
    if (var)
        vObj = Jsi_NameLookup(interp, var);
    if (!vObj)
        return NULL;
    return Jsi_UserObjGetData(interp, vObj, NULL);
}

#endif

Jsi_Map* Jsi_MapNew (Jsi_Interp *interp, Jsi_Map_Type listType, unsigned int keyType, Jsi_HashDeleteProc *freeProc)
{
    Jsi_Map *lPtr, lval;
    lval.typ = listType;
    switch (listType) {
        case JSI_MAP_HASH: lval.v.hash = Jsi_HashNew(interp, keyType, freeProc); break;
        case JSI_MAP_TREE: lval.v.tree = Jsi_TreeNew(interp, keyType, freeProc); break;
        default: return NULL;
    }
    if (!lval.v.hash) return NULL;
    lPtr = (Jsi_Map*)Jsi_Malloc(sizeof(*lPtr));
    *lPtr = lval;
    return lPtr;
}

void Jsi_MapDelete (Jsi_Map *listPtr) {
    switch (listPtr->typ) {
        case JSI_MAP_HASH: Jsi_HashDelete(listPtr->v.hash); break;
        case JSI_MAP_TREE: Jsi_TreeDelete(listPtr->v.tree); break;
        default: return;
    }
    Jsi_Free(listPtr);
}
Jsi_MapEntry* Jsi_MapSet(Jsi_Map *listPtr, void *key, void *value){
    switch (listPtr->typ) {
        case JSI_MAP_HASH: Jsi_HashSet(listPtr->v.hash, key, value); break;
        case JSI_MAP_TREE: Jsi_TreeSet(listPtr->v.tree, key, value); break;
    }
    return NULL;
}
void* Jsi_MapGet(Jsi_Map *listPtr, void *key){
    switch (listPtr->typ) {
        case JSI_MAP_HASH: return Jsi_HashGet(listPtr->v.hash, key);
        case JSI_MAP_TREE: return Jsi_TreeGet(listPtr->v.tree, key);
    }
    return NULL;
}
static int jsi_GetListType(Jsi_MapEntry *h) {
    Jsi_HashEntry *hPtr =(Jsi_HashEntry *)h;
    return hPtr->typ;
}
void* Jsi_MapKeyGet(Jsi_MapEntry *h){
    switch (jsi_GetListType(h)) {
        case JSI_MAP_HASH: return Jsi_HashKeyGet((Jsi_HashEntry*)h);
        case JSI_MAP_TREE: return Jsi_TreeKeyGet((Jsi_TreeEntry*)h);
    }
    return NULL;
}
#ifndef JSI_LITE_ONLY
int Jsi_MapKeysDump(Jsi_Interp *interp, Jsi_Map *listPtr, Jsi_Value **ret, int flags){
    switch (listPtr->typ) {
        case JSI_MAP_HASH: return Jsi_HashKeysDump(interp, (Jsi_Hash*)listPtr, ret, flags);
        case JSI_MAP_TREE: return Jsi_TreeKeysDump(interp, (Jsi_Tree*)listPtr, ret, flags);
    }
    return JSI_ERROR;
}
#endif
void* Jsi_MapValueGet(Jsi_MapEntry *h){
    switch (jsi_GetListType(h)) {
        case JSI_MAP_HASH: return Jsi_HashValueGet((Jsi_HashEntry*)h);
        case JSI_MAP_TREE: return Jsi_TreeValueGet((Jsi_TreeEntry*)h);
    }
    return NULL;
}
void Jsi_MapValueSet(Jsi_MapEntry *h, void *value){
    switch (jsi_GetListType(h)) {
        case JSI_MAP_HASH: return Jsi_HashValueSet((Jsi_HashEntry*)h, value);
        case JSI_MAP_TREE: return Jsi_TreeValueSet((Jsi_TreeEntry*)h, value);
    }
}
Jsi_MapEntry* Jsi_MapEntryFind (Jsi_Map *listPtr, const void *key){
    switch (listPtr->typ) {
        case JSI_MAP_HASH: return (Jsi_MapEntry*)Jsi_HashEntryFind(listPtr->v.hash, key);
        case JSI_MAP_TREE: return (Jsi_MapEntry*)Jsi_TreeEntryFind(listPtr->v.tree, key);
    }
    return NULL;
}
Jsi_MapEntry* Jsi_MapEntryNew (Jsi_Map *listPtr, const void *key, int *isNew){
    switch (listPtr->typ) {
        case JSI_MAP_HASH: return (Jsi_MapEntry*)Jsi_HashEntryNew(listPtr->v.hash, key, isNew);
        case JSI_MAP_TREE: return (Jsi_MapEntry*)Jsi_TreeEntryNew(listPtr->v.tree, key, isNew);
    }
    return NULL;
}
int Jsi_MapEntryDelete (Jsi_MapEntry *entryPtr){
    switch (jsi_GetListType(entryPtr)) {
        case JSI_MAP_HASH: return Jsi_HashEntryDelete((Jsi_HashEntry*)entryPtr);
        case JSI_MAP_TREE: return Jsi_TreeEntryDelete((Jsi_TreeEntry*)entryPtr);
    }
    return 0;
}
Jsi_MapEntry* Jsi_MapEntryFirst (Jsi_Map *listPtr, Jsi_MapSearch *searchPtr){
    searchPtr->typ = listPtr->typ;
    switch (listPtr->typ) {
        case JSI_MAP_HASH: return (Jsi_MapEntry*)Jsi_HashEntryFirst(listPtr->v.hash, &searchPtr->v.hash);
        case JSI_MAP_TREE: return (Jsi_MapEntry*)Jsi_TreeEntryFirst(listPtr->v.tree, &searchPtr->v.tree, 0);
    }
    return NULL;
}
Jsi_MapEntry* Jsi_MapEntryNext (Jsi_MapSearch *searchPtr){
    switch (searchPtr->typ) {
        case JSI_MAP_HASH: return (Jsi_MapEntry*)Jsi_HashEntryNext(&searchPtr->v.hash);
        case JSI_MAP_TREE: return (Jsi_MapEntry*)Jsi_TreeEntryNext(&searchPtr->v.tree);
    }
    return NULL;
}
int Jsi_MapSize(Jsi_Map *listPtr) {
    switch (listPtr->typ) {
        case JSI_MAP_HASH: return Jsi_HashSize(listPtr->v.hash);
        case JSI_MAP_TREE: return Jsi_TreeSize(listPtr->v.tree);
    }
    return -1;
}

// List

    
Jsi_List *Jsi_ListNew(Jsi_ListAttr *attr)
{
    Jsi_List *list = (Jsi_List *)Jsi_Calloc(1, sizeof(Jsi_List) + (attr && attr->valueSpace>0?attr->valueSpace:0));
    SIGINIT(list, LIST);
    if (attr)
        list->attr = *attr;
    if ((list->attr.useMutex) && ((list->attr.lockProc == NULL) || (list->attr.lockProc == Jsi_ListLock))) {
        list->mutex = Jsi_MutexNew(NULL, 0, 0);
        if (list->mutex) {
            list->attr.lockProc = Jsi_ListLock;
        }
    }
    return list;
}

void Jsi_ListDelete(Jsi_List *list) {
    Jsi_ListClear(list);
    if (list->mutex)
        Jsi_MutexDelete(NULL, list->mutex);
    if (list->attr.freeProc)
        (*list->attr.freeProc)(list, NULL);
    else
        free(list);
}

Jsi_Bool Jsi_ListTypeMatches(Jsi_List *list, Jsi_ListEntry *item)
{
    if (list != item->list)
        return (list->attr.freeProc == item->list->attr.freeProc && list->attr.valueSpace == item->list->attr.valueSpace);
    return 0;
}

Jsi_Bool Jsi_ListIsIn(Jsi_List *list, Jsi_ListEntry *item)
{
    if (list->head == item)
        return 1;
    if (list == item->list && (item->next || item->prev))   
        return 1;
    return 0;
}

Jsi_ListEntry* Jsi_ListInsert(Jsi_List *list, Jsi_ListEntry *item, Jsi_ListEntry *at)
{
    Assert(item && item->next==NULL && item->prev==NULL && list);
    if (list->attr.lockProc && (*list->attr.lockProc)(list, 1) != JSI_OK)
        return NULL;
    if (!item->list)
         item->list = list;
    else if (list != item->list) {
        Assert(list->attr.freeProc == item->list->attr.freeProc);
        item->list = list;
    }
    if (!list->head) {
        list->head = list->tail = item;
    } else if (item == list->head) {
        assert(0);
    } else if (at == NULL) {
        item->prev = list->tail;
        list->tail->next = item;
        list->tail = item;
    } else if (at == list->head) {
        item->next = list->head;
        list->head->prev = item;
        list->head = item;
    } else {
        item->next = at;
        item->prev = at->prev;
        at->prev->next = item;
        at->prev = item;
    }
    list->numEntries++;
    item->list = list;
    if (list->attr.lockProc)
        (*list->attr.lockProc)(list, 0);
    return item;
}
 
Jsi_ListEntry* Jsi_ListRemove(Jsi_List *list, Jsi_ListEntry *item)
{
    Assert(item && list->head && list->tail && item->list && item->list);
    if (list->attr.lockProc && (*list->attr.lockProc)(list, 1) != JSI_OK)
        return NULL;
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
    if (list->attr.lockProc)
        (*list->attr.lockProc)(list, 0);
    return item;
}


Jsi_ListEntry *Jsi_ListEntryNew(Jsi_List* list) {
    int extra = (list?list->attr.valueSpace:0);
    Jsi_ListEntry *l = (Jsi_ListEntry*)Jsi_Calloc(1, sizeof(Jsi_ListEntry)+extra);
    SIGINIT(l, LISTENTRY);
    l->list = list;
    if (extra)
        l->value = (void*)(((char*)l)+sizeof(Jsi_ListEntry));
    return l;
}

void Jsi_ListEntryDelete(Jsi_ListEntry *l) {
    Assert(l->next==NULL && l->prev==NULL);
    if (l->list && l->list->attr.freeProc)
        (*l->list->attr.freeProc)(l->list, l);
    else
        free(l);
}

void Jsi_ListClear(Jsi_List *list) {
    Jsi_ListEntry *l;
    if (list->attr.lockProc && (*list->attr.lockProc)(list, 1) != JSI_OK)
        return;
    while (list->head) {
        l = list->head;
        list->head = list->head->next;
        l->next = l->prev = NULL;
        Jsi_ListEntryDelete(l);
    }
    list->numEntries = 0;
    if (list->attr.lockProc)
        (*list->attr.lockProc)(list, 0);
}
 
int Jsi_ListLock(Jsi_List *list, int lock) {
    if (!list->mutex)
        return JSI_OK;
    if (lock)
        return Jsi_MutexLock(NULL, list->mutex);
    Jsi_MutexUnlock(NULL, list->mutex);
    return JSI_OK;
}

void* Jsi_ListGetAttr(Jsi_List *list, Jsi_ListAttr **attr)
{
    if (attr)
        *attr = &list->attr;
    return list->attr.data;
}

int Jsi_ListSize(Jsi_List *list) { return list->numEntries;}

void* Jsi_ListEntryGetValue(Jsi_ListEntry *l) { return l?l->value:NULL; }
void Jsi_ListEntrySetValue(Jsi_ListEntry *l, void *value) { l->value = value;}
Jsi_ListEntry* Jsi_ListEntryPrev(Jsi_ListEntry *l) { return l->prev; }
Jsi_ListEntry* Jsi_ListEntryNext(Jsi_ListEntry *l) { return l->next; }

Jsi_ListEntry* Jsi_ListGetFront(Jsi_List *list) {return list->head;}
Jsi_ListEntry* Jsi_ListGetBack(Jsi_List *list) { return list->tail;}
Jsi_ListEntry* Jsi_ListPopFront(Jsi_List *list) { return Jsi_ListRemove(list, list->head);}
Jsi_ListEntry* Jsi_ListPopBack(Jsi_List *list) {return Jsi_ListRemove(list, list->tail);}
Jsi_ListEntry* Jsi_ListPushFront(Jsi_List *list, Jsi_ListEntry *item) {return Jsi_ListInsert(list, item, list->head);}
Jsi_ListEntry* Jsi_ListPushBack(Jsi_List *list, Jsi_ListEntry *item) {return Jsi_ListInsert(list, item, NULL);}
Jsi_ListEntry* Jsi_ListPushFrontNew(Jsi_List *list, void *value) {
    Jsi_ListEntry* l = Jsi_ListEntryNew(list);
    l->value = value;
    return Jsi_ListInsert(list, l, list->head);
}
Jsi_ListEntry* Jsi_ListPushBackNew(Jsi_List *list, void *value) {
    Jsi_ListEntry* l = Jsi_ListEntryNew(list);
    l->value = value;
    return Jsi_ListInsert(list, l, NULL);
}


#ifndef JSI_OMIT_THREADS

#ifdef __WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct Jsi_Mutex_ {
    int flags;
    int lockTimeout;
    int threadErrCnt;
#ifdef __WIN32
    CRITICAL_SECTION mtx
#else
    pthread_mutex_t mtx;
#endif
} Jsi_Mutex;

#ifdef __WIN32
#include <windows.h>

static int MutexLock(Jsi_Interp *interp, Jsi_Mutex* mtx) {
    int timeout = mtx->lockTimeout;
    if (interp && timeout<0)
        timeout = interp->lockTimeout;
    if (timeout<=0)
        EnterCriticalSection(&mtx->mtx);
    else {
        uint cnt = timeout;
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
static int MutexLock(Jsi_Interp *interp, Jsi_Mutex *mtx) {
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

int Jsi_MutexLock(Jsi_Interp *interp, Jsi_Mutex *mtx) { if (interp) interp->lockRefCnt++; return MutexLock(interp, mtx);}
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
    return (void*)GetCurrentThreadId();
#else
    return (void*)pthread_self();
#endif
}

#else /* ! JSI_OMIT_THREADS */
int Jsi_MutexLock(Jsi_Interp *interp, Jsi_Mutex *mtx) { return JSI_OK; }
void Jsi_MutexUnlock(Jsi_Interp *interp, Jsi_Mutex *mtx) { }
Jsi_Mutex* Jsi_MutexNew(Jsi_Interp *interp) { return NULL; }
void Jsi_MutexDelete(Jsi_Interp *interp, Jsi_Mutex *mtx) { }
void* Jsi_CurrentThread(void) { return NULL; }
void* Jsi_InterpThread(Jsi_Interp *interp) { return NULL; }
#endif

Jsi_Number Jsi_Version(void) {
    Jsi_Number d = JSI_VERSION;
    return d;
}
