#ifndef __JSIPSTATE_H__
#define __JSIPSTATE_H__

#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif


Jsi_ScopeStrs *jsi_ScopeStrsNew();
void jsi_ScopeStrsPush(Jsi_Interp *interp, Jsi_ScopeStrs *ss, const char *string, int argType);
void jsi_ScopeStrsFree(Jsi_Interp *interp, Jsi_ScopeStrs *ss);
const char *jsi_ScopeStrsGet(Jsi_ScopeStrs *ss, int i);

/* what lexical scope means:
 * --------------
 * var a;                       // this is first level of scope
 * function foo() {             // parsing function make lexical scope to push
 *     var b;                   // this is the second level
 *     var c = function() {     // push again
 *         var d;               // third level
 *     }                        // end of an function, pop scope
 * }                            // return to first scope
 * --------------
 */

void jsi_PstatePush(jsi_Pstate *ps);
void jsi_PstatePop(jsi_Pstate *ps);
void jsi_PstateAddVar(jsi_Pstate *ps, const char *str);
Jsi_ScopeStrs *jsi_ScopeGetVarlist(jsi_Pstate *ps);

void jsi_PstateFree(jsi_Pstate *ps);
jsi_Pstate *jsi_PstateNew(Jsi_Interp *interp);
void jsi_PstateClear(jsi_Pstate *ps);
const char * jsi_PstateGetFilename(jsi_Pstate *ps);
int jsi_PstateSetFile(jsi_Pstate *ps, Jsi_Channel fp, int skipbang);
int jsi_PstateSetString(jsi_Pstate *ps, const char *str);

extern int yyparse(jsi_Pstate *ps);




#endif /* __JSIPSTATE_H__ */
