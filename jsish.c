#include "src/jsi.h"
#ifndef JSI_IN_AMALGAMATION
#define JSI_AMALGAMATION
#define JSI__ALL 1
struct jsi_Pstate;
#include "src/jsiStubs.h"
#include "regex/regex.h"
#include "regex/tre.h"
#include "regex/regcomp.c"
#include "regex/regerror.c"
#include "regex/regexec.c"
#include "regex/tre-mem.c"
#include "src/parser.h"
#include "src/jsiInt.h"
#if JSI__MINIZ
#include "miniz/miniz.c"
#endif //JSI__MINIZ
#if JSI__READLINE==1
#include "src/linenoise.c"
#endif //JSI__READLINE==1
#ifndef SQLITE_VERSION
#include "sqlite/src/sqlite3.c"
#endif //SQLITE_VERSION
#ifdef __cplusplus
#include "lws/src/src/lws.h"
#else // __cplusplus
#include "lws/src/lwsSingle.c"
#endif //__cplusplus
#include "src/jsiCode.c"
#include "src/jsiLexer.c"
#include "src/jsiFunc.c"
#include "src/jsiValue.c"
#include "src/jsiRegexp.c"
#include "src/jsiPstate.c"
#include "src/jsiInterp.c"
#include "src/jsiUtils.c"
#include "src/jsiProto.c"
#include "src/jsiFilesys.c"
#include "src/jsiChar.c"
#include "src/jsiString.c"
#include "src/jsiBool.c"
#include "src/jsiNumber.c"
#include "src/jsiArray.c"
#include "src/jsiLoad.c"
#include "src/jsiHash.c"
#include "src/jsiOptions.c"
#include "src/jsiStubs.c"
#include "src/jsiFormat.c"
#include "src/jsiJSON.c"
#include "src/jsiCmds.c"
#include "src/jsiFileCmds.c"
#include "src/jsiObj.c"
#include "src/jsiSignal.c"
#include "src/jsiTree.c"
#include "src/jsiCrypto.c"
#include "src/jsiDString.c"
#include "src/jsiMath.c"
#include "src/jsmn.c"
#include "src/jsiZvfs.c"
#include "src/jsiUtf8.c"
#include "src/jsiUserObj.c"
#include "src/jsiSocket.c"
#include "src/jsiSqlite.c"
#include "src/jsiWebSocket.c"
#include "src/jsiMySql.c"
#include "src/jsiCData.c"
#include "src/jsiVfs.c"
#ifndef JSI_LITE_ONLY
#include "src/parser.c"
#endif //JSI_LITE_ONLY
#include "win/compat.c"
#include "win/strptime.c"
#include "src/jsiEval.c"
#include "src/main.c"
#endif //JSI_IN_AMALGAMATION
