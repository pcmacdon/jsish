#define JSI_LITE_ONLY 1
#include "jsi.c"

// Test 1 : simple example where data is a static string.
void dumpList(Jsi_List *list) {
    printf("\nDumping: %d\n", Jsi_ListSize(list));
    Jsi_ListEntry *l;
    for (l = Jsi_ListGetFront(list); l; l = Jsi_ListEntryNext(l))
        puts((char*)Jsi_ListValueGet(l));
}

void test1() {
    puts("\n\nTEST1");
    Jsi_List *list = Jsi_ListNew(NULL,0,NULL);
    Jsi_ListEntry *l = Jsi_ListEntryNew(list, "three", NULL);
    Jsi_ListPush(list, l, NULL);
    Jsi_ListPushFrontNew(list, "hello");
    Jsi_ListPushBackNew(list,  "fine");
    Jsi_ListPushBackNew(list,  "world");

    dumpList(list);
    Jsi_ListEntryDelete(Jsi_ListPopFront(list));
    dumpList(list);
    Jsi_ListEntryDelete(Jsi_ListPopBack(list));
    dumpList(list);
    Jsi_ListClear(list);
    dumpList(list);
    
    Jsi_ListDelete(list);
}

void dumpMap(Jsi_Map *list) {
    printf("\nDumping: %d\n", Jsi_MapSize(list));
    Jsi_MapEntry *l;
    Jsi_MapSearch s;
    for (l = Jsi_MapSearchFirst(list, &s, 0); l; l = Jsi_MapSearchNext(&s))
        puts((char*)Jsi_MapValueGet(l));
    Jsi_MapSearchDone(&s);
}

void test2() {
    puts("\n\nTEST2");
    Jsi_Map *list = Jsi_MapNew(NULL, JSI_MAP_LIST, JSI_KEYS_ONEWORD, NULL);
    Jsi_MapSet(list, NULL, "one");
    Jsi_MapSet(list, NULL, "two");
    Jsi_MapSet(list, NULL, "three");
    dumpMap(list);
    
    Jsi_MapDelete(list);
}

void test3() {
    // Test hash table with structs.
    puts("\n\nTEST3");
    struct {
        int i, j, k;
    } keyv = {1,2,3};
    Jsi_Map *hash = Jsi_MapNew(NULL, JSI_MAP_HASH, sizeof(keyv), NULL);
    Jsi_MapSet(hash, &keyv, "able");
    keyv.i = 2;
    Jsi_MapSet(hash, &keyv, "baker");
    keyv.i = 1;
    printf("Key 1=%s\n", (char*)Jsi_MapGet(hash, &keyv, 0)); 
    keyv.i = 2;
    printf("Key 2=%s\n", (char*)Jsi_MapGet(hash, &keyv, 0)); 
    Jsi_MapDelete(hash);
}

int main() {
    test1();
    test2();
    test3();
    return 0;
}
