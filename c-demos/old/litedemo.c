// JSI-LITE DEMO: "litedemo.c".  COMPILE WITH: "gcc -DJSI__SQLITE litedemo.c -lm -lsqlite3"
#define JSI_LITE_ONLY
#include "jsi.c"

int main() {
    
    // DATABASE
#ifdef JSI__SQLITE
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
    Jsi_DbQuery(jdb, NULL, ";DROP TABLE IF EXISTS mytable; CREATE TABLE mytable (a int, b TEXT);");

    MyData d = { 99, "A string" };
    Jsi_CDataDb d1[] = {{MyOptions, &d},{}};
    Jsi_DbQuery(jdb, d1, "INSERT INTO mytable %s");
    d.a = 0; d.b[0] = 0;
    Jsi_DbQuery(jdb, d1, "SELECT %s FROM mytable");
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

    Jsi_HashDelete(hPtr);
    Jsi_TreeDelete(tPtr);
    return 0;
}
