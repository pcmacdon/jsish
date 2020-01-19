#include "jsi.c"

int main(int argc, char *argv[])
{
    Jsi_InterpOpts opts = {.argc=argc, .argv=argv};
    opts.no_interactive = 1;
    Jsi_Interp *interp = Jsi_InterpNew(&opts);
    if (!interp)
        return 1;

    if (!Jsi_Main(&opts))
        exit(1);
    
    // User code here ...
    
    Jsi_InterpDelete(interp);
    return(0);
}

