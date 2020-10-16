Db-Query
====

Db-Query uses [CData](Extensions.md) to define data to exchange data between C-structs and Sqlite.

Supported are **SELECT, INSERT, UPDATE, REPLACE, DELETE** with:

- Automatic data conversions.
- Statement caching.
- Minimal C-code.
- Minimal heap/CPU use.
- Shortcut bind to all struct fields: "%s".
- 1-1 array/row mapping (via a "rowid" field).
- A **dirty** bit to limit write-back to modified rows.
- Efficient handling for most strings.
- Arrays of structs.
- Optional use of Javascript.


Usage
----
To start, declare a struct for inputs and outputs, eg:

``` js
typedef struct { int id; double max; /*...*/ } MyData;

// The struct is described with a [specification](Extensions.md):

static Jsi_OptionSpec MyOptions[] = {
    JSI_OPT(INT,        MyData, id,     .help="Int id", .userData="DEFAULT 0 CHECK(id>0)"),
    JSI_OPT(DOUBLE,     MyData, max,    .help="Max value"),
}
```

These are passed to Jsi_DbCarray:


``` js
int Jsi_DbCarray(Jsi_Db *jdb, Jsi_OptionSpec *spec, void *data, int numData, const char *query, Jsi_DbOpts *opts);

// For example, to retrieve the results of a query:

MyData mydata = {.id=1};
Jsi_DbCarray(jdb, spec, &mydata, 1, "SELECT %s FROM mytbl WHERE id = :id", NULL)
Jsi_DbCarray(jdb, spec, &mydata, 1, "INSERT INTO mytbl %s", NULL)
```

To incorporate Jsi_DbCarray into an application,
[download](https://jsish.org/jsi/download), extract jsi.c and include like so:


``` clike
#define JSI__SQLITE 1
#include <sqlite3.h>
#include "jsi.c"

int main(int argc, char *argv) {
  Jsi_Db *jdb = Jsi_DbNew(...);
  Jsi_DbCarray(jdb, ... );
}
```

- For a simple but complete Jsi_DbCarray example see [Jsi-Lite demo](https://jsish.org/jsi/file/c-demos/litedemo.c).
- For a complete working example see [https://jsish.org/jsi/file/c-demos/dbdemo.c]*.


Example
----
Given the following database-table:

``` js
CREATE TABLE mytable (
    name,
    id INT,
    max FLOAT,
    myTime TIMEINT,
    mark,
    markSet,
    desc
);

// and a corresponding struct definition:


typedef struct {
    char name[16];
    int id;
    double max;
    int64 myTime;
    int mark;
    int markSet;
    Jsi_DString desc;
    int64 rowid;
    char dirty;
} MyData;
```

Define a descriptor array of type [Jsi_OptionSpec](Extensions.md):


``` clike
static const char *markStrs[] = {"","A","B","C","D","F",NULL};

static Jsi_OptionSpec MyOptions[] = {
JSI_OPT(STRBUF, MyData, name, .help="Char buf",.userData="DEFAULT ''" ),
JSI_OPT(DSTRING,MyData,desc,.help="Description field"),
JSI_OPT(INT,MyData,id,.help="Int id",.userData="DEFAULT 0 CHECK(id>0)"),
JSI_OPT(DOUBLE,MyData,max,.help="Max value"),
JSI_OPT(DATETIME,MyData,myTime,.help="JS time int64_t",.userData="DEFAULT" ),
JSI_OPT(CUSTOM,MyData,mark,.help="",.custom=Jsi_Opt_SwitchEnum,.data=markStrs ),
JSI_OPT(CUSTOM,MyData,markSet,.help="A set",.custom=Jsi_Opt_SwitchBitset,.data=markStrs ),
JSI_OPT(WIDE,MyData,rowid,.help="DB rowid for update/insert; not stored in db",.flags=JSI_OPT_DB_ROWID"),
JSI_OPT(BOOL,MyData,isdirty,.help="Dirty flag: not stored in db",.flags=JSI_OPT_DB_DIRTY"),
JSI_OPT_END( MyData,.help="This is a struct for dbdemo")
};
```

Which we can then use to store/load data by calling Jsi_DbCarray():


``` js
Jsi_Db *jdb = Jsi_DbNew("~/mytables.db", 0);
MyData d = {"myname", 99, 9.0};
Jsi_DbCarray(jdb, MyOptions, &d, 1, "INSERT INTO mytable %s", NULL);
Jsi_DbCarray(jdb, MyOptions, &d, 1, "SELECT %s FROM mytable", NULL);
Jsi_DbCarray(jdb, MyOptions, &d, 1, "UPDATE mytable SET %s", NULL);
Jsi_DbCarray(jdb, MyOptions, &d, 1, "SELECT id,name FROM mytable WHERE rowid=:rowid", NULL);
```

The return value is either the number of rows loaded, modified or stored, or:

- -1 : an error occurs.
- -2 : the database was locked.

Ideally, a function wrapper is used to improve readability and type-checking:


``` clike
int db_MyData(MyData *data, const char *query) {
    return Jsi_DbCarray(jdb, MyOptions, data, 1, query, NULL);
}

db_MyData(&d, "INSERT INTO mytable %s")
db_MyData(&d, "UPDATE mytable SET %s")
```

Tables
----
The above example operated on a single row.
Multiple rows of data can be handled in a couple of ways.


### Single Struct
The brute-force way to process a table of data is to iterate over a single struct:


``` clike
MyData mydata = {.id=99, .max=100.0, .mark=MARK_A, .markSet=6};
mydata.myTime = time(NULL)*1000LL;
strcpy(mydata.name, "maryjane");
Jsi_DSSet(&mydata.desc, "Some stuff");

Jsi_DbCarray(jdb, 0, 0, 0, ";BEGIN", NULL);
for (i=0, n=1; i<10 && n==1; i++) {
    mydata.id++;
    mydata.max--;
    n = Jsi_DbCarray(jdb, MyOptions, &mydata, 1, "INSERT INTO mytable %s", NULL);
}
Jsi_DbCarray(jdb, 0, 0, 0, ";COMMIT", NULL);
```

While this works, it requires a lot of C-code.


### Arrays of Structs
A simpler way to process a table is via an array of structs, with a single call.
Three forms of array are supported:

#### Static Arrays

A static array has all storage pre-allocated by the user:


``` clike
MyData mydatas[10];
int cnt = Jsi_DbCarray(jdb, MyOptions, mydatas, 10, "SELECT %s FROM mytable", NULL);

for (i=0; i < cnt; i++)
    mydatas[i].id += i;
n = Jsi_DbCarray(jdb, MyOptions, mydatas, cnt, "UPDATE mytable SET %s WHERE rowid = :rowid", NULL);
```

This example does 2 things:
- SELECT to load up to 10 rows into the array.
- UPDATE to iteratively stores the rows after modification.

**Note**:
    The "BEGIN/COMMIT" is implicitly performed for updates and inserts.

#### Jsi_DbOpts Flags

Following are the option-flags for Jsi_Db commands:

``` clike
typedef struct {
    bool ptrs:1;        /* Data is a fixed length array of pointers to (allocated) structs. */
    bool ptr_to_ptrs:1; /* Address of data array is passed, and this is resized to fit results. */
    bool mem_clear:1;   /* Before a query previously used data is reset to empty (eg. DStrings). */
    bool mem_free:1;    /* Reset as per mem_clear, then free data items (query may be empty). */
    bool dirty_only:1;  /* Used by sqlite UPDATE/INSERT/REPLACE. */
    bool no_begin_commit:1;/* Do not wrap UPDATE in BEGIN/COMMIT. */
    bool no_cache:1;    /* Do not cache statement. */
    bool no_static:1;   /* Do not bind text with SQLITE_STATIC. */
    uint reserved:24;       /* Reserved for future use. */
} Jsi_DbOpts;
```

These are used in the following calls.

#### Static Array of Pointers

To use an array of pointers we pass the ptrs flag.

``` clike
static MyData *mdPtr[10] = {};
Jsi_DbOpts opts = {.ptrs=1};
n = Jsi_DbCarray(jdb, MyOptions, mdPtr, 10, "SELECT %s FROM mytable", &opts);
```

In this mode, memory is allocated on demand (up to the maximum size).

**Note**:
    The data argument is now an array of pointers to structs (void *).

#### Dynamic Array of Pointers

For fully dynamic allocation, both of the array and the struct pointers,
we pass the "ptr_to_ptrs" flag:


``` clike
MyData dynPtr = NULL;
Jsi_DbOpts opts = {.ptr_to_ptrs=1};
n = Jsi_DbCarray(jdb, MyOptions, &dynPtr, 0, "SELECT %s FROM mytable WHERE rowid < 5", &opts);
n = Jsi_DbCarray(jdb, MyOptions, &dynPtr, n, "SELECT %s FROM mytable LIMIT 1000", &opts);
```

This mode will manage an extra NULL pointer at the end of the array
to make the current length detectable
(ie. when the "num" parameter is 0).

**Note**:
    The data argument has changed again; it is now a pointer to an array of pointers (void *).


### Function Wrappers
The above examples have several drawbacks:

- the "data" argument is not type-checked.
- calls to Jsi_DbCarray takes a lot of arguments (6).

The first problem is easily demonstrated:


``` clike
Jsi_DbCarray(jdb, MyOptions, "oops, not a struct", n, "SELECT %s FROM mytable", NULL);  // Invalid, but no compiler warning!
```

The simplest solution defines wrapper functions:

    
``` clike
int My_Stat(MyData *data, int num, const char *query, Jsi_DbOpts *opts) {
    return Jsi_DbCarray(jdbPtr, MyOptions, data, num, query, opts);
}

static int My_Semi(MyData data, int num, const char *query, Jsi_DbOpts *opts) {
    Jsi_DbOpts o = {};
    if (opts)
        o = *opts;
    o.ptrs = 1;
    return Jsi_DbCarray(jdbPtr, MyOptions, data, 0, query, &o);
}

static int My_Dyn(MyData data, int num, const char *query, Jsi_DbOpts *opts) {
    Jsi_DbOpts o = {};
    if (opts)
        o = *opts;
    o.ptr_to_ptrs = 1;
    return Jsi_DbCarray(jdbPtr, MyOptions, data, 0, query, &o);
}

My_Stat(mydatas, n, "SELECT %s FROM mytable;", NULL);
My_Semi(mdPtr,   n, "SELECT %s FROM mytable;", NULL);
My_Dyn (&dynPtr, n, "SELECT %s FROM mytable;", NULL);
```

In addition to the adding type checking, the resulting code is simpler.

### Multi-Struct Bind
Multiple structs can be bound to,
using a different bind character for each:


``` clike
int My_Bind(MyData *data1, int num, MyData *data2, const char *query)
{
    Jsi_CarrayBind mybinds[] = {
        { ':', MyOptions, data1, num }, // Input/output array.
        { '$', MyOptions, data2, 1 },   // Input single struct.
        {}
    };
    Jsi_DbOpts opts = {.ptrs=1, .binds=mybinds};
    n = Jsi_DbCarray(jdb, NULL, NULL, 0, query, &opts);
}

mydata.max = -1;
n = My_Bind(mdPtr, num, mydata, "SELECT %s FROM mytable WHERE rowid=:rowid AND max=$max");
```

This causes a single bind of "$max" to
*mydata*, then repeated array-binds to ":rowid",
allowing us to avoid input/output data collisions.


Spec Fields
----
A spec is an array used to describe all or part of an struct using [options](Extensions.md).


### Supported Types
For database access, spec option types are limited to:

Name    | C-Type     | Description
--------|------------|--------------------------
BOOL    | int        | Boolean (uses a "char" variable).
INT     | int        | An integer.
WIDE    | Jsi_Wide   | A 64-bit integer (Jsi_Wide).
DOUBLE  | Jsi_Number | Double floating point.
DSTRING | Jsi_DString| A Jsi_DString value.
STRKEY  | char*      | A char* string key.
STRBUF  | char[]     | A fixed size char string buffer.
STRING  | Jsi_String | A Jsi_Value referring to a string (when not using JSI_LITE_ONLY)
DATETIME| Jsi_Wide   | A date variable, milliseconds since 1970 stored in a 64 bit integer.
CUSTOM  |            | Custom types, including Enum and Bitmap.


### Custom
The custom type supports defining parametrized handlers using the ".custom" and ".data" fields.
Several predefined handlers are available:

#### Custom Enum

It is a common database practice to store numeric values in one table, and then use a foreign key to reference
the string value from another table.  This is certainly a useful abstraction,
but it does add complexity to the schema.
It also typically results in overhead from the use of joins, views and sub-selects.

The alternative is to store the string in the table.  But this requires conversion code
when we wish to use an enum in C.

Now consider the "marks" field from above:


``` clike
typedef enum { MARK_NONE, MARK_A, MARK_B, MARK_C, MARK_D, MARK_F } MarkType;
...
static const char *markStrs[] = {"","A","B","C","D","F",NULL};
...
    JSI_OPT(CUSTOM,     MyData, mark,   .help="Marks", .custom=Jsi_Opt_SwitchEnum, .data=markStrs ),
...
```

The definition ensures that the *marks* value is stored as integer in memory, and as string
in the database:  No manual conversion is required.

The JSI_OPT_NOCASE flag can be used ignore case in the database string value.

#### Custom Bitset

The Jsi_Opt_SwitchBitset option provides access multiple bits in one integer field.
This works similar to the above enum, except that the C stored values are bits set in an integer:


 ``` clike
JSI_OPT(CUSTOM,     MyData, markSet,   .help="Marks set", .custom=Jsi_Opt_SwitchBitset, .data=markStrs ),
```

But in the database they are stored as a list of string attributes:

``` clike
# SELECT markSet FROM mytable;
"A B"
"B C D"
```

Like enum, Jsi automatically provides the translation to/from strings.

The JSI_OPT_NOCASE flag can be used ignore case in the database string value.


### Field/Column Mapping
The .extName field is used to map a C field name to a different database column name:


``` clike
JSI_OPT(STRBUF,     MyData, name,   .extName=struct ),
```

ie. in this example, struct is a reserved word in C.


### NULL Values
For SELECT, the following Sqlite C-API rules apply for NULL sqlite_column values:

- DSTRING and STRBUF: an empty string.
- STRKEY: a NULL pointer.
- Otherwise, the value is "0".


### Dirty Field
A dirty field is used to limit which rows get stored, which is
substantially faster than updating every row in the table.
It is either a BOOL or INT field using the "dirty_only" option flag.


``` clike
JSI_OPT(BOOL,       MyData, isdirty, .help="Dirty flag: not stored in db", .flags=JSI_OPT_DB_DIRTY),
```

The call must be made with the "dirty_only" flag:


``` clike
for (i=1; i<=3; i++) {
    mydatas[i].isdirty = 1;
    mydatas[i].id += 100*i;
}
Jsi_DbOpts opts = {.dirty_only=1};
n = QueryMyStat(mydatas, cnt, "UPDATE mytable SET %s WHERE rowid = :rowid", &opts);
```

**Note**:
    Unless an error occurs, dirty fields are cleared by the call.


### Rowid Field
A field for storing the "rowid" is indicated by a "JSI_OPT_DB_ROWID" option flag.


``` clike
JSI_OPT(WIDE,       MyData, rowid,  .help="DB rowid for update/insert; not stored in db", .flags=JSI_OPT_DB_ROWID),
```

The field will not be stored back to the database, but will be loaded during a SELECT, for use
in query bindings to enforce a 1-1 mapping with the database, eg.


``` clike
QueryMyStat(mydatas, cnt, "UPDATE mytable SET %s WHERE rowid == :rowid", 0);
```


Javascript
----
Javascript is available when "jsi.c" is not compiled with JSI_LITE_ONLY,
and the appropriate initialization used.


### Javascript Initialization
The following shows how to initialize Jsi to have full access to Javascript and the
[database scripting API](Sqlite.md).


``` clike
Jsi_EvalString(interp, "var mydb = "new Sqlite"('~/mytest.db');", 0);
Jsi_Db *jdb = Jsi_UserObjDataFromVar(interp, "mydb");
sqlite3 *db = Jsi_DbHandle(interp, jdb);
```


### The Cdata Command
Javascript can access data arrays created in C-code using
the [Cdata](Extensions.md) command.

The first step is to use Jsi_CarrayRegister() making mdPtr available as mydata.


``` clike
Jsi_DbOpts opts = {.ptrs=1};
Jsi_CarrayRegister(interp, mydata, MyOptions, mdPtr, num, &opts);
```

Then we can index data array elements with:

``` js
Cdata.get( 'mydata', 0, 'max' );
Cdata.set( 'mydata', 1, {'max':99} );
```

### Queries With Cdata
Javascript queries can use "Cdata" targets to load and store data:

``` clike
db = new Sqlite('~/mytable.db');

size = db.query('SELECT %s FROM mytable', {Cdata:'mydata'});
for (i = 0; i < size; i++) {
    max = Cdata.get('mydata', i, 'max');
    max += i*100;
    Cdata.set('mydata', i, {max:max});
}
db.query('UPDATE %s FROM mytable', {Cdata:'mydata'});

Cdata.get('mydata', 0);
Cdata.size('mydata');      // Get array allocated size.
Cdata.info('mydata');      // Struct info.
```

### Schema Generation
The Javascript command "Cdata.schema()" returns the schema for a Cdata definition,
for example to create a table:

``` js
db.query("CREATE TABLE newtable(" + Cdata.schema('mydata') + ")";

// which can then be used to access data:

db.query('INSERT %s INTO newtable', {Cdata:'mydata'});
db.query('SELECT %s FROM newtable', {Cdata:'mydata'});
```

The schema for the above would look something like:

``` bash
CREATE TABLE newtable(
-- 'MyData': This is a struct for dbdemo
  name "TEXT DEFAULT ''" -- Fixed size char buf
 ,desc "TEXT" -- Description field of arbitrary length
 ,id "INT DEFAULT 0 CHECK(id>0)" -- Int id
 ,max "FLOAT" -- Max value
 ,myTime "TIMEINT DEFAULT"(round((julianday('now') - 2440587.5)86400.0)) -- A unix/javascript time field in milliseconds (64 bits)
 ,mark "TEXT" -- Marks
 ,markSet "TEXT" -- A set
-- MD5: bc2a7cfc68725e60396dd2a2a4113f75
);
```

The schema is generated by appending:

- The column name from the "extName" or the "name" field.
- The generated type.
- The "userData" field (eg. "DEFAULT 1 CHECK (id>0)")
- A DATETIME field with a .userData="DEFAULT" will init to the current time.
- Lastly, the "help" field as a comment (ie. after a "-- " string).

The schema is completed using fields from JSI_OPT_END:

- "help" is prepended as a comment.
- "userData" is appended (eg. ", FORIEGN KEY(id) REFERENCES tkey(tid)").
- And a calculated MD5-comment.


### Schema Checking
One advantage of using generated schemas is that it provides a way to deal with data changes.

The calculated MD5 value (which ignores ".help" data) can be used in
tracking database compatibility as in the following example:

``` js
var md5 = "Cdata.schemaMd5"(*'mydata'*)
var schema = "db.onecolumn"("SELECT sql FROM sqlite_master WHERE name='newtable' AND type=='table'");
if (!schema.match(*'MD5: '*+md5))
    FixUpData(*'mydata','newtable'*); // Fixup function: eg. export, recreate table and import data.
```

### Configuration
Option configuration is available thus:

``` js
var mydb = "new Sqlite"(*'~/mytest.db'*, {maxStmts:100});
mydb.conf({maxStmts:1000});
```


Performance
----
Performance of Jsi_DbCarray() should be similar to that of hand generated C-code.
Heap memory requests are avoided where possible.
Except for Sqlite internal memory requests, heap allocation occurs only when:

- A JSI_DString field exceeds 200 bytes.
- A query string exceeds 2000 bytes (including generated argument lists).
- A query is first added to the cache.
- ptr* flags are used.


Some overhead arises from using sqlite3_bind_parameter_name(),
and string concatenation which is used in binding list creation.
However, being stack orientated together with caching compiled queries
gives reasonably good performance.

Following is output from the million row benchmark included in the "dbdemo" program:

``` bash
./c-demos/dbdemo -benchmark

==>

-1.000000
TESTING 1000000 ROWS
INIT C:    0.198 secs
   6.704 sec,   149164 rows/sec    INSERT 1000000 ROWS
   2.883 sec,   346860 rows/sec    SELECT 1000000 ROWS
  10.029 sec,    99710 rows/sec    UPDATE 1000000 ROWS, 1 FIELD
  10.754 sec,    92988 rows/sec    UPDATE 1000000 ROWS, ALL FIELDS
   4.381 sec,    22825 rows/sec    UPDATE 100000 DIRTY ROWS
   1.412 sec,      708 rows/sec    UPDATE 1000 DIRTY ROWS
   0.272 sec,       36 rows/sec    UPDATE 10 DIRTY ROWS
```

The last 3 use dirty row updates to demonstrate keeping a database in sync
with minimal overhead.


Miscellaneous
----
### BEGIN/COMMIT
Normally, any write query with size greater than one will implicilty add BEGIN/COMMITs.
The *JSI_DB_NO_BEGINCOMMIT* flag can be used to disable this behaviour.

Also, the "BEGIN" or "COMMIT" string can be overridden at compile time like so:

``` clike
#define JSI_DBQUERY_BEGIN_STR "BEGIN TRANSACTION"
#define JSI_DBQUERY_COMMIT_STR "COMMIT"
#include "jsi.c"
```

### Error Handling
A custom error handler can be invoked upon error using:

``` clike
#define JSI_DBQUERY_ERRORCMD MyErrHandler
#include "jsi.c"

int MyErrHandler(Jsi_Db *jdb, "Jsi_OptionSpec" *specs, void *data, int numData, const char *query, int flags, int rc) {
   printf("Error in query: %s\n", query);
   return rc;
}
```


Sqlite-C: Traditional
----
The standard Sqlite has an easy to use C-API with two groups of functions:

- "sqlite3_bind_*()"
- "sqlite3_column_*()"

which are employed as follows:

``` clike
stmt = sqlite3_prepare(db, "SELECT a,b FROM mytbl WHERE a=?1 AND b=?2", ...)
sqlite_bind_int(stmt, 1, mydata.b);

sqlite3_step(stmt);
mydata.a = sqlite3_column_int(stmt, 2);
```

While this approach is trivial with small schemas,
code validation gets to be an issue as database complexity increases because:

- There is no simple *programmatic* way to enforce bind/column indexes will match a query.
- Type mis-matches (between database and C fields) can easily occur.
- Correct handling of Sqlite return codes gets increasingly complex.
- Code maintainence increases proportionly with the number of tables/columns/queries.

For example, here is an except from real world code:


``` clike
stmt = sqlite3_prepare(db, "INSERT INTO descriptions VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20);");
```

Handling bindings reliably for such queries becomes increasingly tedious.

<!-- meta:{"file":{"index":1002 }} -->
