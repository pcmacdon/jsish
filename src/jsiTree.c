/* An implementation of Red-Black Trees with invariant node pointers. 
 * Nodes are allocated using single malloc that includes the key. 
 * This means that string/struct keys (which are of varying length) can not be copied between nodes.
 * So instead of swapping node key/values, positions are swapped when balancing the tree. */

#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

enum {_JSI_TREE_RED=0, _JSI_TREE_BLACK=1};

/********************** _JSI_TREE_RED/_JSI_TREE_BLACK HELPERS **************************/

static Jsi_TreeEntry* grandparent(Jsi_TreeEntry* n) {
    Assert (n != NULL);
    Assert (n->parent != NULL);
    Assert (n->parent->parent != NULL);
    return n->parent->parent;
}

static Jsi_TreeEntry* sibling(Jsi_TreeEntry* n) {
    Assert (n != NULL);
    Assert (n->parent != NULL);
    return (n == n->parent->left ? n->parent->right : n->parent->left);
}

static Jsi_TreeEntry* uncle(Jsi_TreeEntry* n) {
    Assert (n != NULL);
    Assert (n->parent != NULL);
    Assert (n->parent->parent != NULL);
    return sibling(n->parent);
}

static int node_color(Jsi_TreeEntry* n) {
    return n == NULL ? _JSI_TREE_BLACK : n->f.bits.color;
}

static void set_color(Jsi_TreeEntry* n, int color) {
    if (color == _JSI_TREE_BLACK && n == NULL) return;
    n->f.bits.color = color;
}

static void replace_node(Jsi_TreeEntry* oldn, Jsi_TreeEntry* newn) {
    Assert(oldn);
    Jsi_Tree* t = oldn->treePtr;
    if (oldn->parent == NULL) {
        t->root = newn;
    } else {
        if (oldn == oldn->parent->left)
            oldn->parent->left = newn;
        else
            oldn->parent->right = newn;
    }
    if (newn != NULL) {
        newn->parent = oldn->parent;
    }
}

static void rotate_left(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* r;
    Assert(n);
    r = n->right;
    replace_node(n, r);
    n->right = r->left;
    if (r->left != NULL) {
        r->left->parent = n;
    }
    r->left = n;
    n->parent = r;
}

static void rotate_right(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* l;
    Assert(n);
    l = n->left;
    replace_node(n, l);
    n->left = l->right;
    if (l->right != NULL) {
        l->right->parent = n;
    }
    l->right = n;
    n->parent = l;
}

static void insert_case5(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* g = grandparent(n);
    set_color(n->parent, _JSI_TREE_BLACK);
    set_color(g, _JSI_TREE_RED);
    if (n == n->parent->left) {
        rotate_right(g);
    } else {
        Assert (n == n->parent->right);
        rotate_left(g);
    }
}

static void insert_case4(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* g = grandparent(n);
    if (n == n->parent->right && n->parent == g->left) {
        rotate_left(n->parent);
        n = n->left;
    } else if (n == n->parent->left && n->parent == g->right) {
        rotate_right(n->parent);
        n = n->right;
    }
    insert_case5(n);
}

static void insert_case1(Jsi_TreeEntry* n);

static void insert_case3(Jsi_TreeEntry* n) {
    Jsi_TreeEntry *g, *u = uncle(n);
    if (node_color(u) == _JSI_TREE_RED) {
        set_color(n->parent, _JSI_TREE_BLACK);
        set_color(u, _JSI_TREE_BLACK);
        g = grandparent(n);
        set_color(g, _JSI_TREE_RED);
        insert_case1(g);
    } else {
        insert_case4(n);
    }
}

static void insert_case2(Jsi_TreeEntry* n) {
    if (node_color(n->parent) == _JSI_TREE_BLACK)
        return;
    insert_case3(n);
}

static void insert_case1(Jsi_TreeEntry* n) {
    if (n->parent == NULL)
        set_color(n, _JSI_TREE_BLACK);
    else
        insert_case2(n);
}

static Jsi_TreeEntry* maximum_node(Jsi_TreeEntry* n) {
    Assert (n != NULL);
    while (n->right != NULL) {
        n = n->right;
    }
    return n;
}

static void delete_case6(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* s = sibling(n);
    set_color(s, node_color(n->parent));
    set_color(n->parent, _JSI_TREE_BLACK);
    if (n == n->parent->left) {
        Assert (node_color(s->right) == _JSI_TREE_RED);
        set_color(s->right, _JSI_TREE_BLACK);
        rotate_left(n->parent);
    }
    else
    {
        //Assert (node_color(s->left) == _JSI_TREE_RED);
        set_color(s->left, _JSI_TREE_BLACK);
        rotate_right(n->parent);
    }
}

static void delete_case5(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* s = sibling(n);
    if (node_color(s) == _JSI_TREE_BLACK ) {
        if (n == n->parent->left &&
                node_color(s->right) == _JSI_TREE_BLACK &&
                node_color(s->left) == _JSI_TREE_RED)
        {
            set_color(s, _JSI_TREE_RED);
            set_color(s->left, _JSI_TREE_BLACK);
            rotate_right(s);
        }
        else if (n == n->parent->right &&
                 node_color(s->right) == _JSI_TREE_RED &&
                 node_color(s->left) == _JSI_TREE_BLACK)
        {
            set_color(s, _JSI_TREE_RED);
            set_color(s->right, _JSI_TREE_BLACK);
            rotate_left(s);
        }
    }
    delete_case6(n);
}

static void delete_case4(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* s = sibling(n);
    if (node_color(n->parent) == _JSI_TREE_RED &&
            node_color(s) == _JSI_TREE_BLACK &&
            node_color(s->left) == _JSI_TREE_BLACK &&
            node_color(s->right) == _JSI_TREE_BLACK)
    {
        set_color(s, _JSI_TREE_RED);
        set_color(n->parent, _JSI_TREE_BLACK);
    }
    else
        delete_case5(n);
}

static void delete_case1(Jsi_TreeEntry* n);

static void delete_case3(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* s  = sibling(n);
    if (node_color(n->parent) == _JSI_TREE_BLACK &&
        node_color(s) == _JSI_TREE_BLACK &&
        node_color(s->left) == _JSI_TREE_BLACK &&
        node_color(s->right) == _JSI_TREE_BLACK)
    {
        set_color(s, _JSI_TREE_RED);
        delete_case1(n->parent);
    } else
        delete_case4(n);
}

static void delete_case2(Jsi_TreeEntry* n) {
    Jsi_TreeEntry* s = sibling(n);
    if (node_color(s) == _JSI_TREE_RED) {
        set_color(n->parent, _JSI_TREE_RED);
        set_color(s, _JSI_TREE_BLACK);
        if (n == n->parent->left)
            rotate_left(n->parent);
        else
            rotate_right(n->parent);
    }
    delete_case3(n);
}

static void delete_case1(Jsi_TreeEntry* n) {
    if (n->parent != NULL)
        delete_case2(n);
}

/***********************************************************/

int jsi_treeHeight(Jsi_TreeEntry* hPtr, int n)
{
    int l = -1, r = -1;
    if (hPtr->right == NULL && hPtr->right == NULL )
        return n;
    if (hPtr->left)
        l = jsi_treeHeight(hPtr->left, n+1);
    if (hPtr->right)
        r = jsi_treeHeight(hPtr->right, n+1);
    return (r > l ? r : l);
}

int jsi_nodeDepth(Jsi_TreeEntry* hPtr) {
    int d = 0;
    while (hPtr->parent != NULL) {
        d++;
        hPtr = hPtr->parent;
    }
    return d;
}


static int StringPtrCompare(Jsi_Tree *treePtr, const void *key1, const void *key2)
{
    //return (key1 - key2);
    if (key1 == key2) return 0;
    //return Jsi_DictionaryCompare((char*)key1, (char*)key2);
    return Jsi_Strcmp((char*)key1, (char*)key2);
}


static int StringCompare(Jsi_Tree *treePtr, const void *key1, const void *key2)
{
    return Jsi_DictionaryCompare((char*)key1, (char*)key2);
    //return Jsi_Strcmp((char*)key1, (char*)key2);
}

static int OneWordCompare(Jsi_Tree *treePtr, const void *key1, const void *key2)
{
    return ((uintptr_t )key1 - (uintptr_t)key2);
}

static int TreeArrayCompare(Jsi_Tree *treePtr, const void *key1, const void *key2)
{
    return memcmp(key1, key2, treePtr->keyType);
}


static Jsi_TreeEntry *TreeStringCreate( Jsi_Tree *treePtr, const void *key, bool *newPtr)
{
    Jsi_TreeEntry *hPtr;
    size_t size;

    if ((hPtr = Jsi_TreeEntryFind(treePtr, key))) {
        if (newPtr)
            *newPtr = 0;
        return hPtr;
    }
    if (newPtr)
        *newPtr = 1;
    size = sizeof(Jsi_TreeEntry) + Jsi_Strlen((char*)key) /*- sizeof(jsi_TreeKey)*/ + 1;
    hPtr = (Jsi_TreeEntry*)Jsi_Calloc(1,size);
    SIGINIT(hPtr,TREEENTRY);
    hPtr->typ = JSI_MAP_TREE;
    hPtr->treePtr = treePtr;
    hPtr->value = 0;
    Jsi_Strcpy(hPtr->key.string, (char*)key);
    treePtr->numEntries++;
    return hPtr;
}

static Jsi_TreeEntry *TreeArrayCreate(Jsi_Tree *treePtr, const void *key, bool *newPtr)
{
    Jsi_TreeEntry *hPtr;
    size_t size;

    if ((hPtr = Jsi_TreeEntryFind(treePtr, key))) {
        if (newPtr)
            *newPtr = 0;
        return hPtr;
    }
    if (newPtr)
        *newPtr = 1;
    size = sizeof(Jsi_TreeEntry) + treePtr->keyType; /*- sizeof(jsi_TreeKey);*/
    hPtr = (Jsi_TreeEntry*)Jsi_Calloc(1,size);
    SIGINIT(hPtr,TREEENTRY);
    hPtr->typ = JSI_MAP_TREE;
    hPtr->treePtr = treePtr;
    hPtr->value = 0;
    memcpy(hPtr->key.string, key, treePtr->keyType);
    treePtr->numEntries++;
    return hPtr;
}

static Jsi_TreeEntry *OneWordCreate( Jsi_Tree *treePtr, const void *key, bool *newPtr)
{
    Jsi_TreeEntry *hPtr;
    size_t size;
    if ((hPtr = Jsi_TreeEntryFind(treePtr, key))) {
        if (newPtr)
            *newPtr = 0;
        return hPtr;
    }
    if (newPtr)
        *newPtr = 1;
    size = sizeof(Jsi_TreeEntry);
    hPtr = (Jsi_TreeEntry*)Jsi_Calloc(1,size);
    SIGINIT(hPtr,TREEENTRY);
    hPtr->typ = JSI_MAP_TREE;
    hPtr->treePtr = treePtr;
    hPtr->value = 0;
    hPtr->key.oneWordValue = (void *)key;
    treePtr->numEntries++;
    return hPtr;
}


static Jsi_TreeEntry *StringPtrCreate( Jsi_Tree *treePtr, const void *key, bool *newPtr)
{
    return OneWordCreate(treePtr, key, newPtr);
}

void *Jsi_TreeValueGet(Jsi_TreeEntry *hPtr)
{
    return hPtr->value;
}

void *Jsi_TreeKeyGet(Jsi_TreeEntry *hPtr)
{
    Jsi_Tree *t = hPtr->treePtr;
    return (t->keyType == JSI_KEYS_ONEWORD || t->keyType == JSI_KEYS_STRINGKEY ? hPtr->key.oneWordValue : hPtr->key.string);
}


Jsi_TreeEntry *Jsi_TreeEntryFind (Jsi_Tree *treePtr, const void *key)
{
    Jsi_TreeEntry* hPtr = treePtr->root;
    int rc;
    if (treePtr->flags.destroyed)
        return NULL;
    if (treePtr->opts.lockTreeProc && (*treePtr->opts.lockTreeProc)(treePtr, 1) != JSI_OK)
        return NULL;
    while (hPtr != NULL) {
        rc = treePtr->opts.compareTreeProc(treePtr, Jsi_TreeKeyGet(hPtr), key);
        if (rc == 0) {
            break;
        }
        hPtr = (rc < 0 ? hPtr->left : hPtr->right);
    }
    if (treePtr->opts.lockTreeProc)
        (*treePtr->opts.lockTreeProc)(treePtr, 0);
    return hPtr;
}

Jsi_TreeEntry *Jsi_TreeEntryNew(Jsi_Tree *treePtr, const void *key, bool *isNew)
{
    Jsi_TreeEntry* hPtr;
    bool isn;
    if (treePtr->flags.destroyed)
        return NULL;
    if (treePtr->opts.lockTreeProc && (*treePtr->opts.lockTreeProc)(treePtr, 1) != JSI_OK)
        return NULL;
    treePtr->flags.inserting=1;
    if (treePtr->flags.internstr) {
        Assert(treePtr->keyType == JSI_KEYS_STRINGKEY);
        if (!treePtr->strHash)
            treePtr->strHash = Jsi_HashNew(treePtr->opts.interp, JSI_KEYS_STRING, NULL);
        key = Jsi_HashEntryNew(treePtr->strHash, key, NULL);
    }
    hPtr = treePtr->createProc(treePtr, key, &isn);
    if (isNew)
        *isNew = isn;
    if (isn == 0 || treePtr->flags.nonredblack == 1 || !hPtr) {
        treePtr->flags.inserting=0;
        goto done;
    }
    treePtr->epoch++;
    hPtr->f.bits.color = _JSI_TREE_RED;
    if (treePtr->root == NULL) {
        treePtr->root = hPtr;
    } else {
        Jsi_TreeEntry* n = treePtr->root;
        while (1) {
            int rc = treePtr->opts.compareTreeProc(treePtr, Jsi_TreeKeyGet(n) , key);
            if (rc == 0) {
                Assert(0);
            } else if (rc < 0) {
                if (n->left == NULL) {
                    n->left = hPtr;
                    break;
                } else {
                    n = n->left;
                }
            } else {
                if (n->right == NULL) {
                    n->right = hPtr;
                    break;
                } else {
                    n = n->right;
                }
            }
        }
        hPtr->parent = n;
    }
    insert_case1(hPtr);
    treePtr->flags.inserting = 0;
done:
    if (treePtr->opts.lockTreeProc)
        (*treePtr->opts.lockTreeProc)(treePtr, 0);
    return hPtr;
}

Jsi_Tree *Jsi_TreeNew(Jsi_Interp *interp, unsigned int keyType, Jsi_TreeDeleteProc *freeProc)
{
    Jsi_Tree* treePtr = (Jsi_Tree*)Jsi_Calloc(1,sizeof(Jsi_Tree));
    SIGINIT(treePtr,TREE);
    treePtr->opts.mapType = (Jsi_Map_Type)JSI_MAP_TREE;
    treePtr->typ = (Jsi_Map_Type)JSI_MAP_TREE;
    treePtr->root = NULL;
    treePtr->opts.interp = interp;
    treePtr->numEntries = 0;
    treePtr->epoch = 0;
    treePtr->opts.keyType = (Jsi_Key_Type)keyType;
    treePtr->keyType = (Jsi_Key_Type)keyType;
    treePtr->opts.freeTreeProc = freeProc;

    switch (keyType) {
    case JSI_KEYS_STRING:   /* NULL terminated string keys. */
        treePtr->opts.compareTreeProc = StringCompare;
        treePtr->createProc = TreeStringCreate;
        break;

    case JSI_KEYS_STRINGKEY: /*  */
        treePtr->opts.compareTreeProc = StringPtrCompare;
        treePtr->createProc = StringPtrCreate;
        break;
        
    case JSI_KEYS_ONEWORD: /* 32 or 64 bit atomic keys. */
        treePtr->opts.compareTreeProc = OneWordCompare;
        treePtr->createProc = OneWordCreate;
        break;


    default:            /* Struct. */
        if (keyType < JSI_KEYS_STRUCT_MINSIZE) {
            Jsi_LogError("Jsi_TreeNew: Key size can't be %d, must be > %d", keyType, JSI_KEYS_STRUCT_MINSIZE);
            Jsi_Free(treePtr);
            return NULL;
        }
        treePtr->opts.compareTreeProc = TreeArrayCompare;
        treePtr->createProc = TreeArrayCreate;
        break;
    }
    return treePtr;
}

static void destroy_node(Jsi_Interp *interp, Jsi_TreeEntry* n)
{
    if (n == NULL) return;
    if (n->right != NULL) destroy_node(interp, n->right);
    if (n->left != NULL) destroy_node(interp, n->left);
    n->left = n->right = NULL;
    Jsi_TreeEntryDelete(n);
}

void Jsi_TreeClear (Jsi_Tree *treePtr)
{
    SIGASSERTV(treePtr, TREE);
    if (treePtr->opts.lockTreeProc && (*treePtr->opts.lockTreeProc)(treePtr, 1) != JSI_OK)
        return;
    destroy_node(treePtr->opts.interp, treePtr->root);
    treePtr->root = NULL;
    if (treePtr->opts.lockTreeProc)
        (*treePtr->opts.lockTreeProc)(treePtr, 0);
}

void Jsi_TreeDelete (Jsi_Tree *treePtr)
{
    SIGASSERTV(treePtr, TREE);
    if (treePtr->flags.destroyed)
        return;
    //Jsi_TreeClear(treePtr);
    treePtr->flags.destroyed = 1;
    destroy_node(treePtr->opts.interp, treePtr->root);
    _JSI_MEMCLEAR(treePtr);
    Jsi_Free(treePtr);
}

/* Swap positions of nodes in tree.  This avoids moving the value, which we can't do for strings/structs. */
static void SwapNodes(Jsi_TreeEntry* n, Jsi_TreeEntry* m)
{
    Jsi_Tree* t = n->treePtr;
    Jsi_TreeEntry *np, *nl, *nr, *mp, *ml, *mr;
    int mpc = 0, npc = 0, col = n->f.bits.color;
    n->f.bits.color = m->f.bits.color;  m->f.bits.color = col;
    np = n->parent; nl = n->left; nr = n->right;
    mp = m->parent; ml = m->left; mr = m->right;
    if (mp) mpc = (mp->left == m ?1 : 2);
    if (np) npc = (np->left == n ?1 : 2);

    n->parent = mp; n->left = ml; n->right = mr;
    m->parent = np; m->left = nl; m->right = nr;
    
    if (np == m) {
        m->parent = n;
        if (mr == n) n->right = m; else n->left = m;
    } else if (mp == n) {
        n->parent = m;
        if (nr == m) m->right = n; else m->left = n;
    }
    /* Fixup back pointers. */
    if (m->left)  m->left->parent  = m;
    if (m->right) m->right->parent = m;
    if (n->left)  n->left->parent  = n;
    if (n->right) n->right->parent = n;
    if (mpc) { if (mpc==1) n->parent->left = n; else  n->parent->right = n;}
    if (npc) { if (npc==1) m->parent->left = m; else  m->parent->right = m; }
    if (n->parent == NULL) {
        t->root = n;
    } else if (m->parent == NULL) {
        t->root = m;
    }
}

static void delete_one_child(Jsi_TreeEntry*n)
{
    Jsi_TreeEntry *child;
    Assert(n->left == NULL || n->right == NULL);
    child = n->right == NULL ? n->left  : n->right;
#if 1
    if (node_color(n) == _JSI_TREE_BLACK) {
        set_color(n, node_color(child));
        delete_case1(n);
    }
    replace_node(n, child);
    if (n->parent == NULL && child != NULL)
        set_color(child, _JSI_TREE_BLACK);
    
#else
    replace_node(n, child);
    if (node_color(n) == _JSI_TREE_BLACK) {
        if (node_color(child) == _JSI_TREE_RED)
            child->f.bits.color = _JSI_TREE_BLACK;
        else
            delete_case1(n);
    }
#endif
}

int Jsi_TreeEntryDelete (Jsi_TreeEntry *entryPtr)
{
    int cnt = 0;
    Jsi_TreeEntry* n = entryPtr;
    Jsi_Tree* treePtr = n->treePtr;

    if (treePtr->flags.destroyed  || treePtr->flags.nonredblack == 1 /* || entryPtr->f.bits.deletesub */) {
        goto dodel;
    }
    /*printf("DEL(tree=%p,root=%p): (%p)%s\n", treePtr, treePtr->root, entryPtr,(char*)entryPtr->key.string);*/
    /*dumpTree(treePtr);*/
    if (treePtr->opts.lockTreeProc && (*treePtr->opts.lockTreeProc)(treePtr, 1) != JSI_OK)
        return -1;
    entryPtr->treePtr->epoch++;
    if (n->left != NULL && n->right != NULL) {
        /* swap key/values delete pred instead */
        Jsi_TreeEntry* pred = maximum_node(n->left);
        switch (treePtr->keyType) {
        case JSI_KEYS_STRINGKEY:
        case JSI_KEYS_ONEWORD: {
            void *nv = n->value;
            n->value = pred->value;
            pred->value = nv;
            nv = n->key.oneWordValue;
            n->key.oneWordValue = pred->key.oneWordValue;
            pred->key.oneWordValue = nv;
            n = pred;
            break;
        }
        case JSI_KEYS_STRING:
            SwapNodes(n,pred);
            break;
        default: { // Struct keys have the same length so we swap bytes.
            uint i;
            void *nv = n->value;
            n->value = pred->value;
            pred->value = nv;
            char ct, *cs = (char*)(n->key.string), *cd = (char*)(pred->key.string);
            for (i=0; i<treePtr->keyType; i++, cs++, cd++) {
                ct = *cd;
                *cd = *cs;
                *cs = ct;
            }
        }
                
        }
    }
    delete_one_child(n);
    cnt++;
    /*dumpTree(treePtr);*/
dodel:
    treePtr->numEntries--;
    n->treePtr = NULL;
    if (treePtr->opts.freeTreeProc && n && n->value)
        (treePtr->opts.freeTreeProc)(treePtr->opts.interp, n, n->value);
    Jsi_Free(n);
    if (treePtr->opts.lockTreeProc)
        (*treePtr->opts.lockTreeProc)(treePtr, 0);
    return cnt;
}

static void searchSpace(Jsi_TreeSearch *searchPtr, int n)
{
    if ((searchPtr->top+n) >= searchPtr->max) {
        int i, cnt = (searchPtr->max *= 2);
        if (searchPtr->Ptrs == searchPtr->staticPtrs)
            searchPtr->Ptrs = (Jsi_TreeEntry**)Jsi_Calloc(cnt, sizeof(Jsi_TreeEntry*));
        else
            searchPtr->Ptrs = (Jsi_TreeEntry**)Jsi_Realloc(searchPtr->Ptrs, cnt* sizeof(Jsi_TreeEntry*));
        for (i=0; i<cnt; i++)
            SIGINIT((searchPtr->Ptrs[i]),TREEENTRY);

    }
}

static Jsi_TreeEntry *searchAdd(Jsi_TreeSearch *searchPtr,  Jsi_TreeEntry *hPtr)
{
    int order = (searchPtr->flags & JSI_TREE_ORDER_MASK);
    searchSpace(searchPtr, 2);
    switch (order) {
        case JSI_TREE_ORDER_LEVEL:
            if (hPtr) {
                if (hPtr->right)
                    searchPtr->Ptrs[searchPtr->top++] = hPtr->right;
                if (hPtr->left)
                    searchPtr->Ptrs[searchPtr->top++] = hPtr->left;
                return hPtr;
            }
            if (searchPtr->top<=0)
                return NULL;
            hPtr = searchPtr->Ptrs[0];
            searchPtr->top--;
            if (searchPtr->top > 0) {
                /* Not very efficient way to implement a queue, but works for now. */
                memmove(searchPtr->Ptrs, searchPtr->Ptrs+1, sizeof(Jsi_TreeEntry*)*searchPtr->top);
            }
            if (hPtr->right)
                searchPtr->Ptrs[searchPtr->top++] = hPtr->right;
            if (hPtr->left)
                searchPtr->Ptrs[searchPtr->top++] = hPtr->left;
            return hPtr;
            break;
            
        case JSI_TREE_ORDER_POST:
            if (hPtr)
                searchPtr->Ptrs[searchPtr->top++] = searchPtr->current = hPtr;
            while (searchPtr->top>0) {
                hPtr = searchPtr->Ptrs[searchPtr->top-1];
                if (hPtr->right == searchPtr->current || hPtr->left == searchPtr->current ||
                    (hPtr->left == NULL && hPtr->right == NULL)) {
                    searchPtr->top--;
                    searchPtr->current = hPtr;
                    return hPtr;
                } else {
                    searchSpace(searchPtr, 2);
                    if (hPtr->left)
                        searchPtr->Ptrs[searchPtr->top++] = hPtr->left;
                    if (hPtr->right)
                        searchPtr->Ptrs[searchPtr->top++] = hPtr->right;
                }
            }
            return NULL;
            break;
            
        case JSI_TREE_ORDER_PRE:
            if (!hPtr) {
                if (searchPtr->top<=0) return NULL;
                hPtr = searchPtr->Ptrs[--searchPtr->top];
            }
            searchPtr->Ptrs[searchPtr->top++] = hPtr;
            if (hPtr->left) searchPtr->Ptrs[searchPtr->top++] = hPtr->left;
            if (hPtr->right) searchPtr->Ptrs[searchPtr->top++] = hPtr->right;
            break;
            
        case JSI_TREE_ORDER_IN:
            while (1) {
                searchSpace(searchPtr, 2);
                if (searchPtr->current) {
                    searchPtr->Ptrs[searchPtr->top++] = searchPtr->current;
                    searchPtr->current = searchPtr->current->right;
                } else {
                    if (searchPtr->top<=0)
                        return NULL;
                    hPtr = searchPtr->Ptrs[--searchPtr->top] ;
                    searchPtr->current = hPtr->left;
                    return hPtr;
                }
            }
            break;
            
        default:
            if (hPtr) {
                Jsi_Interp *interp = hPtr->treePtr->opts.interp;
                JSI_NOTUSED(interp);
                Jsi_LogError("Invalid order: %d", order);    
            }    
    }
    return searchPtr->Ptrs[--searchPtr->top];
}

Jsi_TreeEntry *Jsi_TreeSearchFirst (Jsi_Tree *treePtr, Jsi_TreeSearch *searchPtr, int flags, const void *startKey)
{
    Jsi_TreeEntry *hPtr = NULL, *hPtr2 = NULL;
    if (!treePtr) return NULL;
    memset(searchPtr, 0, sizeof(*searchPtr));
    searchPtr->treePtr = treePtr;
    searchPtr->flags = flags;
    searchPtr->Ptrs = searchPtr->staticPtrs;
    searchPtr->max = sizeof(searchPtr->staticPtrs)/sizeof(searchPtr->staticPtrs[0]);
    searchPtr->epoch = treePtr->epoch;
    if (startKey || (flags & JSI_TREE_SEARCH_KEY))
        hPtr2 = Jsi_TreeEntryFind(treePtr, startKey);;
    searchPtr->current = treePtr->root;
    hPtr = searchAdd(searchPtr, treePtr->root);
    if (hPtr2 && hPtr && hPtr2 != hPtr)
        while (hPtr && hPtr2 != hPtr) // TODO: need a more efficient way to do this...
            hPtr = Jsi_TreeSearchNext(searchPtr);
    return hPtr;
}

void Jsi_TreeValueSet(Jsi_TreeEntry *hPtr, void *value)
{
    Jsi_Value *v = (Jsi_Value*)value;
#if JSI__MEMDEBUG
    SIGASSERTV(hPtr, TREEENTRY);
    if (hPtr->treePtr->flags.valuesonly)
        SIGASSERTV(v,VALUE);
#endif
    hPtr->value = v;
}

#ifndef JSI_LITE_ONLY

Jsi_Tree *Jsi_TreeFromValue(Jsi_Interp *interp, Jsi_Value *v)
{
    if (!Jsi_ValueIsObjType(interp, v, JSI_OT_OBJECT))
        return NULL;
    return v->d.obj->tree;
}

#endif 

Jsi_TreeEntry *Jsi_TreeSearchNext(Jsi_TreeSearch *searchPtr)
{
    Jsi_TreeEntry *hPtr = NULL;
    if (searchPtr->epoch == searchPtr->treePtr->epoch)
        hPtr = searchAdd(searchPtr, NULL);
    if (!hPtr)
        Jsi_TreeSearchDone(searchPtr);
    return hPtr;
}

void Jsi_TreeSearchDone(Jsi_TreeSearch *searchPtr)
{
    if (searchPtr->Ptrs != searchPtr->staticPtrs)
        Jsi_Free(searchPtr->Ptrs);
    searchPtr->Ptrs = searchPtr->staticPtrs;
    searchPtr->top = 0;
}

Jsi_TreeEntry *Jsi_TreeSet(Jsi_Tree *treePtr, const void *key, void *value)
{
    Jsi_TreeEntry *hPtr;
    bool isNew;
    hPtr = Jsi_TreeEntryNew(treePtr, key, &isNew);
    if (!hPtr) return hPtr;
    Jsi_TreeValueSet(hPtr, value);
    return hPtr;
}

void *Jsi_TreeGet(Jsi_Tree *treePtr, void *key, int flags)
{
    Jsi_TreeEntry *hPtr = Jsi_TreeEntryFind(treePtr, key);
    if (!hPtr)
        return NULL;
    return Jsi_TreeValueGet(hPtr);
}

// Delete entry, and invoke freeProc.
bool Jsi_TreeUnset(Jsi_Tree *treePtr, void *key) {
    Jsi_TreeEntry *hPtr = Jsi_TreeEntryFind(treePtr, key);
    if (!hPtr)
        return false;
    Jsi_TreeEntryDelete(hPtr);
    return true;
}

static int tree_inorder(Jsi_Tree *treePtr, Jsi_TreeEntry *hPtr, Jsi_TreeWalkProc *callback, void *data) {
    uint epoch = treePtr->epoch;
    if (hPtr == NULL) return JSI_OK;
    if (hPtr->right != NULL) {
        if (tree_inorder(treePtr, hPtr->right, callback, data) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
    }
    if (callback(treePtr, hPtr, data) != JSI_OK || epoch != treePtr->epoch)
        return JSI_ERROR;
    Assert(hPtr->treePtr);
    if (hPtr->left != NULL) {
        if (tree_inorder(treePtr, hPtr->left, callback, data) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
    }
    return JSI_OK;
}


static int tree_preorder(Jsi_Tree *treePtr, Jsi_TreeEntry *hPtr, Jsi_TreeWalkProc *callback, void *data) {
    uint epoch = treePtr->epoch;
    if (hPtr == NULL) return JSI_OK;
    if (callback(treePtr, hPtr, data) != JSI_OK || epoch != treePtr->epoch)
        return JSI_ERROR;
    if (hPtr->right != NULL) {
        if (tree_preorder(treePtr, hPtr->right, callback, data) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
    }
    if (hPtr->left != NULL) {
        if (tree_preorder(treePtr, hPtr->left, callback, data) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
    }
    return JSI_OK;
}


static int tree_postorder(Jsi_Tree *treePtr, Jsi_TreeEntry *hPtr, Jsi_TreeWalkProc *callback, void *data) {
    uint epoch = treePtr->epoch;
    if (hPtr == NULL) return JSI_OK;
    if (hPtr->right != NULL) {
        if (tree_postorder(treePtr, hPtr->right, callback, data) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
    }
    if (hPtr->left != NULL) {
        if (tree_postorder(treePtr, hPtr->left, callback, data) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
    }
    if (callback(treePtr, hPtr, data) != JSI_OK || epoch != treePtr->epoch)
        return JSI_ERROR;
    return JSI_OK;
}


static int tree_levelorder(Jsi_Tree *treePtr, Jsi_TreeEntry *hPtr, Jsi_TreeWalkProc *callback,
    void *data, int curlev, int level, int *cnt) {
    uint epoch = treePtr->epoch;
    if (hPtr == NULL) return JSI_OK;
    if (curlev > level) return JSI_OK;
    if (curlev == level) {
        if (callback(treePtr, hPtr, data) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
        (*cnt)++;
    }
    if (hPtr->right != NULL) {
        if (tree_levelorder(treePtr, hPtr->right, callback, data, curlev+1, level, cnt) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
    }
    if (hPtr->left != NULL) {
        if (tree_levelorder(treePtr, hPtr->left, callback, data, curlev+1, level, cnt) != JSI_OK || epoch != treePtr->epoch)
            return JSI_ERROR;
    }
    return JSI_OK;
}


int Jsi_TreeWalk(Jsi_Tree* treePtr, Jsi_TreeWalkProc* callback, void *data, int flags) {
    Jsi_Interp *interp = treePtr->opts.interp;
    JSI_NOTUSED(interp);
    int n = 0, m = -1, lastm, order;
    order = flags & JSI_TREE_ORDER_MASK;
    switch (order) {
    case JSI_TREE_ORDER_PRE:
        return tree_preorder(treePtr, treePtr->root, callback, data);
    case JSI_TREE_ORDER_POST:
        return tree_postorder(treePtr, treePtr->root, callback, data);
    case JSI_TREE_ORDER_IN:
        return tree_inorder(treePtr, treePtr->root, callback, data);
    case JSI_TREE_ORDER_LEVEL:
        while (1) {
            lastm = m;
            if (tree_levelorder(treePtr, treePtr->root, callback, data, 0, n, &m) != JSI_OK)
                return JSI_ERROR;
            if (lastm == m)
                return JSI_OK;
            n++;
        }
            
    default:
        Jsi_LogError("Invalid order: %d", order);
    }
    return JSI_ERROR;
}

#ifdef JSI_TEST_RBTREE

JSI_RC mycall(Jsi_Tree* treePtr, Jsi_TreeEntry* hPtr, void *data)
{
    printf("CALL: %s(%d) : %d\n", (char*)Jsi_TreeKeyGet(hPtr), jsi_nodeDepth(hPtr), (int)Jsi_TreeValueGet(hPtr));
    return JSI_OK;
}

static void TreeTest(Jsi_Interp* interp) {
    Jsi_Tree *st, *wt, *mt;
    Jsi_TreeEntry *hPtr, *hPtr2;
    bool isNew, i;
    Jsi_TreeSearch srch;
    struct tdata {
        int n;
        int m;
    } t1, t2;
    char nbuf[100];
    
    wt = Jsi_TreeNew(interp, JSI_KEYS_ONEWORD, NULL);
    mt = Jsi_TreeNew(interp, sizeof(struct tdata), NULL);

    Jsi_TreeSet(wt, wt,(void*)0x88);
    Jsi_TreeSet(wt, mt,(void*)0x99);
    printf("WT: %p\n", Jsi_TreeGet(wt, mt));
    printf("WT2: %p\n", Jsi_TreeGet(wt, wt));
    Jsi_TreeDelete(wt);

    t1.n = 0; t1.m = 1;
    t2.n = 1; t2.m = 2;
    Jsi_TreeSet(mt, &t1,(void*)0x88);
    Jsi_TreeSet(mt, &t2,(void*)0x99);
    Jsi_TreeSet(mt, &t2,(void*)0x98);
    printf("CT: %p\n", Jsi_TreeGet(mt, &t1));
    printf("CT2: %p\n", Jsi_TreeGet(mt, &t2));
    Jsi_TreeDelete(mt);

    st = Jsi_TreeNew(interp, JSI_KEYS_STRING, NULL);
    hPtr = Jsi_TreeEntryNew(st, "bob", &isNew);
    Jsi_TreeValueSet(hPtr, (void*)99);
    Jsi_TreeSet(st, "zoe",(void*)77);
    hPtr2 = Jsi_TreeSet(st, "ted",(void*)55);
    Jsi_TreeSet(st, "philip",(void*)66);
    Jsi_TreeSet(st, "alice",(void*)77);
    puts("SRCH");
    for (hPtr=Jsi_TreeSearchFirst(st,&srch,  JSI_TREE_ORDER_IN, NULL); hPtr; hPtr=Jsi_TreeSearchNext(&srch))
        mycall(st, hPtr, NULL);
    Jsi_TreeSearchDone(&srch);
    puts("IN");
    Jsi_TreeWalk(st, mycall, NULL, JSI_TREE_ORDER_IN);
    puts("PRE");
    Jsi_TreeWalk(st, mycall, NULL, JSI_TREE_ORDER_PRE);
    puts("POST");
    Jsi_TreeWalk(st, mycall, NULL, JSI_TREE_ORDER_POST);
    puts("LEVEL");
    Jsi_TreeWalk(st, mycall, NULL, JSI_TREE_ORDER_LEVEL);
    Jsi_TreeEntryDelete(hPtr2);
    puts("INDEL");
    Jsi_TreeWalk(st, mycall, NULL, 0);

    for (i=0; i<1000; i++) {
        snprintf(nbuf, sizeof(nbuf), "name%d", i);
        Jsi_TreeSet(st, nbuf,(void*)i);
    }
    Jsi_TreeWalk(st, mycall, NULL, 0);
    for (i=0; i<1000; i++) {
        Jsi_TreeEntryDelete(st->root);
    }
    puts("OK");
    Jsi_TreeWalk(st, mycall, NULL, 0);
    Jsi_TreeDelete(st);

}

int jsi_InitTree(Jsi_Interp *interp, int release)
{
    if (release) return JSI_OK;
    TreeTest(interp);
    return JSI_OK;
}

#else

Jsi_RC jsi_InitTree(Jsi_Interp *interp, int release)
{
    if (release) return JSI_OK;
    /* TODO: maintain hash table of trees created per interp? */
    return JSI_OK;
}
#endif
uint Jsi_TreeSize(Jsi_Tree *treePtr) { return treePtr->numEntries; }

Jsi_RC Jsi_TreeConf(Jsi_Tree *treePtr, Jsi_MapOpts *opts, bool set)
{
    if (set)
        treePtr->opts = *opts;
    else
        *opts = treePtr->opts;
    return JSI_OK;
}

#ifndef JSI_LITE_ONLY

static Jsi_Value *jsi_treeFmtKey(Jsi_MapEntry* h, struct Jsi_MapOpts *opts, int flags)
{
    Jsi_TreeEntry* hPtr = (Jsi_TreeEntry*)h;
    void *key = Jsi_TreeKeyGet(hPtr);
    if (opts->keyType == JSI_KEYS_ONEWORD)
        return Jsi_ValueNewNumber(opts->interp, (Jsi_Number)(intptr_t)key);
    char nbuf[100];
    snprintf(nbuf, sizeof(nbuf), "%p", key);
    return Jsi_ValueNewStringDup(opts->interp, nbuf);
}

Jsi_RC Jsi_TreeKeysDump(Jsi_Interp *interp, Jsi_Tree *tablePtr, Jsi_Value **ret, int flags) {
    char *key;
    int n = 0;
    Jsi_TreeEntry *hPtr;
    Jsi_TreeSearch search;
    Jsi_Obj *nobj;
    Jsi_MapFmtKeyProc* fmtKeyProc = tablePtr->opts.fmtKeyProc;
    
    if (!fmtKeyProc && tablePtr->keyType == JSI_KEYS_ONEWORD && flags!=JSI_KEYS_ONEWORD )
        fmtKeyProc = jsi_treeFmtKey;
        
    if (!fmtKeyProc && tablePtr->keyType >= JSI_KEYS_STRUCT_MINSIZE) 
        return Jsi_LogError("Can not dump struct tree");
    nobj = Jsi_ObjNew(interp);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    for (hPtr = Jsi_TreeSearchFirst(tablePtr, &search, flags, NULL);
        hPtr != NULL; hPtr = Jsi_TreeSearchNext(&search)) {
        key = (char*)Jsi_TreeKeyGet(hPtr);
        Jsi_Value *val;
        if (fmtKeyProc) {
            val = (*fmtKeyProc)((Jsi_MapEntry*)hPtr, &tablePtr->opts, flags);
            if (!val) {
                Jsi_LogError("key format failed");
                Jsi_ValueMakeUndef(interp, ret);
                return JSI_ERROR;
            }
        } else if (tablePtr->keyType == JSI_KEYS_ONEWORD)
            val = Jsi_ValueNewNumber(interp, (Jsi_Number)(uintptr_t)key);
        else
            val = Jsi_ValueNewStringKey(interp, key);
        Jsi_ObjArraySet(interp, nobj, val, n++);
        if (fmtKeyProc && val->refCnt>1) //TODO: Hmmm. for StructKey Jsi_OptionsDump() returns refCnt=1
            Jsi_DecrRefCount(interp, val);
    }
    Jsi_TreeSearchDone(&search);
    return JSI_OK;
}
#endif

