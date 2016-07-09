/* JSI main program */
#include <stdlib.h>
#include <string.h>
#ifdef JSI_MEM_DEBUG
#include "jsiInt.h"
#else
#include "jsi.h"
#endif

static int deleted = 0, exitCode = 0;
static Jsi_Value *vf = NULL;

static int InterpDelete(Jsi_Interp *interp, void *ptr) {
    if (vf)
        Jsi_DecrRefCount(interp, vf);
    vf = NULL;
         exitCode = (int)ptr;
    deleted = 1;
    return JSI_OK;
}

int main(int argc, char **argv)
{
    int rc = JSI_OK, jsFound = 0;
    Jsi_Interp* interp = Jsi_InterpNew(NULL, argc, argv, 0);
    if (!interp)
        return(1);
    Jsi_InterpOnDelete(interp, &InterpDelete, NULL);

#ifndef NO_JAZ
    /* Mount zip at end of executable */
    Jsi_Value *v = Jsi_Executable(interp);
    const char *exeFile = Jsi_ValueString(interp, v, NULL);
    if (argc != 2 || Jsi_Strcmp(argv[1], "--nozvfs")) {
        rc = Jsi_ExecZip(interp, exeFile, JSI_ZVFS_DIR, &jsFound);
        if (rc >= 0) {
            if (!jsFound) {
#if (!defined(JSI_OMIT_FILESYS)) && (!defined(JSI_OMIT_ZVFS))
                fprintf(stderr, "warning: no main.jsi or jsiIndex.jsi\n");
#endif
            }
            if (deleted)
                exit(exitCode);
            else if (rc != JSI_OK) {
                fprintf(stderr, "Error\n");
                exit(1);
            }
        }
    }
#endif

#ifdef USER_EXTENSION
    extern int USER_EXTENSION(Jsi_Interp *interp);
    if (USER_EXTENSION (interp) != JSI_OK) {
        fprintf(stderr, "extension load failed");
        exit(1);
    }
#endif
    Jsi_ShiftArgs(interp);
    if (argc == 1) {
        rc = Jsi_Interactive(interp, JSI_OUTPUT_QUOTE|JSI_OUTPUT_NEWLINES);
    } else {
        if (argc == 2 && (Jsi_Strcmp(argv[1], "--help")==0 || Jsi_Strcmp(argv[1], "-h" )==0)) {
            puts("usage: jsi ?--help | -e SCRIPT | FILE arg arg ...?");
            exit(0);
        }
        if (argc == 2 && (Jsi_Strcmp(argv[1], "--version")==0 || Jsi_Strcmp(argv[1], "-v" )==0)) {
            char str[200] = "\n";
            Jsi_Channel chan = Jsi_Open(interp, Jsi_ValueNewStringKey(interp, "/zvfs/lib/sourceid.txt"), "r");
            if (chan)
                Jsi_Read(chan, str, sizeof(str));
            printf("%d.%d.%d (%" JSI_NUMGFMT ") %s", JSI_VERSION_MAJOR, JSI_VERSION_MINOR, JSI_VERSION_RELEASE, Jsi_Version(), str);
            exit(0);
        }
        if (argc > 2 && (Jsi_Strcmp(argv[1], "--invoke")==0 || Jsi_Strcmp(argv[1], "-i" )==0)) {
            Jsi_DString dStr = {};
            Jsi_DSAppend(&dStr, "jsi_invokeCmd(\"", argv[2], "\",console.args.slice(1));", NULL);
            rc = Jsi_EvalString(interp, Jsi_DSValue(&dStr), JSI_EVAL_NOSKIPBANG);
            Jsi_DSFree(&dStr);
        }
        else if (argc == 3 && (Jsi_Strcmp(argv[1], "--eval")==0 || Jsi_Strcmp(argv[1], "-e" )==0))
            rc = Jsi_EvalString(interp, argv[2], JSI_EVAL_NOSKIPBANG);
        else {
            const char *ext = strrchr(argv[1], '.');
            /* Support running "main.jsi" from a zip file. */
            if (ext && (strcmp(ext,".zip")==0 ||strcmp(ext,".jsz")==0 ) ) {
                rc = Jsi_ExecZip(interp, argv[1], NULL, &jsFound);
                if (rc<0) {
                    fprintf(stderr, "zip mount failed\n");
                    exit(1);
                }
                if (!(jsFound&JSI_ZIP_MAIN)) {
                    fprintf(stderr, "main.jsi not found\n");
                    exit(1);
                }
            } else {
                if (argc>1) {
                    vf = Jsi_ValueNewStringKey(interp, argv[1]);
                    Jsi_IncrRefCount(interp, vf);
                }
                rc = Jsi_EvalFile(interp, vf, JSI_EVAL_ARGV0|JSI_EVAL_INDEX);
                if (vf) {
                    Jsi_DecrRefCount(interp, vf);
                    vf = NULL;
                }
                
            }
        }
        if (deleted)
            exit(exitCode);
        if (rc == 0) {
            /* Skip output from an ending semicolon which evaluates to undefined */
            Jsi_Value *ret = Jsi_ReturnValue(interp);
            if (!Jsi_ValueIsType(interp, ret, JSI_VT_UNDEF)) {
                Jsi_DString dStr = {};
                fputs(Jsi_ValueGetDString(interp, ret, &dStr, 0), stdout);
                Jsi_DSFree(&dStr);
                fputs("\n", stdout);
            }
        } else
            fputs("ERROR\n", stderr);

    }
    if (!deleted)
        Jsi_InterpDelete(interp);
    return rc;
}
