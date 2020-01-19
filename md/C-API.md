C-API
====
<div id="sectmenu"></div>
Jsi has a sizeable C-API, best documented by the header file [jsi.h](../src/jsi.h).

A portion of which can be used without the interpreter code by C programmers (Jsi-Lite).

CData
----
See [CData](CData.md).

DBQuery
----
See [DBQuery](DBQuery.md).


DString
----
DString provides dynamic string functionality via a struct
which uses the stack when strings are short:

    Jsi_DString d = {"Here is your score "};
    puts(Jsi_DSPrintf(&d, "%s: -> %d/%d", user, n, m));
    Jsi_DSFree(&d);

Since the above strings are under 200 characters, no malloc call is required.

    Jsi_DString d = {};
    Jsi_DSPrintf(&d , "%0300d", 1); // Malloc
    Jsi_DSSetLength(&d, 0);
    Jsi_DSPrintf(&d , "%0300d", 1); // No-malloc
    Jsi_DSFree(&d);
    Jsi_DSPrintf(&d , "%0300d", 1); // Malloc
    Jsi_DSFree(&d);

Space is discared with Jsi_DSFree, and can be reused with Jsi_DSSetLength:

You can also use Jsi_DSInit():

    Jsi_DString d;
    Jsi_DSInit(&d);
    Jsi_DSAppend(&d, "Some stuff", NULL);
    Jsi_DSAppendLen(&d, "!", 1);
    Jsi_DSFree(&d);

| Name            | Description                                                                                    |
|-----------------|------------------------------------------------------------------------------------------------|
| Jsi_DSAppend    | Append one or more string arguments (plus NULL sentinal).                                      |
| Jsi_DSAppendLen | Append a string of given length (or -1 for strlen).                                            |
| Jsi_DSFree      | Release allocated memory and sets variable back to re-initialized/empty.                       |
| Jsi_DSFreeDup   | Return malloced string, then calls Jsi_DSFree.                                                 |
| Jsi_DSInit      | Initialize the variable, ignoring current data therein.                                        |
| Jsi_DSLength    | Return the length.                                                                             |
| Jsi_DSPrintf    | Format output and append to DString. Returns string from the current printf.                   |
| Jsi_DSSet       | Same as Jsi_DSSetLength(dsPtr,0) plus Jsi_AppendLen.                                           |
| Jsi_DSSetLength | If &lt; current length truncates string. Else sets min allocated space. Return allocated size. |
| Jsi_DSValue     | Return string value.                                                                           |
| JSI_DSTRING_VAR | Macro that declares a large DString on the stack.                                              |


    char*   Jsi_DSAppend(Jsi_DString *dsPtr, const char *str, ...);
    char*   Jsi_DSAppendLen(Jsi_DString *dsPtr, const char *bytes, int length);
    void    Jsi_DSFree(Jsi_DString *dsPtr);
    char*   Jsi_DSFreeDup(Jsi_DString *dsPtr);
    void    Jsi_DSInit(Jsi_DString *dsPtr);
    uint    Jsi_DSLength(Jsi_DString *dsPtr);
    char*   Jsi_DSPrintf(Jsi_DString *dsPtr, const char *fmt, ...);
    char*   Jsi_DSSet(Jsi_DString *dsPtr, const char *str);
    uint    Jsi_DSSetLength(Jsi_DString *dsPtr, uint length);
    char*   Jsi_DSValue(Jsi_DString *dsPtr);
    #define JSI_DSTRING_VAR(varPtr,size) //...


### Jsi_DSAppend
Calls Jsi_DSAppendLen for each string value argument, passing in -1 for the length.
  Each string is assumed to be null terminated and the final argument must be a NULL.

  RETURNS: The string starting at the first appended character.

### Jsi_DSAppendLen
Append length bytes to the DString. If length is &lt; 0,
the value of strlen is used.  If required, the DString is realloced to
be large enough to contain bytes, plus an extra null byte that is added to the end.

RETURNS: The string starting at the first appended character.

### Jsi_DSFree
Frees any allocated space and sets the DString back to empty such that it is safe to exit the scope.
Or the DString may be reused (also see Jsi_DSSetLength).

### Jsi_DSFreeDup
Returns the malloced string value and resets the DString in the same way as Jsi_DSFree.
This just avoids the user having to do an extra malloc/free if the DString was already malloced.
It is then the responsibility of the caller to free the returned value.

RETURNS: The string that was contained in the DString.

### Jsi_DSPrintf
Perform printf style string formatting as directed by the fmt string.
Under the covers, this utilizes vsnprintf.

RETURNS: The string starting at the first appended character.

### Jsi_DSSet
Same as calling Jsi_DSSetLength(dsPtr,0) followed by Jsi_DSAppendLen(dsPtr,str).
Sets the DString to str without freeing any allocated space.

**Warning**: It is not safe to exit the scope without first calling Jsi_DSFree.


### Jsi_DSSetLength
Depending on dsPtr->len, truncates a string or sets the minimum allocated space.

- If length is &lt; 0, does nothing and just returns the current size allocation.
- if length is &lt; current length, the string is truncated.
- Otherwise, enforces the allocated space is at least length.

**Note**:
    This will not set dsPtr->len unless truncating.
    Also an extra byte is always added to the allocation,
    but this is not reported in the allocated length.

  RETURNS: The currently allocated size. ie. the size of the maximum string that
  will fit without a call to realloc.

### Jsi_DSValue
: Gets the current string value.

  RETURNS: The string dsPtr->str.

### `JSI_DSTRING_VAR`


### Large String Buffers
When working with larger strings, we may want to preallocate a large
string in order to avoid repeated calls to realloc() as the string grows.
The normal approach might be something like:

    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DSSetLength(&dStr, 50000);

Another alternative is to use the `JSI_DSTRING_VAR` macro, which avoids using malloc entirely.
`JSI_DSTRING_VAR` efficiently declares a Jsi_DString* pointing to an enlarged static DString upon the stack: eg:

    JSI_DSTRING_VAR(dsPtr, 50000);
    Jsi_DSPrintf(dsPtr, "%04999d", 1); // No malloc.


### C++ Strings

Consider C++ stringstream, which provides convenient dynamic string support with type safety.

    std::stringstream str;
    str << "ABC " << 123;
    puts(str.str().c_str());

The tradeoffs of stringstream are:

- Implicit memory allocation.
- Implicit code execution.
- Hard to inspect in gdb.
- Awkward to obtain the C string value.
- Not available in plain C-code.

DString provides familiar printf style formatting:

    Jsi_DString dstr = {};
    puts(Jsi_DSPrintf(&dstr, "ABC %d", 123));

Printf style modifiers can be significantly simpler than stringstream:

    Jsi_DSPrintf(&dstr, "%02d%-3d%04d", v1, v2, v3);
    str << std::setfill('0') << std::setw(2) << v1
        << std::setfill(' ') << std::setw(3) << std::left  << v2
        << std::setfill('0') << std::setw(4) << std::right << v3;


### Safety
The gcc compiler makes DString usage quite safe.

- It generates warnings for Jsi_DSPrintf argument mismatches.
- It warns when Jsi_DSAppend is missing NULL terminator.

There however are some gotchas to be aware of:

- DStrings greater than 200 bytes can not be assigned to one another.
- Failing to call Jsi_DSFree can leak memory.
- A DString has a maximum (compile time limit) of a 100 Meg.


JSON
----
Jsi implements a non-validating JSON parser.  That is, will accept JSON that
is not strictly compliant.


### High-Level


#### Jsi_JSONParse

The main function for parsing JSON to generate a Jsi_Value is:

    Jsi_RC Jsi_JSONParse(Jsi_Interp *interp, const char *js, Jsi_Value ret, int flags);

If the bit flags field does not contain JSI_JSON_STRICT, the parser will accept
non-standard, unquoted property names, as in:

    {file:"FF", line:NN}


#### Jsi_JSONParseFmt

This function is used primarily to reduce C-code.

    Jsi_RC Jsi_JSONParseFmt(Jsi_Interp *interp, Jsi_Value ret, const char *fmt, ...);


Consider a simple example where we wish to return a javascript object:

    {file:"File", line:1}

In C we would write something like:

    Jsi_Obj *nobj;
    Jsi_ValueMakeObject(interp, ret, nobj = Jsi_ObjectNew(interp));
    Jsi_ObjInsertFromValue( interp, nobj, Jsi_ValueNewStringKey(interp, "file"),
        Jsi_ValueNewStringKey(interp, file));
    Jsi_ObjInsertFromValue( interp, nobj, Jsi_ValueNewStringKey(interp, "line"),
       Jsi_ValueNewNumber(interp, line));

Alternatively, this can be simplified to:

    return Jsi_JSONParse(interp, ret, "{file:\"%s\", line:%d}", file, line);

The more deeply nested the object, the more code is simplified, eg:

    Jsi_JSONParseFmt(interp, ret, "{ a: [ {x:%d, y:%d}, {x:%d, y:[%d,%d,%d]}] }",a,b,c,d,e,f);

Due to the efficiency nature of low-level parsing, this adds very little extra overhead.

This approach can save a lot of code, particularly when writing commands
that return nested objects and/or arrays.
.

**Note**:
    Permissive mode is the default.  So there is no need to quote property names which in C rapidly becomes tedious:

    "{\"os\":\"%s\", \"platform\":\"%s\", \"hasThreads\":%s, \"pointerSize\":%d, "

**Warning**:
    The above works only for cases where data does not contain special JSON characters.


### Low-Level
For the low level,
parses JSON into an array of tokens in a highly stack efficient manner.
It uses a single
array of tokens for output so that for limited size JSON strings,
no memory gets allocated during the parse.
When memory does get allocated, it is only to resize the token array.


### Sub-Interps
Any data sent between sub-[interps](Interp.md) will first be converted to/from JSON.
This because all data objects are private to an interp.

Jsi-Lite
----

Jsi-Lite is a subset of the Jsi C source code which can be used without the script engine.


### Jsi_DString
[Jsi_DString](#DString) is available in Jsi-Lite.


### Jsi_Hash
This pages describes how to use Jsi_Hash.
Search for Jsi_Hash in [jsi.h](../jsi.h#Jsi_Hash) for details.

Hash provides simple hash table functionality.

    int isNew;
    Jsi_Hash *tbl = Jsi_HashNew(interp, JSI_KEYS_STRING, NULL);
    hPtr = Jsi_HashEntryNew(tbl, "foo", &isNew);
    Jsi_HashEntrySet(hPtr, 99);
    Jsi_HashSet(tbl, "bar", 100);
    Jsi_HashSearch search;
    for (hPtr = Jsi_HashEntryFirst(tbl, &search);
        hPtr != NULL; hPtr = Jsi_HashEntryNext(&search)) {
        key = Jsi_HashKeyGet(hPtr);
        int n = Jsi_HashValueGet(hPtr);
    }

There are plenty of examples using Hash in the Jsi source code.


### Jsi_Tree
The underlying data structure for objects in JSI is a tree Red-Black trees with invariant node
pointers: nodes are allocated using a single malloc, including space for the key.
This introduces a problem in that varying string keys can not be copied between nodes,
which is required when re-balancing the tree. Although tree supports swapping node positions
instead of keys, objects instead use a key of type STRINGPTR, a combination Hash table and and Tree,
which is fairly efficient because objects often share keys.



### Jsi_List
Jsi_List implements a double linked list.
Not heavily used. Included mainly for completeness.


### Jsi_Map
Jsi_Map encapsulates Hash/Tree/List.  Allows switching
underlying implementation by changing a single declaration.


### Example Code
We use Jsi-Lite as follows:

    #define JSI_LITE_ONLY
    #include "jsi.c"
    //Your code goes here.

Following is some demo code:

- a minimal demo of Jsi-Lite: [litedemo.c](../c-demos/litedemo.c).
- a demo of Jsi_List: [litedemo.c](../c-demos/listdemo.c).
- a more comprehensive database demo: [dbdemo.c](../c-demos/dbdemo.c).


Options
----
A Jsi_OptionSpec array provides details of fields in a struct. This information is used for
translating values to/from and from C struct fields, both for Javascript and
by the [Sqlite C-API](DBQuery.md).

A Jsi_OptionSpec array specifies for a struct field the
name, type, offset, help string, etc.  This specification is passed, along with a pointer
to the actual data,
to `Jsi_OptionProcess()` and `Jsi_OptionConf()`.


### Example Usage
The following example is a subset of the Jsi Sqlite command:

    enum { MUTEX_DEFAULT, MUTEX_NONE, MUTEX_FULL };
    static const char *mtxStrs[] = { "default", "none", "full", 0 };
    
    static Jsi_OptionSpec Options[] =
    {
        JSI_OPT(INT,    SqliteDb, debug,    .help="Set debugging level"),
        JSI_OPT(CUSTOM, SqliteDb, execOpts, .help="Default options for exec", .custom=&Jsi_OptSwitchSuboption, .data=ExecFmtOptions),
    
        JSI_OPT(INT,    SqliteDb, maxStmts, .help="Max cache size for compiled statements"),
        JSI_OPT(BOOL,   SqliteDb, readonly, .help="Database is readonly", .flags=JSI_OPT_INIT_ONLY ),
        JSI_OPT(VALUE,  SqliteDb, vfs,      .help="VFS to use", .flags=JSI_OPT_INIT_ONLY),
        JSI_OPT(CUSTOM, SqliteDb, mutex,    .help="Mutex type to use", .custom=&Jsi_OptSwitchEnum, .data=mtxStrs),
        JSI_OPT_END(SqliteDb)
    };
    
      //...
    if (Jsi_OptionsProcess(interp, Options, optObj, &opts, 0) < 0) {
         return JSI_ERROR;
    }
    if (opts.debug)
        puts("Sqlite created");


### Field Specification
Options are specified using the JSI_OPT() macro. The first 3 arguments are TYPE,STRUCT,NAME,
where the NAME is a field name in STRUCT.  Additional attributes can be specified using ".name=value" form.

<a name=opttypes></a>
#### Option Types

The option TYPE is one of:

| Name      | C-Type      | Description                                                                   |
|-----------|-------------|-------------------------------------------------------------------------------|
| BOOL      | bool        | Boolean (stored in a "char" variable).                                        |
| INT       | int         | An integer.                                                                   |
| UINT      | uint        | An unsigned integer.                                                          |
| LONG      | long        | An long integer.                                                              |
| ULONG     | ulong       | An long unsigned integer.                                                     |
| SHORT     | short       | An short integer.                                                             |
| USHORT    | ushort      | An short unsigned integer.                                                    |
| INT8      | int8_t      | An 8-bit integer.                                                             |
| INT16     | int16_t     | A 16-bit integer.                                                             |
| INT32     | int32_t     | A 32-bit integer.                                                             |
| INT64     | int64_t     | A 64-bit integer.                                                             |
| UINT8     | uint8_t     | An unsigned 8-bit integer.                                                    |
| UINT16    | uint16_t    | An unsigned 16-bit integer.                                                   |
| UINT32    | uint32_t    | An unsigned 32-bit integer.                                                   |
| UINT64    | uint64_t    | An unsigned 64-bit integer.                                                   |
| SIZE_T    | size_t      | An size_t integer.                                                            |
| INTPTR_T  | intptr_t    | An pointer sized integer.                                                     |
| UINTPTR_T | uintptr_t   | An pointer sized unsigned integer.                                            |
| FLOAT     | float       | A floating point.                                                             |
| DOUBLE    | double      | Double floating point.                                                        |
| LDOUBLE   | ldouble     | Long double floating point.                                                   |
| DSTRING   | Jsi_DString | A Jsi_DString value.                                                          |
| STRKEY    | const char* | A char* string key.                                                           |
| STRBUF    | char[]      | A fixed size char string buffer.                                              |
| VALUE     | Jsi_Value*  | A Jsi_Value.                                                                  |
| STRING    | Jsi_Value*  | A Jsi_Value referring to a string.                                            |
| VAR       | Jsi_Value*  | A Jsi_Value referring to a variable.                                          |
| OBJ       | Jsi_Value*  | A Jsi_Value referring to an object.                                           |
| ARRAY     | Jsi_Value*  | A Jsi_Value referring to an array.                                            |
| FUNC      | Jsi_Value*  | A Jsi_Value referring to a function.                                          |
| TIME_T    | time_t      | A date variable, milliseconds since 1970 stored in a time_t/long.             |
| TIME_D    | double      | A date variable, milliseconds since 1970 stored in a Jsi_Number/double.       |
| TIME_W    | int64_t     | A date variable, milliseconds since 1970 stored in a Jsi_Wide/64 bit integer. |
| CUSTOM    | void*       | Custom parsed value.                                                          |


#### Flags
The following flags are predefined for individual items in an OptionSpec:

| Name                   | Description                                    |
|------------------------|------------------------------------------------|
| `JSI_OPT_INIT_ONLY`    | Allow set only at init, disallow update/conf   |
| `JSI_OPT_NO_DUPVALUE`  | Values/Strings are not to be duped (dangerous) |
| `JSI_OPT_READ_ONLY`    | Value can never be set.                        |
| `JSI_OPT_DIRTY_FIELD`  | Used for DB update.                            |
| `JSI_OPT_IS_SPECIFIED` | User set the option.                           |
| `JSI_OPT_DB_IGNORE`    | Field is not to be used for DB.                |
| `JSI_OPT_CUST_NOCASE`  | Ignore case (eg. for ENUM and BITSET)          |
| `JSI_OPT_CUST_FLAG2`   | Reserved for custom flags                      |
| `JSI_OPT_CUST_FLAG2`   | Reserved for custom flags                      |
| `JSI_OPT_CUST_FLAG2`   | Reserved for custom flags                      |


#### Custom Types
Custom types provide parsing/formatting functions to interpret javascript data to/from a C struct-field.
Again, the best example is the Jsi source itself.

The following predefined custom types come builtin:

| Name                     | Description                    | .data      |
|--------------------------|--------------------------------|------------|
| Jsi_OptSwitchEnum        | Enum match                     | stringlist |
| Jsi_OptSwitchBitset      | Set of zero or more enum match | stringlist |
| Jsi_OptSwitchValueVerify | Provide callback to verify     | Callback   |
| Jsi_OptSwitchSuboption   | Recursive sub-option support   | sub-Option |

Refer to the Jsi source for example usage and declarations.


#### OptionSpec Fields
The following fields are defined in Jsi_OptionSpec:

| Field   | Type              | Description                                                            |
|---------|-------------------|------------------------------------------------------------------------|
| type    | enum              | Field type (from above)                                                |
| name    | char*             | Name of field                                                          |
| offset  | int               | Offset in array                                                        |
| size    | const char*       | Sizeof of field.                                                       |
| iniVal  | union             | Used for compile-time validation                                       |
| flags   | int               | Flags (from above)                                                     |
| custom  | Jsi_OptionCustom* | Custom parsing struct                                                  |
| data    | void*             | User data for custom options.                                          |
| help    | char*             | Short 1 line help string.                                              |
| info    | char*             | Longer command description: re-define JSI_DETAIL macro to compile-out. |
| init    | const char*       | Initial string value (used by info.cmds).                              |
| extName | const char*       | External name: used by the DB interface.                               |

The first five fields are implicitly set by the JSI_OPT() macro.
The JSI_END() macro also sets extName.

All others are available for setting by the user with .FIELD=VALUE notation.


### Parse Flags
The following flags are available for use with Jsi_OptionProcess/Jsi_OptionConf:

| Name                    | Description                                                 |
|-------------------------|-------------------------------------------------------------|
| `JSI_OPTS_IS_UPDATE`    | This is an update/conf (do not reset the specified flags)   |
| `JSI_OPTS_IGNORE_EXTRA` | Ignore extra members not found in spec                      |
| `JSI_OPTS_FORCE_STRICT` | Complain about unknown options, even when noStrict is used. |
| `JSI_OPTS_PREFIX`       | Allow matching unique prefix of object members.             |
| `JSI_OPTS_VERBOSE`      | Dump verbose options                                        |


### Compile-time Validation
Compile-time validation checks option values to ensure that stated type and the field
type actually match.  If they don't, the program will likely crash at runtime.
The implementation uses an implicit assignment to iniVal which should
generate a compiler warning if there is a type mismatch.

Here is an example:

    typedef struct {
        int val;
        //...
    } MyData;
    
    Jsi_OptionSpec opts[] = {
        JSI_OPT(STRING, MyData, val),
        JSI_OPT_END(    MyData)
    };

which should generate:

    test.c:7:6: warning: initialization from incompatible pointer type [enabled by default]
    test.c:7:6: warning: (near initialization for ‘opts[0].iniVal.ini_STRING’) [enabled by default]


### Is Specified
When a user provides a value for an option, the flag `JSI_OPT_IS_SPECIFIED` is set in the option flags.
Call Jsi_OptionChanged to determine if an option was specified.

**Note**:
    The Jsi header file [jsi.h](../jsi.h#Jsi_OptionSpec) and source are best consulted for complete examples.

**Warning**:
    There are known concurrency issues with this feature.
