# Misc

[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

## Options

Options are parameters handled by builtin commands.
These get passed in an object:

``` js
var db = new Sqlite('/tmp/testsql.db',{maxStmts:1000, readonly:true});
```

Usually there is a conf() method providing access to options after creation:

``` js
Interp.conf(); // Dump all options
Interp.conf('strict'); // Get one option
Interp.conf({strict:false, maxDepth:99}); // Set one or more options.
```

**Note**:
    Some options may be readOnly. And an option that is initOnly
    may only be set at object creation time.

    Internally, option parsing is implemented via [C Options](Extensions.md#options).



## Introspection

There are several levels of introspection built-in to Jsi.
One is displayed when calling an invalid method:

``` js
Array.xxx();
error: 'Array' sub-commands are: concat fill filter forEach indexOf join lastIndexOf map pop push reverse shift sizeOf slice some sort splice unshift.    (at or near "xxx")
```

Another, is querying with the
[Info](Reference.md#Info) command:

``` js
Info.platform();
{ crc:7, hasThreads:true, intSize:4, isBigEndian:false, numberSize:8, os:"linux", platform:"unix", pointerSize:8, timeSize:8, wideSize:8 }

Info.cmds();
[  "Array", "Boolean", "File", "FileIO", "Function", "Interp", "Info", "JSON", "Math", "Number",
   "Object", "RegExp", "Signal", "Sqlite", "String", "Sys", "Websocket", "Zvfs", "assert", "clearInterval",
   "console", "decodeURI", "encodeURI", "exit", "source",
   "isFinite", "isNaN", "load", "parseFloat", "parseInt", "puts", "quote",
   "setInterval", "setTimeout", "format" ]

Info.cmds('Array.*');
[ "concat", "fill", "filter", "forEach", "indexOf", "join", "lastIndexOf", "map", "pop", "push",
  "reverse", "shift", "sizeOf", "slice", "some", "sort", "splice", "unshift" ]
    
Info.named();
[ "Socket", "Channel", "MySql", "WebSocket", "Sqlite", "Interp" ]

var ii = new Interp();
Info.named('Interp');
[ "#Interp_1" ]
```

and so on.


## Syntax

The following syntax is implemented by Jsi (see [Reference](Reference.md) for commands):

```
continue [IDENT] ;
break [IDENT] ;
debugger ;
delete IDENT ;
do { STMTS; } while( EXPR ) ;
for ([var] IDENT = EXPR; EXPR; EXPR) { STMTS; }
for ([var] IDENT in EXPR) { STMTS; }
for ([var] IDENT of EXPR) { STMTS; }
IDENT in OBJECT
function [IDENT] ([IDENT, IDENT, ...]) { STMTS; }
function [IDENT] ([IDENT:TYPE[=PRIMATIVE], IDENT:TYPE[=PRIMATIVE], ...]):TYPE { STMTS; }
([IDENT, IDENT, ...]) => EXPR
([IDENT, IDENT, ...]) => { STMTS; }
if (EXPR) { STMTS; } [ else { STMTS; } ]
IDENT instanceof IDENT ;
[new] FUNC( ARGS ) ;
return [EXPR] ;
switch (EXPR) { case EXPR: STMTS; [break;] case EXPR: STMTS; [break;]  ... [default EXPR;] }
throw EXPR ;
try { EXPR; } catch(IDENT) { STMTS; } [finally { STMTS; }]
typeof EXPR ;
var IDENT [ = EXPR ] [, ...] ;
let IDENT [ = EXPR ] [, ...] ;
const IDENT [ = EXPR ] [, ...] ;
with ( EXPR ) { STMTS; }
```

- Square brackets indicate optional.
- Curley braces are just for illustrative purposes and except for function and switch are not required.


### Expressions
Expressions take the usual form:

```
IDENT*.*IDENT
IDENT*[*EXPR]
IDENT*(*ARGS)
(EXPR)
EXPR ? STMTS :  STMTS
STMTS , STMTS [, ...]
EXPR OP EXPR
```

where OP is one of the binary operators +, -, *, /, etc.


### Terminals
| Name      | Description                                                                       |
|-----------|-----------------------------------------------------------------------------------|
| ARGS      | Zero or more comma-seperated arguments                                            |
| EXPR      | An expression (see below)                                                         |
| FUNC      | A function value                                                                  |
| IDENT     | Is an valid identifier                                                            |
| PRIMITIVE | A primitive value acceptable as an [argument type](Functions.md).           |
| STMTS     | Is zero or more statements                                                        |
| TYPE      | A type value acceptable as defaults                                               |


### Keywords
Keywords can be displayed using Info.keywords():

``` js
["...", "any", "arguments", "array", "boolean", "break", "case", "catch",
"continue", "debugger", "default", "delete", "do", "else", "false",
"finally", "for", "function", "if", "in", "instanceof", "new", "null",
"number", "object", "of", "regexp", "return", "string", "switch",
"this", "throw", "true", "try", "typeof", "undefined", "userobj", "var",
"void", "while", "with"]
```

## Language Comparisons


Following is a feature comparison of various languages with Jsi.

| Feature          | Jsi     | NodeJs | Tcl | Lua   | Perl | Python |
|------------------|---------|--------|-----|-------|------|--------|
| Standard         | ES 5.2+ | ES 6+  |     |       |      |        |
| Implemention     | C/C++   | C++    | C   | C/C++ | C    | C      |
| C++ Compatible   | Y       | Y      |     |       |      |        |
| Embeddable       | Y       | N      | Y   | Y     | Y    | Y      |
| C API            | Y       | N      | Y   | Y     | Y    | Y      |
| Non-Script API   | Y       |        |     |       |      |        |
| Standalone       | Y       |        | Y   | Y     |      |        |
| Type Checking    | Y       |        |     |       |      |        |
| Sub-Interps      | Y       |        |     |       |      |        |
| Introspection    | Y       |        |     |       |      |        |
| Error Navigation | Y       |        |     |       |      |        |
| Logging          | Y       |        |     |       |      |        |
| Builtin Debugger | Y       | Y      |     |       | Y    |        |
| Debugger GUI     | Y       |        |     |       |      |        |
| Web Ready        | Y       |        |     |       |      |        |
| Modular Apps     | Y       |        |     |       |      |        |
| Shrink Wrap      | Y       |        | Y   |       |      |        |
| Applications     | Y       |        |     |       |      |        |
| Self Configure   | Y       |        |     |       |      |        |

These comparisons are of software out-of-the-box.

**Note**::
    As of version 5.3.4, Lua also supports native C++.


## Design and Origin

Jsi is a Javascript interpreter written in C.
Additional functionality, implemented with scripts is bundled as "jsish".

Jsi is a byte-code oriented interpreter originally designed for interfacing-with, and embedding into C.

This makes it very different from [Node-js](https://nodejs.org) which is a compiler written in **C++**,
and which is not designed to be embedded.

Jsi is **C**-centric whereas Node is **JS**-centric.  Meaning that with Jsi, the locus of
control can resides in C-code.

Although Jsi was originally was based off source from [quad-wheel](https://code.google.com/archive/p/quad-wheel),
it is now internally modelled after [Tcl](https://www.tcl.tk/doc/scripting.html).


Extensions include:

- Functions parameters may have [types and defaults](Functions.md).
- [Testing](Testing.md) support that leverages type-checking.
- Select features from newer versions of the standard (eg. Array **of** and **forEach**).
- Limited basic arrow functions: no default values, and single parameters may not be braced.
- Non-standard object functions such as **merge** are available, eg. **o = o1.merge({a:1,b:2});**

## Goals

Following are principle goals  Jsi:

- Support embedded development using plain **C** (C99).
- But should also be compilable by **native GNU g++**, without use of *"extern C"*.
- Have as few dependencies as possible.
- Be generally free of value/object/memory leaks (verified with -fsanitize).
- Provide amalgamated source for simplified [application integration](Building.md#Embedding) .
- Low-level C-functions available in a **C-only** [Lite](#jsi-lite) version.
- Come with a [Debugger](Debug.md).
- Support Web applications, particularly with database and websockets.
- Support [standalone applications](Builtins.md#zvfs) via self-mounting .zip.
- [Package](Start.md) and extension support.

Compiling as C++ is supported, mostly for integrity checks.

**Note**:
    C-integration is the main priority in Jsi, not speed of script execution.


## Rational

- Desktop applications are held hostage by their user interface, be it QT, GTK, IOS or .NET.
- Increasingly web browsers are becoming the GUI, usually over the Internet.
- Moderately complex applications often end up requiring some form of script support, eg. Lua.
- If an application employs a Web GUI, a script language already is being used: Javascript.
- Time, energy and resources can be saved by using the same language on both ends.
- In fact, the same scripts can even be run in both the browser and the app.
- JSON provides seamless data interchange, thus avoiding data structure compatibility issues.


## Packaging

A [deploy](Deploy.md) is a zip/sqlar archive or fossil
repository containing one or more applications, which Jsi can mount and execute.

For example, this <a href="https://jsish.org/App10/Ledger">Ledger</a> demo is
served out as follows
using <a href="https://jsish.org/jsi-app">jsi-app</a> fossil
(via an Nginx reverse proxy).


But deploying any application inevitably entails dealing with version dependencies.
Jsi handles this in a novel way by mounting **tagged** releases from a fossil repository.
If an application restart is set to automatically update the repository,
it ensures the latest supported release always gets run.
New code may be committed by developers at any time,
but only tagged releases will be used.

## Security

As Jsi is self contained, running it standalone in a chroot-jail needs only
a few files from /dev and /etc.   For example, this [Ledger](https://jsish.org/App01/Ledger)
demo is run in an unprivileged chroot containing a single executable (*jsish*),
with non-data files made immutable with **chattr**.
And if this is not secure enough, Jsi offers [Safe-mode](Interp.md#safe-mode).

In addition, serving content from a zip or fossil repository adds
another abstraction layer, making it that much harder to corrupt or hijack content.

An advantage of the chroot approach as compared with something like containers, is
that far less disk space and system memory is required.
Jsi and fossil together total around 10Meg of disk. Using hard links you
could create hundreds of chrooted apps with little-to-no additional disk space.
But more important is the 
reduction in [Attack Surface](https://en.wikipedia.org/wiki/Attack_surface).
There is far less code involved and far less complexity.

## Web-Serve

### Nginx

Although Jsi can serve web content directly, on the Internet
it is more common to reverse proxy to a web-server via localhost:

Following config is for setting up jsish as a reverse proxy under Nginx

    location /App00/Ledger { proxy_pass http://localhost:8800; include proxy_params; }

And jsish might be invoked as:

    jsish -a jsi-app.fossil Ledger -port 8800 -urlPrefix /App00/Ledger -noGui true

Or run via chroot/su.


### Chroot Setup

Given it's small size, jsish is well suited for chroot deployments.

On the jsish.org site the user **jsiusr00** was created with directory contents:

``` bash
ls -RF jsiusr00

jsiusr00:
bin/  dev/  etc/  ledgerjs.db  tmp/  usr/

jsiusr00/bin:
fossil*  jsish*

jsiusr00/dev:
null  random  urandom

jsiusr00/etc:
hosts  resolv.conf

jsiusr00/tmp:

jsiusr00/usr:
jsi-app.fossil  jsi-app.sqlar  jsi-app.zip  ledgerjs.db
```

To scale this up while saving space, multiple users "jsiusrNN" are created with readonly files hard-linked to "jsiusr00".

Finally "chattr +i" is used make them immutable.

Thus incremental size for each additional
user is really only the data file "ledgerjs.db".

``` bash
 du -s jsiusr*
 12468   jsiusr00
 980 jsiusr01
 960 jsiusr02
 960 jsiusr03
 ...
 1224    jsiusr10
 960 jsiuser11
 ...
 960 jsiuser19
```

All previous directories have no shell, so with the addition of quotas and ulimits we end up with a
deployment that is simple but secure.

**NOTE:** The slight bump in "jsiusr10" is due to the addition of "sh", to allow execing fossil in a chroot.



### Wget

To download a file from the web we can use:

``` bash
jsish -w -O jsi-app.zip http://jsish.org/jsi-app/zip
```

### Websrv

The builtin web server can open it in a
local browser and serve out html to it, from the command-line:

``` bash
jsish -W index.html
```

and programmatically:

``` js
    Jsi_Websrv('index.html');
    update(-1);
```


## Packages

### pkgDirs

Packages are searched for are specified by pkgDirs, which can be set pkgDirs using:

``` js
Interp.conf({pkgDirs:['/usr/local/lib', '/usr/share']});
```

When a package is not found in pkgDirs, the executable and main script directories are checked.

**Note**: Sub-interps inherit pkgDirs from the parent.

### Auto-Load
Because it is sometimes desirable to defer loading code until first invocation,
Jsi provides a mechanism
using  member values in the object Jsi_Auto to evaluate.
For example, given the file myfunc.js:

``` js
provide('myfunc');
function double(n) { return n*2; };
function half(n)   { return n/2; };

// We can arrange for dynamic sourcing with:
Jsi_Auto.double =
Jsi_Auto.half   = 'require("myfunc");';

double(4); // Load is triggered!
half(4);

// This also works for object commands:
Jsi_Auto.MySql = "require('MySql')";
//...
var db = new MySql({user:'root', database:'jsitest'});
```

### Auto-Files

Jsi_Auto will demand-load files by
traversing the list **Interp.autoFiles**, eg:

``` js
Interp.conf('autoFiles');
[ "/zvfs/lib/autoload.jsi" ]
```

In addition, if an application is invoked from a **.zip** file, it's **autoload.jsi** file is appended.


#### Object Functions
Autoloading non-constructor object functions can use the following approach
(the example ignores the fact that JSON is not actually loadable):

``` js
var JSON = {
   parse:     function() { require('JSON'); return JSON.parse.apply(this,arguments); },
   stringify: function() { require('JSON'); return JSON.stringify.apply(this,arguments); }
   //...
}
JSON.parse(str); // Load is triggered!
```

Although that works, it suffers from one problem: the first call will not type-check functions.

We could fix this shortcoming with:

``` js
var JSON = {
    check: function(str:string, strict:boolean=false):boolean { require('JSON'); return JSON.check.apply(this,arguments); },
    //...
};
```

But correctly transcribing function signatures is more complicated.

Fortunately there is a helper:
:  function Jsi_AutoMake(objName:string):string

This can be used to generate the auto file:

    File.write( Jsi_AutoMake('JSON'), 'autoload.jsi');

The file autoload.jsi now contains calls and load and type-check correctly:

``` js
var JSON = {
    check: function(str:string, strict:boolean=true):boolean { require('JSON'); return JSON.check.apply(this,arguments); },
    parse: function(str:string, strict:boolean=true):any { require('JSON'); return JSON.parse.apply(this,arguments); },
    stringify: function(obj:object, strict:boolean=true):string { require('JSON'); return JSON.stringify.apply(this,arguments); },
};
```

## CAPI

### DString

DString provides dynamic string functionality via a struct
which uses the stack when strings are short:

``` clike
    Jsi_DString d = {"Here is your score "};
    puts(Jsi_DSPrintf(&d, "%s: -> %d/%d", user, n, m));
    Jsi_DSFree(&d);
```

Since the above strings are under 200 characters, no malloc call is required.

``` clike
    Jsi_DString d = {};
    Jsi_DSPrintf(&d , "%0300d", 1); // Malloc
    Jsi_DSSetLength(&d, 0);
    Jsi_DSPrintf(&d , "%0300d", 1); // No-malloc
    Jsi_DSFree(&d);
    Jsi_DSPrintf(&d , "%0300d", 1); // Malloc
    Jsi_DSFree(&d);
```

Space is discared with Jsi_DSFree, and can be reused with Jsi_DSSetLength:

You can also use Jsi_DSInit():

``` clike
    Jsi_DString d;
    Jsi_DSInit(&d);
    Jsi_DSAppend(&d, "Some stuff", NULL);
    Jsi_DSAppendLen(&d, "!", 1);
    Jsi_DSFree(&d);
```

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


``` clike
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
```


### Large String Buffers
When working with larger strings, we may want to preallocate a large
string in order to avoid repeated calls to realloc() as the string grows.
The normal approach might be something like:

``` clike
    Jsi_DString dStr;
    Jsi_DSInit(&dStr);
    Jsi_DSSetLength(&dStr, 50000);
```

Another alternative is to use the `JSI_DSTRING_VAR` macro, which avoids using malloc entirely.
`JSI_DSTRING_VAR` efficiently declares a Jsi_DString* pointing to an enlarged static DString upon the stack: eg:

``` clike
    JSI_DSTRING_VAR(dsPtr, 50000);
    Jsi_DSPrintf(dsPtr, "%04999d", 1); // No malloc.
```

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

``` clike
    Jsi_DSPrintf(&dstr, "%02d%-3d%04d", v1, v2, v3);
    str << std::setfill('0') << std::setw(2) << v1
        << std::setfill(' ') << std::setw(3) << std::left  << v2
        << std::setfill('0') << std::setw(4) << std::right << v3;
```

### Safety
The gcc compiler makes DString usage quite safe.

- It generates warnings for Jsi_DSPrintf argument mismatches.
- It warns when Jsi_DSAppend is missing NULL terminator.

There however are some gotchas to be aware of:

- DStrings greater than 200 bytes can not be assigned to one another.
- Failing to call Jsi_DSFree can leak memory.
- A DString has a maximum (compile time limit) of a 100 Meg.


## JSON

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

``` clike
    Jsi_Obj *nobj;
    Jsi_ValueMakeObject(interp, ret, nobj = Jsi_ObjectNew(interp));
    Jsi_ObjInsertFromValue( interp, nobj, Jsi_ValueNewStringKey(interp, "file"),
        Jsi_ValueNewStringKey(interp, file));
    Jsi_ObjInsertFromValue( interp, nobj, Jsi_ValueNewStringKey(interp, "line"),
       Jsi_ValueNewNumber(interp, line));

    // Alternatively, this can be simplified to:
    return Jsi_JSONParse(interp, ret, "{file:\"%s\", line:%d}", file, line);

    // The more deeply nested the object, the more code is simplified, eg:
    Jsi_JSONParseFmt(interp, ret, "{ a: [ {x:%d, y:%d}, {x:%d, y:[%d,%d,%d]}] }",a,b,c,d,e,f);
```

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

Any data sent between sub-[interps](Interp.md) will first be converted to/from JSON.
This because all data objects are private to an interp.

## Jsi-Lite


Jsi-Lite, which is a subset of the Jsi C source code which can be used without the script engine:


``` clike
    #define JSI_LITE_ONLY
    #include "jsi.c"
    //Your code goes here.
```

It include [DString](#dstring), Hash, List, Map, and Tree.

Tree is the underlying data structure for objects in JSI using Red-Black trees with invariant node
pointers: nodes are allocated using a single malloc, including space for the key.
This introduces a problem in that varying string keys can not be copied between nodes,
which is required when re-balancing the tree. Although tree supports swapping node positions
instead of keys, objects instead use a key of type STRINGPTR, a combination Hash table and and Tree,
which is fairly efficient because objects often share keys.


## Options

A Jsi_OptionSpec array provides details of fields in a struct. This information is used for
translating values to/from and from C struct fields, both for Javascript and
by the [Sqlite C-API](DBQuery.md).

A Jsi_OptionSpec array specifies for a struct field the
name, type, offset, help string, etc.  This specification is passed, along with a pointer
to the actual data,
to `Jsi_OptionProcess()` and `Jsi_OptionConf()`.


### Example Usage
The following example is a subset of the Jsi Sqlite command:

``` clike
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
```

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

``` clike
    typedef struct {
        int val;
        //...
    } MyData;
    
    Jsi_OptionSpec opts[] = {
        JSI_OPT(STRING, MyData, val),
        JSI_OPT_END(    MyData)
    };

==>

    test.c:7:6: warning: initialization from incompatible pointer type [enabled by default]
    test.c:7:6: warning: (near initialization for ‘opts[0].iniVal.ini_STRING’) [enabled by default]
```

### Is Specified
When a user provides a value for an option, the flag `JSI_OPT_IS_SPECIFIED` is set in the option flags.
Call Jsi_OptionChanged to determine if an option was specified.

**Note**:
    The Jsi header file [jsi.h](https://jsish.org/fossil/jsi/file/jsi.h#Jsi_OptionSpec) and source are best consulted for complete examples.

**Warning**:
    There are known concurrency issues with this feature.

## License

Jsi source is generally is covered by an MIT.

<!-- meta:{"file":{"index":1002}} -->
