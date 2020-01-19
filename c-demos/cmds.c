/*
  Multi-command object: Cmd.add, Cmd.diff

  BUILDING:
    cc cmds.c -lz -lm -ldl -lpthread -I../src
*/
#include "jsi.c"

static Jsi_CmdProcDecl(AddCmd) {
    int i, n = Jsi_ValueGetLength(interp, args);
    Jsi_Number m, sum = 0;
    for (i=0; i<n; i++) {
        if (Jsi_ValueGetNumber(interp, Jsi_ValueArrayIndex(interp, args, i), &m) != JSI_OK)
            return JSI_ERROR;
        sum += m;
    }
    Jsi_ValueMakeNumber(interp, ret, sum);
    return JSI_OK;
}


static Jsi_CmdProcDecl(DiffCmd) {
    Jsi_Number m, n, diff;
    char buf[BUFSIZ];
    if (Jsi_ValueGetNumber(interp, Jsi_ValueArrayIndex(interp, args, 0), &m) != JSI_OK)
        return JSI_ERROR;
    if (Jsi_ValueGetNumber(interp, Jsi_ValueArrayIndex(interp, args, 1), &n) != JSI_OK)
        return JSI_ERROR;
    diff = m-n;
    // Non-strict JSON makes it easy to return complex objects in C.
    snprintf(buf, sizeof(buf), "{ diff:%g, values:[%g, %g] }", diff, m, n);
    Jsi_JSONParse(interp, buf, ret, 0);
    return JSI_OK;
}

static Jsi_CmdSpec myCmds[] = {
    { "add",        AddCmd,        0, -1, "...", .help="Return sum of all arguments", .retType=JSI_TT_NUMBER },
    { "diff",       DiffCmd,       2,  2, "v1:number, v2:number", .help="Return object with difference of values", .retType=JSI_TT_OBJECT },
    { NULL, 0,0,0,0,.help="My test commands" }
};

Jsi_InitProc Jsi_InitCmds;

Jsi_RC Jsi_InitCmds(Jsi_Interp *interp, int release) {
    Jsi_CommandCreateSpecs(interp, "Cmd", myCmds, NULL, 0);
    return JSI_OK;
}

int main(int argc, char *argv[])
{
    Jsi_InterpOpts opts = {.argc=argc, .argv=argv};
    Jsi_Interp *interp = Jsi_InterpNew(&opts);
    if (Jsi_InitCmds(interp, 0) != JSI_OK)
        exit(1);
    Jsi_EvalString(interp, "puts(Cmd.add(1,2,3)); puts(Cmd.diff(17,9));", 0);
    Jsi_Interactive(interp, 0);
    exit(0);
}
