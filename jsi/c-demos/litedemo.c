// JSI-LITE DEMO: "litedemo.c".  COMPILE WITH: "gcc -DHAVE_SQLITE litedemo.c -lm -lsqlite3"
#define JSI_LITE_ONLY
#include "jsi.c"

int main() {
    
    // DATABASE
#ifdef HAVE_SQLITE
    typedef struct {
        int a;
        char b[30];
    } MyData;
    
    Jsi_OptionSpec MyOptions[] = {
        JSI_OPT(INT,    MyData, a),
        JSI_OPT(STRBUF, MyData, b),
        JSI_OPT_END(MyData)
    };
    
    Jsi_Db *jdb = Jsi_DbNew("~/mytable.db", 0);
    Jsi_DbQuery(jdb, 0, 0, 0, ";DROP TABLE IF EXISTS mytable; CREATE TABLE mytable (a int, b TEXT);", 0);

    MyData d = { 99, "A string" };
    Jsi_DbQuery(jdb, MyOptions, &d, 1, "INSERT INTO mytable %s", 0);
    d.a = 0; d.b[0] = 0;
    Jsi_DbQuery(jdb, MyOptions, &d, 1, "SELECT %s FROM mytable", 0);
#endif

    // STRINGS
    Jsi_DString dStr = {"Hello World"};
    Jsi_DSAppend(&dStr, " and ", "Goodbye world!", NULL); 

    // HASH
    Jsi_Hash* hPtr = Jsi_HashNew(NULL,JSI_KEYS_STRING, NULL);
    Jsi_HashSet(hPtr, "robin", "1");
    
    // TREE
    Jsi_Tree* tPtr = Jsi_TreeNew(NULL,JSI_KEYS_STRING, NULL);
    Jsi_TreeSet(tPtr, "sparrow", " 2");

    return 0;
}
