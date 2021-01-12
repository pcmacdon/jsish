/* JSI main program */
#ifndef JSI_AMALGAMATION
#include "jsi.h"
#endif
#include <string.h>
#include <stdlib.h>

#ifndef JSI_LITE_ONLY
int jsi_main(int argc, char **argv)
{
     // A replacement for shebang "#!/usr/bin/env".
    Jsi_DString sStr = {};
    FILE *fp = NULL;
    if (argc >= 3 && Jsi_Strchr(argv[1], ' ') && Jsi_Strstr(argv[1], "%s")) {
        Jsi_DString tStr = {};
        int i;
        Jsi_DSAppend(&tStr, argv[0], " ", NULL);
        Jsi_DSPrintf(&tStr, argv[1], argv[2]);
        for (i=3; i<argc; i++)
            Jsi_DSAppend(&tStr, " ", argv[i], NULL);
        Jsi_SplitStr(Jsi_DSValue(&tStr), &argc, &argv, NULL, &sStr);
        Jsi_DSFree(&tStr);
    }
    // Perform shebang extraction.
    else if (argc == 3 && !Jsi_Strcmp(argv[1], "-!") && (fp=fopen(argv[2],"r+"))) {
        char ibuf[1024], *icp = fgets(ibuf, sizeof(ibuf), fp);
        fclose(fp);
        if (icp && icp[0] == '#' && icp[1] == '!') {
            Jsi_DString tStr = {};
            icp += 2;
            if (strstr(icp, "%s")) {
                icp = Jsi_DSPrintf(&tStr, icp, argv[2]);
            } else {
                int len = strlen(icp);
                if (len>0 && icp[len-1]=='\n') icp[len-1] = 0;
                icp = Jsi_DSAppend(&tStr, icp, " ", argv[2], NULL);
            }
            int rc = system(icp);
            Jsi_DSFree(&tStr);
            return rc;
        }
    }
    Jsi_InterpOpts opts = {.argc=argc, .argv=argv};
#ifdef JSI__ZHASH
    opts.zhash = JSI__ZHASH;
#endif
    Jsi_Interp *interp = Jsi_Main(&opts);
    if (interp)
        Jsi_InterpDelete(interp);
    Jsi_DSFree(&sStr);
    exit(0);
}

#if JSI__MAIN
int main(int argc, char **argv) { return jsi_main(argc, argv); }
#endif
#endif
