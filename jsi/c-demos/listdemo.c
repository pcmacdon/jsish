#define JSI_LITE_ONLY 1
#include "jsi.c"

// Test 1 : simple example where data is a static string.
void dumpList(Jsi_List *list) {
    printf("\nDumping: %d\n", Jsi_ListSize(list));
    Jsi_ListEntry *l;
    for (l = Jsi_ListGetFront(list); l; l = Jsi_ListEntryNext(l))
        puts((char*)Jsi_ListEntryGetValue(l));
}

void test1() {
    puts("\n\nTEST1");
    Jsi_List *list = Jsi_ListNew(NULL);
    Jsi_ListEntry *l = Jsi_ListEntryNew(list);
    Jsi_ListEntrySetValue(l, "there");
    Jsi_ListInsert(list, l, NULL);
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

// TEST 2 : a more complex example where data is an allocated struct, and mutexed.

typedef struct {
    int id;
    time_t time;
} MyList;

typedef struct {
    int n;
    char *name;
} MyListEntry;


void DumpMyList(Jsi_List *list);
Jsi_ListEntry *myData1Create(Jsi_List *list, const char *name);

void test2() {
    puts("\n\nTEST2");
    Jsi_ListAttr attr = { .valueSpace=sizeof(MyListEntry)  };
    Jsi_List *list = Jsi_ListNew(&attr);
    
    Jsi_ListEntry *l = myData1Create(list, "there");
    Jsi_ListInsert(list, l, NULL);

    MyListEntry* m = (MyListEntry*)Jsi_ListEntryGetValue(Jsi_ListGetFront(list));
    printf("Str: %s\n", m->name);
    
    Jsi_ListPushFront(list, myData1Create(list, "hello"));
    Jsi_ListPushBack(list,  myData1Create(list, "fine"));
    Jsi_ListPushBack(list,  myData1Create(list, "world"));

    DumpMyList(list);
    Jsi_ListEntryDelete( Jsi_ListPopFront(list));
    DumpMyList(list);
    Jsi_ListEntryDelete(Jsi_ListPopBack(list));
    DumpMyList(list);
    Jsi_ListClear(list);
    DumpMyList(list);
    
    Jsi_ListDelete(list);
}

Jsi_ListEntry *myData1Create(Jsi_List *list, const char *name) {
    MyListEntry *d  = (MyListEntry*)Jsi_Calloc(1, sizeof(*d));
    d->name = Jsi_Strdup(name);
    d->n = 1;
    Jsi_ListEntry *l =  Jsi_ListEntryNew(list);
    Jsi_ListEntrySetValue(l, d);
    return l;
}


// TEST 3

void myDataFreeProc(Jsi_List *list, Jsi_ListEntry *l);
Jsi_ListEntry *myDataCreate(Jsi_List *list, const char *name);

void test3() {
    puts("\n\nTEST3");
    MyList myData = { 1, time(NULL) };
    Jsi_ListAttr attr = { .data=&myData, .freeProc=myDataFreeProc, .useMutex=1 };
    Jsi_List *list = Jsi_ListNew(&attr);
    
    Jsi_ListEntry *l = myDataCreate(list, "there");
    Jsi_ListInsert(list, l, NULL);
    
    Jsi_ListPushFront(list, myDataCreate(list, "hello"));
    Jsi_ListPushBack(list,  myDataCreate(list, "fine"));
    Jsi_ListPushBack(list,  myDataCreate(list, "world"));

    DumpMyList(list);
    Jsi_ListEntryDelete( Jsi_ListPopFront(list));
    DumpMyList(list);
    Jsi_ListEntryDelete(Jsi_ListPopBack(list));
    DumpMyList(list);
    Jsi_ListClear(list);
    DumpMyList(list);
    
    Jsi_ListDelete(list);
}


Jsi_ListEntry *myDataCreate(Jsi_List *list, const char *name) {
    MyListEntry *d  = (MyListEntry *)Jsi_Calloc(1, sizeof(*d));
    d->name = Jsi_Strdup(name);
    d->n = 1;
    Jsi_ListEntry *l =  Jsi_ListEntryNew(list);
    Jsi_ListEntrySetValue(l, d);
    return l;
}

void DumpMyList(Jsi_List *list) {
    printf("\nDumping: %d\n", Jsi_ListSize(list));
    Jsi_ListEntry *l;
    
    if (Jsi_ListLock(list, 1) != JSI_OK)
        return;
    for (l = Jsi_ListGetFront(list); l; l = Jsi_ListEntryNext(l)) {
        MyListEntry *d  = (MyListEntry *)l->value;
        puts(d->name);
    }
    Jsi_ListLock(list, 0);

}

void myDataFreeProc(Jsi_List *list, Jsi_ListEntry *l) {
    if (!l) { // Freeing list itself.
        free(list);
    } else {
        // Free one entry
        MyListEntry *d  = (MyListEntry *)l->value;
        if (d) {
            if (d->name)
                free(d->name);
            free(d);
        }
        free(l);
    }
}


int main() {
    test1();
    test2();
    return 0;
}
