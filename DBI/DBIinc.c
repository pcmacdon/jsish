#include <ctype.h>

// Must define this IdGet function when using "idconf".
static DBI_id* DBI_IdGet(DBI *cmdPtr, int id, bool create) {
    Jsi_HashEntry *hPtr;
    Jsi_Interp *interp = interp;
    DBI_id* pss;
    bool isNew;
    if (id<0)
        id = ++cmdPtr->_idx;
    if (create)
        hPtr = Jsi_HashEntryNew(cmdPtr->_resHash, (void*)(long)id, &isNew);
    else
        hPtr = Jsi_HashEntryFind(cmdPtr->_resHash, (void*)(long)id);
    if (!hPtr) {
        Jsi_LogError("failed to %s id", (create?"create":"get"));
        return NULL;
    }
    if (create == 0 || isNew == 0) {
        pss = (DBI_id*)Jsi_HashValueGet(hPtr);
    } else {
        pss = (DBI_id*)Jsi_Calloc(1, sizeof(*pss));
        pss->_hPtr = hPtr;
        Jsi_HashValueSet(hPtr, pss);
        pss->_cmdPtr = cmdPtr;
        pss->id = id;
    }
    return pss;
}

static Jsi_RC DBI_IdErase(Jsi_Interp *interp, Jsi_HashEntry* hPtr, void *ptr) {
    DBI_id *pss = (DBI_id *)ptr;
    if (pss) {
        //LogDebug("FREEPSS: %d\n", pss->id);
        pss->_hPtr = NULL;
        if (pss->_res)
            dbi_result_free(pss->_res);
        if (pss->_fieldTypes)
            Jsi_Free(pss->_fieldTypes);
        if (pss->_fieldNames)
            Jsi_Free(pss->_fieldNames);
        if (pss->sql)
            Jsi_DecrRefCount(interp, pss->sql);
        Jsi_Free(pss);
        //sockDeletePss(pss);
    }
    return JSI_OK;
}

static Jsi_RC DBI_AddArgs(DBI *cmdPtr, Jsi_DString *dStr, const char *query, int qlen, Jsi_Value *args) {
    Jsi_Interp *interp = cmdPtr->_interp;
    int n, idx, len, argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *val;
    char *str, *cp, *qp = (char*)query;
    
    while ((cp = Jsi_Strchr(qp, '%'))) {
        if (cp[1] == '%') {
            Jsi_DSAppendLen(dStr, qp, cp-qp+1);
            qp = cp+2;
            continue;
        }
        Jsi_DSAppendLen(dStr, qp, cp-qp);
        n = 0;
        while (isdigit(cp[n])) n++;
        idx = atoi(cp+1);
        qp = cp + n + 2;
        if (idx <= 0 || idx > argc || !(val = Jsi_ValueArrayIndex(interp, args, idx-1)))
            return Jsi_LogError("failure in DBI_AddArgs");

        if (Jsi_ValueIsNumber(interp, val)) {
            Jsi_Number num;
            Jsi_ValueGetNumber(interp, val, &num);
            Jsi_DSPrintf(dStr, "%g", num);
        } else if ((str = (char*)Jsi_ValueToString(interp, val, &len)) && len) {
            uchar quoted[len*2+100], *qp = quoted;
            len = dbi_conn_quote_binary_copy(cmdPtr->_db, (const uchar*)str, len, &qp);
            if (len <= 0)
                return Jsi_LogError("quote failure in DBI_AddArgs");
            Jsi_DSAppendLen(dStr, (char*)qp, len);
            free(qp);
        } else
            Jsi_DSAppend(dStr, "null", NULL);
    }
    Jsi_DSAppend(dStr, qp, NULL);
    return JSI_OK;
}

static Jsi_RC DBI_IdValue(Jsi_Interp *interp, DBI_id *pss, Jsi_Value **ret, int idx)
{
    if (idx < 0 || idx >= pss->fieldCount || !pss->onrecord)
        return Jsi_LogError("index out of range");
    int fidx = pss->_fieldTypes[idx];
    char *str = NULL;

    if (dbi_result_field_is_null_idx(pss->_res, idx+1))
        Jsi_ValueMakeNull(interp, ret);
    else
        switch (fidx)
        {
            case DBI_TYPE_DATETIME:
                Jsi_ValueMakeNumber(interp, ret, dbi_result_get_datetime_idx(pss->_res, idx+1));
                break;
            case DBI_TYPE_INTEGER:
                Jsi_ValueMakeNumber(interp, ret, dbi_result_get_longlong_idx(pss->_res, idx+1));
                break;
            case DBI_TYPE_DECIMAL:
                Jsi_ValueMakeNumber(interp, ret, dbi_result_get_double_idx( pss->_res, idx+1));
                break;
            case DBI_TYPE_BINARY:
                str = (char*)dbi_result_get_binary_copy_idx( pss->_res, idx+1 );;
            case DBI_TYPE_STRING:
                if (!str) 
                    str = dbi_result_get_string_copy_idx(pss->_res, idx+1);
                Jsi_ValueMakeBlob(interp, ret, (unsigned char *)str,
                    dbi_result_get_field_length_idx(pss->_res, idx+1));
        }

    return JSI_OK;
}

static const char jdbi_needCsvQuote[] = {
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 0, 1, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 1, 
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,   
};

/*
** Output a single term of CSV.  Actually, p->separator is used for
** the separator, which may or may not be a comma.  p->nullvalue is
** the null value.  Strings are quoted if necessary.
*/
static void mdbOutputCsv(DBI_query *p, const char *z, Jsi_DString *dStr, int bSep)
{
    if( z==0 ) {
        Jsi_DSAppend(dStr,  p->nullvalue?p->nullvalue:"", NULL);
    } else {
        int i;
        int nSep = Jsi_Strlen(p->separator);
        for(i=0; z[i]; i++) {
            if(jdbi_needCsvQuote[((unsigned char*)z)[i]] || 
                (z[i]==p->separator[0] && (nSep==1 || memcmp(z, p->separator, nSep)==0)) ) {
                i = 0;
                break;
            }
        }
        if( i==0 ) {
            Jsi_DSAppend(dStr, "\"", NULL);
            for(i=0; z[i]; i++) {
                if( z[i]=='"' ) Jsi_DSAppend(dStr, "\"", NULL);
                Jsi_DSAppendLen(dStr, z+i, 1);
            }
            Jsi_DSAppend(dStr, "\"", NULL);
        } else {
            Jsi_DSAppend(dStr, z, NULL);
        }
    }
    if( bSep ) {
        Jsi_DSAppend(dStr, p->separator, NULL);
    }
}

static void mdbOutputHtmlString(DBI_query *p, const char *z, Jsi_DString *dStr)
{
    while( *z ) {
        switch (*z) {
        case '<':
            Jsi_DSAppend(dStr, "&lt;", NULL);
            break;
        case '>':
            Jsi_DSAppend(dStr, "&gt;", NULL);
            break;
        case '&':
            Jsi_DSAppend(dStr, "&amp;", NULL);
            break;
        case '\"':
            Jsi_DSAppend(dStr, "&quot;", NULL);
            break;
        case '\'':
            Jsi_DSAppend(dStr, "&#39;", NULL);
            break;
        default:
            Jsi_DSAppendLen(dStr, z, 1);
            break;
        }
        z++;
    }
}

static void mdbOutputQuotedString(Jsi_DString *dStr, const char *z) {
    int i;
    int nSingle = 0;
    for(i=0; z[i]; i++) {
        if( z[i]=='\'' ) nSingle++;
    }
    if( nSingle==0 ) {
        Jsi_DSAppend(dStr,"'", z, "'", NULL);
    } else {
        Jsi_DSAppend(dStr,"'", NULL);
        while( *z ) {
            for(i=0; z[i] && z[i]!='\''; i++) {}
            if( i==0 ) {
                Jsi_DSAppend(dStr,"''", NULL);
                z++;
            } else if( z[i]=='\'' ) {
                Jsi_DSAppendLen(dStr,z, i);
                Jsi_DSAppend(dStr,"''", NULL);
                z += i+1;
            } else {
                Jsi_DSAppend(dStr, z, NULL);
                break;
            }
        }
        Jsi_DSAppend(dStr,"'", NULL);
    }
}



Jsi_RC mdbEvalStep(DBI_id *pss) {
    int ind = pss->curInd++;
    if (ind>=pss->resultCount) return JSI_BREAK;
    if (ind)
        pss->onrecord = dbi_result_seek_row(pss->_res, ind+1);
    return JSI_OK;
}

static void mdbEvalRowInfo(DBI_id *pss, int *pnCol, char ***papColName, short **papColType) {
    *papColName =pss->_fieldNames;
    *papColType = pss->_fieldTypes;
    *pnCol = pss->fieldCount;
}


static void mdbEvalSetColumnJSON(DBI_id *pss, int idx, Jsi_DString *dStr, DBI_query* optPtr) {
    Jsi_Interp *interp = pss->_cmdPtr->_interp;
    char nbuf[200];
    const char *str = NULL;
    int len, fidx = pss->_fieldTypes[idx];
    Jsi_Number num;
    switch(fidx) {
        case DBI_TYPE_BINARY:
                str = (char*)dbi_result_get_binary_idx( pss->_res, idx+1 );;
        case DBI_TYPE_STRING:
            if (!str) 
                    str = dbi_result_get_string_idx( pss->_res, idx+1 );
            len = dbi_result_get_field_length_idx(pss->_res, idx+1);
            if( !str ) {
                const char *nv = optPtr->nullvalue;
                Jsi_DSAppend(dStr, nv?nv:"null", NULL);
                return;
            }
            Jsi_JSONQuote(interp, str, len, dStr);
            break;
        case DBI_TYPE_DATETIME:
            num = dbi_result_get_datetime_idx( pss->_res, idx+1);
            Jsi_NumberToString(interp, num, nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            break;
        case DBI_TYPE_INTEGER:
            snprintf(nbuf, sizeof(nbuf), "%lld", dbi_result_get_longlong_idx(pss->_res, idx+1 ));
            Jsi_DSAppend(dStr, nbuf, NULL);
            break;
        case DBI_TYPE_DECIMAL:
            Jsi_NumberToString(interp, dbi_result_get_double_idx( pss->_res, idx+1 ), nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            break;
        default:
            Jsi_LogWarn("unknown type: %d", fidx);
    
    }
}

static void mdbEvalSetColumn(DBI_id *pss, int idx, Jsi_DString *dStr, DBI_query* optPtr) {
    Jsi_Interp *interp = pss->_cmdPtr->_interp;
    char nbuf[200];
    const char *str = NULL;
    int len, fidx = pss->_fieldTypes[idx];
    Jsi_Number num;
    switch(fidx) {
        case DBI_TYPE_BINARY:
                str = (char*)dbi_result_get_binary_idx( pss->_res, idx+1 );;
        case DBI_TYPE_STRING:
            if (!str) 
                    str = dbi_result_get_string_idx( pss->_res, idx+1 );
            len = dbi_result_get_field_length_idx(pss->_res, idx+1);
            if( !str ) {
                const char *nv = optPtr->nullvalue;
                Jsi_DSAppend(dStr, nv?nv:"null", NULL);
                return;
            }
            Jsi_DSAppendLen(dStr, str, len);
            break;
        case DBI_TYPE_DATETIME:
            num = dbi_result_get_datetime_idx( pss->_res, idx+1);
            Jsi_NumberToString(interp, num, nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            break;
        case DBI_TYPE_INTEGER:
            snprintf(nbuf, sizeof(nbuf), "%lld", dbi_result_get_longlong_idx(pss->_res, idx+1 ));
            Jsi_DSAppend(dStr, nbuf, NULL);
            break;
        case DBI_TYPE_DECIMAL:
            Jsi_NumberToString(interp, dbi_result_get_double_idx( pss->_res, idx+1 ), nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            break;
        default:
            Jsi_LogWarn("unknown type: %d", fidx);
    
    }
}

#if 1
static Jsi_RC mdbEvalCallCmd(DBI_id *pss, Jsi_Interp *interp, Jsi_RC result, DBI_query* optPtr, Jsi_Value *callback)
{
    int cnt = 0;
    Jsi_RC rc = result;
    Jsi_Value *varg1;
    Jsi_Obj *argso;
    char **apColName = NULL;
    short *apColType = NULL;
    //if (p->jdb->debug & mdbTMODE_EVAL)
    //    JSI_DBQUERY_PRINTF( "DEBUG: eval\n");

    while( (rc==JSI_OK) && JSI_OK==(rc = mdbEvalStep(pss)) ) {
        int i;
        int nCol;

        cnt++;
        mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
        if (nCol<=0)
            continue;
        if (Jsi_ValueIsNull(interp, callback))
            continue;
        /* Single object containing sql result members. */
        varg1 = Jsi_ValueMakeObject(interp, NULL, argso = Jsi_ObjNew(interp));
        for(i=0; i<nCol; i++) {
            Jsi_Value *vcur = Jsi_ValueNew(interp);
            rc = DBI_IdValue(interp, pss, &vcur, i);
            //Jsi_Value *nnv = dbi EvalSetColumnValue(p, i, NULL);
            Jsi_ObjInsert(interp, argso, apColName[i], vcur, 0);
        }
        Jsi_IncrRefCount(interp, varg1);
        bool rb = Jsi_FunctionInvokeBool(interp, callback, varg1);
        if (Jsi_InterpGone(interp))
            return JSI_ERROR;
        Jsi_DecrRefCount(interp, varg1);
        if (rb)
            break;
    }
    //mdbEvalFinalize(p);

    if( rc==JSI_OK || rc==JSI_BREAK ) {
        //Jsi_ResetResult(interp);
        rc = JSI_OK;
    }
    return rc;
}

#endif

static Jsi_RC DbiQueryCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
     Jsi_Value **ret, Jsi_Func *funcPtr, DBI_id *pss, DBI_query* qopts, bool modeor)
{
    Jsi_RC rc = JSI_OK;
    DBI* cmdPtr = pss->_cmdPtr;
    Jsi_DString eStr = {};
    JSI_DSTRING_VAR(dStr, 8192);
    int cnt = 0;
    char **apColName = NULL;
    short *apColType = NULL;
    DBI_query opts, *optPtr = &opts, *copts = &cmdPtr->queryOpts;
    opts = *qopts;
    if (qopts != copts) {
#define JDBI_OOPTS(n) if (!opts.n) opts.n = copts->n
        JDBI_OOPTS(callback);
        JDBI_OOPTS(widths);
        JDBI_OOPTS(headers);
        JDBI_OOPTS(limit);
        JDBI_OOPTS(nullvalue);
        JDBI_OOPTS(separator);
        if (!modeor)
            JDBI_OOPTS(mode);
    }
        
    //optPtr->callback = NULL;
    //optPtr->width = NULL;
    Jsi_Value *callback = optPtr->callback, *width = optPtr->widths;
            
#if 0
    if (arg) {
        if (Jsi_ValueIsNull(interp,arg)) {
        } else if (Jsi_ValueIsFunction(interp,arg)) {
            callback = optPtr->callback = arg;
        } else if (Jsi_ValueIsObjType(interp, arg, JSI_OT_OBJECT)) {
            isopts = 1;
        } else 
            return Jsi_LogError("argument must be null, a function, or options");
    }

    if (isopts) {
        if (Jsi_OptionsProcess(interp, QueryFmtOptions, &opts, arg, 0) < 0)
            return JSI_ERROR;
        callback = (optPtr->callback ? optPtr->callback : jdb->queryOpts.callback);
        width = (optPtr->width ? optPtr->width : jdb->queryOpts.width);
    }
/*    if (jdb->queryOpts.CData) {
        char *cdata = (char*)jdb->queryOpts.CData;
        MySqlObjMultipleBind* copts = Jsi_CarrayLookup(interp, cdata);
        if (!copts) 
            return Jsi_LogError("unknown CData option: %s", jdb->queryOpts.CData);
        int n = MySqlObjQuery(jdb, copts->opts, copts->data, copts->numData, zSql, copts->flags);
        Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)n);
        return JSI_OK;
    } */

    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    sEval.nocache = optPtr->nocache;
    if (mdbEvalInit(interp, &sEval, jdb, zSql, &sStr, 0, 0) != JSI_OK) {
        rc = JSI_ERROR;
        goto bail;
    }
    sEval.namedParams = (optPtr->noNamedParams==0 && !optPtr->values);
    sEval.ret = *ret;
    oEopt = jdb->optPtr;
    jdb->optPtr = &opts;
    
    if (sEval.namedParams) {
        rc = mdbEvalPrep(&sEval);
        if (rc == JSI_ERROR)
            goto bail;
        if (rc == JSI_BREAK) {
            rc = JSI_OK;
            goto bail;
        }
    }
#endif
    if (!optPtr->separator) {
        switch (optPtr->mode) {
            case list: optPtr->separator = "|"; break;
            case column: optPtr->separator = " "; break;
            case tabs: optPtr->separator = "\t"; break;
            default: optPtr->separator = ",";
        }
    }

    if (callback) {
        if (optPtr->mode != rows)
            rc = Jsi_LogError("'mode' must be 'rows' with 'callback'");
        else 
            rc = mdbEvalCallCmd(pss, interp, JSI_OK, optPtr, callback);
        goto bail;
    }
    switch (optPtr->mode) {
        case cursor:
            goto bail;
        case none:
            goto bail;
            break;
        case rows:
        {
            Jsi_Value *vcur, *vrow;
            int cnt = 0;
            Jsi_Obj *oall, *ocur;
            Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
    
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                ocur = Jsi_ObjNewType(interp, JSI_OT_OBJECT);
                vrow = Jsi_ValueMakeObject(interp, NULL, ocur);
                for(i=0; i<nCol && rc==JSI_OK; i++) {
                    vcur = Jsi_ValueNew(interp);
                    rc = DBI_IdValue(interp, pss, &vcur, i);
                    //vcur = dbiEvalSetColumnValue(pss, i, NULL);
                    Jsi_ObjInsert(interp, ocur, apColName[i], vcur, 0);
                }
                Jsi_ObjArrayAdd(interp, oall, vrow);
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            //mdbEvalFinalize(&sEval);
            if (rc != JSI_ERROR)
                rc = JSI_OK;
            goto bail;
            break;
        }
        case json:
            if (optPtr->headers) {
                Jsi_DSAppend(dStr, "[ ", NULL);
                while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                    int i;
                    int nCol;
                    mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                    if (cnt == 0) {
                        Jsi_DSAppend(dStr, "[", NULL);
                        for(i=0; i<nCol; i++) {
                            if (i)
                                Jsi_DSAppend(dStr, ", ", NULL);
                            Jsi_JSONQuote(interp, apColName[i], -1, dStr);
                        }
                        Jsi_DSAppend(dStr, "]", NULL);
                        cnt++;
                    }
                    if (cnt)
                        Jsi_DSAppend(dStr, ", ", NULL);
                    Jsi_DSAppend(dStr, "[", NULL);
                    for(i=0; i<nCol; i++) {
                        if (i)
                            Jsi_DSAppend(dStr, ", ", NULL);
                        mdbEvalSetColumnJSON(pss, i, dStr, optPtr);
                    }
                    Jsi_DSAppend(dStr, "]", NULL);
                    cnt++;
                    if (optPtr->limit && cnt>optPtr->limit) break;
                }
                Jsi_DSAppend(dStr, " ]", NULL);
                
            } else {
                Jsi_DSAppend(dStr, "[ ", NULL);
                while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                    int i;
                    int nCol;
                    mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                    if (cnt)
                        Jsi_DSAppend(dStr, ", ", NULL);
                    Jsi_DSAppend(dStr, "{", NULL);
                    for(i=0; i<nCol; i++) {
                        if (i)
                            Jsi_DSAppend(dStr, ", ", NULL);
                        Jsi_JSONQuote(interp, apColName[i], -1, dStr);
                        Jsi_DSAppend(dStr, ":", NULL);
                        mdbEvalSetColumnJSON(pss, i, dStr, optPtr);
                    }
                    Jsi_DSAppend(dStr, "}", NULL);
                    cnt++;
                    if (optPtr->limit && cnt>=optPtr->limit) break;
                }
                Jsi_DSAppend(dStr, " ]", NULL);
            }
            break;
            
        case json2: {
                while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                    int i;
                    int nCol;
                    mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                    if (cnt == 0 && 1) {
                        Jsi_DSAppend(dStr, "{ \"names\": [ ", NULL);
                        for(i=0; i<nCol; i++) {
                            if (i)
                                Jsi_DSAppend(dStr, ", ", NULL);
                            Jsi_JSONQuote(interp, apColName[i], -1, dStr);
                        }
                        Jsi_DSAppend(dStr, " ], \"values\": [ ", NULL);
                    }
                    if (cnt)
                        Jsi_DSAppend(dStr, ", ", NULL);
                    Jsi_DSAppend(dStr, "[", NULL);
                    for(i=0; i<nCol; i++) {
                        if (i)
                            Jsi_DSAppend(dStr, ", ", NULL);
                        mdbEvalSetColumnJSON(pss, i, dStr, optPtr);
                    }
                    Jsi_DSAppend(dStr, " ]", NULL);
                    cnt++;
                    if (optPtr->limit && cnt>=optPtr->limit) break;
                }
                if (cnt)
                    Jsi_DSAppend(dStr, " ] } ", NULL);
            }
            break;
            
        case list:
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                if (cnt == 0 && optPtr->headers) {
                    for(i=0; i<nCol; i++) {
                        if (i)
                            Jsi_DSAppend(dStr, optPtr->separator, NULL);
                        Jsi_DSAppend(dStr, apColName[i], NULL);
                    }
                }
    
                if (cnt || optPtr->headers)
                    Jsi_DSAppend(dStr, "\n", NULL);
                for(i=0; i<nCol; i++) {
                    if (i)
                        Jsi_DSAppend(dStr, optPtr->separator, NULL);
                    mdbEvalSetColumn(pss, i, dStr, optPtr);
                }
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            break;
            
        case column: {
            int *wids = NULL;
            Jsi_DString vStr = {};
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                int i, w;
                int nCol;
                
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                if (cnt == 0 && nCol>0) {
                    Jsi_DString sStr;
                    wids = (int*)Jsi_Calloc(nCol, sizeof(int));
                    Jsi_DSInit(&sStr);
                    for(i=0; i<nCol; i++) {
                        int j = Jsi_Strlen(apColName[i]);
                        wids[i] = (j<10?10:j);
                        if (width) {
                            Jsi_Value *wv = Jsi_ValueArrayIndex(interp, width, i);
                            if (wv) {
                                Jsi_Number dv;
                                Jsi_ValueGetNumber(interp, wv, &dv);
                                if (dv>0)
                                    wids[i] = (int)dv;
                            }
                        }
                        w = (j<wids[i] ? j : wids[i]);
                        Jsi_DSAppendLen(dStr, apColName[i], w);
                        w = (j<wids[i] ? wids[i]-j+1 : 0);
                        while (w-- > 0)
                            Jsi_DSAppend(dStr, " ", NULL);
                    }
                    for(i=0; i<nCol && optPtr->headers; i++) {
                        w = wids[i];
                        w -= Jsi_Strlen(apColName[i]);
                        if (i) {
                            Jsi_DSAppend(dStr, optPtr->separator, NULL);
                            Jsi_DSAppend(&sStr, optPtr->separator, NULL);
                        }
                        w = wids[i];
                        while (w-- > 0)
                            Jsi_DSAppend(&sStr, "-", NULL);
                    }
                    if (optPtr->headers)
                        Jsi_DSAppend(dStr, "\n", Jsi_DSValue(&sStr), "\n", NULL);
                    Jsi_DSFree(&sStr);
                }
    
                if (cnt)
                    Jsi_DSAppend(dStr, "\n", NULL);
                for(i=0; i<nCol; i++) {
                    if (i)
                        Jsi_DSAppend(dStr, optPtr->separator, NULL);
                    Jsi_DSSetLength(&vStr, 0);
                    mdbEvalSetColumn(pss, i, &vStr, optPtr);
                    int nl = Jsi_DSLength(&vStr);
                    if (nl > wids[i]) {
                        Jsi_DSSetLength(&vStr, wids[i]);
                        w = 0;
                    } else {
                        w = wids[i]-nl;
                    }
                    Jsi_DSAppend(dStr, Jsi_DSValue(&vStr), NULL);
                    while (w-- > 0)
                        Jsi_DSAppend(dStr, " ", NULL);
                }
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            Jsi_DSFree(&vStr);
            if (wids)
                Jsi_Free(wids);
            break;
        }
        
        case insert: {
            Jsi_DString vStr = {};    
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                int i;
                int nCol;
                const char *tbl = (optPtr->table ? optPtr->table : "table");
                if (cnt)
                    Jsi_DSAppend(dStr, "\n", NULL);
                Jsi_DSAppend(dStr, "INSERT INTO ", tbl, " VALUES(", NULL);
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                for(i=0; i<nCol; i++) {
                    Jsi_Number dv;
                    const char *azArg;
                    Jsi_DSSetLength(&vStr, 0);
                    mdbEvalSetColumn(pss, i, &vStr, optPtr);
                    
                    int ptype = pss->_fieldTypes[i];
                    
                    azArg = Jsi_DSValue(&vStr);
                    const char *zSep = i>0 ? ",": "";
                    if (azArg[i]==0 && ptype != JSI_OPTION_STRING) {
                      Jsi_DSAppend(dStr, zSep, "NULL", NULL);
                    } else if( ptype == DBI_TYPE_STRING) {
                      if( zSep[0] ) Jsi_DSAppend(dStr,zSep, NULL);
                      mdbOutputQuotedString(dStr, azArg);
                    } else if ( ptype ==DBI_TYPE_DECIMAL) {
                      Jsi_DSAppend(dStr, zSep, azArg, NULL);
                    } else if( Jsi_GetDouble(interp, azArg, &dv) == JSI_OK ) {
                      Jsi_DSAppend(dStr, zSep, azArg, NULL);
                    } else {
                      if( zSep[0] ) Jsi_DSAppend(dStr,zSep, NULL);
                      mdbOutputQuotedString(dStr, azArg);
                    }
                }
                Jsi_DSAppend(dStr, ");", NULL);
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            Jsi_DSFree(&vStr);
        }
    
        case tabs:
        case csv: {
            Jsi_DString vStr = {};  
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                if (cnt == 0 && optPtr->headers) {
                    for(i=0; i<nCol; i++) {
                        if (i)
                            Jsi_DSAppend(dStr, optPtr->separator, NULL);
                        Jsi_DSAppend(dStr, apColName[i], NULL);
                    }
                }
    
                if (cnt || optPtr->headers)
                    Jsi_DSAppend(dStr, "\n", NULL);
                for(i=0; i<nCol; i++) {
                    if (i)
                        Jsi_DSAppend(dStr, optPtr->separator, NULL);
                    Jsi_DSSetLength(&vStr, 0);
                    mdbEvalSetColumn(pss, i, &vStr, optPtr);
                    if (optPtr->mode == csv)
                        mdbOutputCsv(optPtr, Jsi_DSValue(&vStr), dStr, 0);
                    else
                        Jsi_DSAppend(dStr, Jsi_DSValue(&vStr), NULL);
                }
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            Jsi_DSFree(&vStr);
            break;
        }
            
        case line: {
            int i, w = 5, ww;
            int nCol;
            Jsi_DString vStr = {};   
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                if (cnt == 0) {
                    for(i=0; i<nCol; i++) {
                        ww = Jsi_Strlen(apColName[i]);
                        if (ww>w)
                            w = ww;
                    }
                }
    
                for(i=0; i<nCol; i++) {
                    Jsi_DString eStr;
                    Jsi_DSInit(&eStr);
                    Jsi_DSSetLength(&vStr, 0);
                    mdbEvalSetColumn(pss, i, &vStr, optPtr);
                    Jsi_DSPrintf(&eStr, "%*s = %s", w, apColName[i], Jsi_DSValue(&vStr));
                    Jsi_DSAppend(dStr, (cnt?"\n":""), Jsi_DSValue(&eStr), NULL);
                    Jsi_DSFree(&eStr);
                }
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            Jsi_DSFree(&vStr);
            break;
        }

        case html: {
            Jsi_DString vStr = {};   
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                if (cnt == 0 && optPtr->headers) {
                    Jsi_DSAppend(dStr, "<TR>", NULL);
                    for(i=0; i<nCol; i++) {
                        Jsi_DSAppend(dStr, "<TH>", NULL);
                        mdbOutputHtmlString(optPtr, apColName[i], dStr);
                        Jsi_DSAppend(dStr, "</TH>", NULL);
                    }
                    Jsi_DSAppend(dStr, "</TR>", NULL);
                }
                if (cnt || optPtr->headers)
                    Jsi_DSAppend(dStr, "\n", NULL);
                Jsi_DSAppend(dStr, "<TR>", NULL);
                for(i=0; i<nCol; i++) {
                    Jsi_DSAppend(dStr, "<TD>", NULL);
                    Jsi_DSSetLength(&vStr, 0);
                    mdbEvalSetColumn(pss, i, &vStr, optPtr);
                    mdbOutputHtmlString(optPtr, Jsi_DSValue(&vStr), dStr);
                    Jsi_DSAppend(dStr, "</TD>", NULL);
                }
                Jsi_DSAppend(dStr, "</TR>", NULL);
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            Jsi_DSFree(&vStr);
            break;
        }

        case arrays:
        {
            Jsi_Value *vcur, *vrow;
            int cnt = 0;
            Jsi_Obj *oall, *ocur;
            Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
    
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                if (cnt == 0 && optPtr->headers) {
                    vrow = Jsi_ValueMakeArrayObject(interp, NULL, ocur = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
                    for(i=0; i<nCol; i++) {
                        vcur = Jsi_ValueNewStringDup(interp, apColName[i]);
                        Jsi_ObjArrayAdd(interp, ocur, vcur);
                    }
                    Jsi_ObjArrayAdd(interp, oall, vrow);
                }
                vrow = Jsi_ValueMakeArrayObject(interp, NULL, ocur = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
                for(i=0; i<nCol && rc==JSI_OK; i++) {
                    vcur = Jsi_ValueNew(interp);
                    rc = DBI_IdValue(interp, pss, &vcur, i);
                    //vcur = dbiEvalSetColumnValue(pss, i, NULL);
                    Jsi_ObjArrayAdd(interp, ocur, vcur);
                }
                Jsi_ObjArrayAdd(interp, oall, vrow);
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            //mdbEvalFinalize(&sEval);
            if (rc != JSI_ERROR)
                rc = JSI_OK;
            goto bail;
            break;
        }
        case array1d:
        {
            Jsi_Value *vcur;
            int cnt = 0;
            Jsi_Obj *oall;
            Jsi_ValueMakeArrayObject(interp, ret, oall = Jsi_ObjNewType(interp, JSI_OT_ARRAY));
    
            while( JSI_OK==(rc = mdbEvalStep(pss)) ) {
                int i;
                int nCol;
                mdbEvalRowInfo(pss, &nCol, &apColName, &apColType);
                if (cnt == 0 && optPtr->headers) {
                    for(i=0; i<nCol; i++) {
                        vcur = Jsi_ValueNewStringDup(interp, apColName[i]);
                        Jsi_ObjArrayAdd(interp, oall, vcur);
                    }
                }
                for(i=0; i<nCol && rc==JSI_OK; i++) {
                    vcur = Jsi_ValueNew(interp);
                    rc = DBI_IdValue(interp, pss, &vcur, i);
                    //vcur = dbiEvalSetColumnValue(pss, i, NULL);
                    Jsi_ObjArrayAdd(interp, oall, vcur);
                }
                cnt++;
                if (optPtr->limit && cnt>=optPtr->limit) break;
            }
            //mdbEvalFinalize(&sEval);
            if (rc != JSI_ERROR)
                rc = JSI_OK;
            goto bail;
            break;
        }
    }
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    Jsi_ValueMakeStringDup(interp, ret, Jsi_DSValue(dStr));
bail:
   /* //mdbEvalFinalize(&sEval);
    if (isopts) {
        Jsi_OptionsFree(interp, QueryFmtOptions, &opts, 0);
    }*/
    Jsi_DSFree(dStr);
    Jsi_DSFree(&eStr);
    //jdb->optPtr = oEopt;

    return rc;
}


static DBI_id* DbiQueryNew(Jsi_Interp *interp, DBI* cmdPtr, dbi_result res)
{
    DBI_id *pss = DBI_IdGet(cmdPtr, -1, true);
    if (!pss) {
        dbi_result_free(res);
        return NULL;
    }
    pss->onrecord = dbi_result_first_row(res);
    pss->resultCount = dbi_result_get_numrows(res);
    pss->fieldCount = dbi_result_get_numfields(res);
    pss->_fieldTypes = (short*)Jsi_Calloc( pss->fieldCount, sizeof(unsigned short) );
    pss->_fieldNames = (char**)Jsi_Calloc( pss->fieldCount, sizeof(char *) );
    for (int x=0; x < pss->fieldCount; x++) {
        const char *fieldname = dbi_result_get_field_name(res, x+1);
        pss->_fieldTypes[x] = dbi_result_get_field_type_idx(res, x+1);
        pss->_fieldNames[x] = (char*)Jsi_KeyAdd(interp, fieldname);
    }

    pss->_res = res;
    return pss;
}
