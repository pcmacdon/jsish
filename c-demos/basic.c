/*
  Basic use of Jsi API.

*/
#include "jsi.h"

static Jsi_CmdProcDecl(MyCmd);

int main(int argc, char *argv[])
{
    Jsi_Interp *interp = Jsi_InterpMain(argc, argv, NULL);

    /* Basic script. */
    Jsi_EvalString(interp, "for (var i=1; i<=3; i++)  puts('TEST: '+i);", 0);

    /* C-Extension */
    Jsi_CommandCreate(interp, "MyCmd", MyCmd, NULL);
    Jsi_EvalString(interp, "MyCmd(3,2,1);", 0);

#ifndef OMIT_SQLITE
    /* Database */
    Jsi_EvalString(interp,
                   "var db = new Sqlite('~/testjs.db');"
                   "db.eval('create table if not exists foo(a,b); insert into foo VALUES(1,2);');"
                   "puts(db.query('select * from foo;'));"
                   "puts(db.query('select * from foo;',{headers:true,mode:'column'}));", 0);
#endif
    return(0);
}

/* Callback for C extension. */
static Jsi_CmdProcDecl(MyCmd) {
    /* Just echo arguments */
    int i, n = Jsi_ValueGetLength(interp, args);
    printf("Called MyCmd() with:\n");
    for (i=0; i<n; i++)
        printf("  arguments[%d] = '%s'\n", i,
               Jsi_ValueArrayIndexToStr(interp, args, i, NULL));
    return JSI_OK;
}
