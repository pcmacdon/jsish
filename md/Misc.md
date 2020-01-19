Miscellaneous
=============
<div id="sectmenu"></div>
Options
----
Options are parameters handled by builtin commands.
These get passed in an object:

    var db = new Sqlite('/tmp/testsql.db',{maxStmts:1000, readonly:true});

Usually there is a conf() method providing access to options after creation:

    Interp.conf(); // Dump all options
    Interp.conf('strict'); // Get one option
    Interp.conf({strict:false, maxDepth:99}); // Set one or more options.

**Note**:
    Some options may be readOnly. And an option that is initOnly
    may only be set at object creation time.

    Internally, option parsing is implemented via [C Options](C-API.md#options).



Introspection
----
There are several levels of introspection built-in to Jsi.
One is displayed when calling an invalid method:

    Array.xxx();
    error: 'Array' sub-commands are: concat fill filter forEach indexOf join lastIndexOf map pop push reverse shift sizeOf slice some sort splice unshift.    (at or near "xxx")

Another, is querying with the
[Info](Reference.md#Info) command:

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

and so on.


Syntax
----
The following syntax is implemented by Jsi (see [Reference](Reference.md) for commands):

    continue [IDENT] ;
    break [IDENT] ;
    debugger ;
    delete IDENT ;
    do { STMTS; } while( EXPR ) ;
    for ([var] IDENT = EXPR; EXPR; EXPR) { STMTS; }
    for ([var] IDENT in EXPR) { STMTS; }
    for ([var] IDENT of EXPR) { STMTS; }
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
    with ( EXPR ) { STMTS; }

- Square brackets indicate optional.
- Curley braces are just for illustrative purposes and except for function and switch are not required.


### Expressions
Expressions take the usual form:

    IDENT*.*IDENT
    IDENT*[*EXPR]
    IDENT*(*ARGS)
    (EXPR)
    EXPR ? STMTS :  STMTS
    STMTS , STMTS [, ...]
    EXPR OP EXPR

where OP is one of the binary operators +, -, *, /, etc.


### Terminals
| Name      | Description                                                                       |
|-----------|-----------------------------------------------------------------------------------|
| ARGS      | Zero or more comma-seperated arguments                                            |
| EXPR      | An expression (see below)                                                         |
| FUNC      | A function value                                                                  |
| IDENT     | Is an valid identifier                                                            |
| PRIMITIVE | A primitive value acceptable as an [argument type](Types.md#typenames).           |
| STMTS     | Is zero or more statements                                                        |
| TYPE      | A type value acceptable as defaults                                               |


### Keywords
Keywords can be displayed using Info.keywords():

    "...", "any", "arguments", "array", "boolean", "break", "case", "catch",
    "continue", "debugger", "default", "delete", "do", "else", "false",
    "finally", "for", "function", "if", "in", "instanceof", "new", "null",
    "number", "object", "of", "regexp", "return", "string", "switch",
    "this", "throw", "true", "try", "typeof", "undefined", "userobj", "var",
    "void", "while", "with"


Language Comparisons
----

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


License
----
Jsi source is covered by the following MIT license:

    The MIT License (MIT)
    
    Copyright (c) 2013 Peter MacDonald
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.


### Libwebsockets
Jsi links agains Libwebockets, which is covered by LGPL
with an extra clause allowing static linking.

    Libwebsockets and included programs are provided under the terms of the GNU
    Library General Public License (LGPL) 2.1, with the following exceptions:
    
    1) Static linking of programs with the libwebsockets library does not
    constitute a derivative work and does not require the author to provide
    source code for the program, use the shared libwebsockets libraries, or
    link their program against a user-supplied version of libwebsockets.
    
    If you link the program to a modified version of libwebsockets, then the
    changes to libwebsockets must be provided under the terms of the LGPL in
    sections 1, 2, and 4.
    
    2) You do not have to provide a copy of the libwebsockets license with
    programs that are linked to the libwebsockets library, nor do you have to
    identify the libwebsockets license in your program or documentation as
    required by section 6 of the LGPL.
    
    However, programs must still identify their use of libwebsockets. The
    following example statement can be included in user documentation to
    satisfy this requirement:
    
    "[program] is based in part on the work of the libwebsockets  project
    (http://libwebsockets.org)"
    
                      GNU LESSER GENERAL PUBLIC LICENSE
                           Version 2.1, February 1999
    ...

**Note**:
    This seems to to say that as long as libwebsockets is not modified,
    all that is required is an acknowledgement in your user documentation.


### Others
Other software including sqlite, miniz, jsmn, regex from musl,
etc are either public domain, BSD or MIT compatible.


Design and Origin
----
Jsi is a Javascript interpreter written in C.
Additional functionality, implemented with scripts is bundled as "jsish".

Jsi is a byte-code oriented interpreter originally designed for interfacing-with, and embedding into C.

This makes it very different from [Node-js](https://nodejs.org) which is a compiler written in **C++**,
and which is not designed to be embedded.

Jsi is **C**-centric whereas Node is **JS**-centric.  Meaning that with Jsi, the locus of
control can resides in C-code.

Although Jsi was originally was based off source from [quad-wheel](https://code.google.com/archive/p/quad-wheel),
it is now internally modelled after [Tcl](https://www.tcl.tk/doc/scripting.html).

ECMA Compatibilty
----
Jsi implements much of version 5.1 of the
[Ecma-script 262 standard](http://www.ecma-international.org/ecma-262/5.1/ECMA-262.pdf).
Notable deviations include:

- Semicolons are required at the end of expressions.
- Using empty array/object elements will kick an error, eg. **[1,,3]**.
- **delete** actually deletes things, not just object properties.
- **length** works for objects, as well as arrays/strings/functions.
- The value of **typeof[]** is **"array"** instead of **"object"**.
- No **Error** object implemented: the argument to **catch()** is just a string.
- No **Date** object implemented: use **strftime/strptime**.
- UTF support is only for strings (not code) and is lightly tested.
- Prototype and other inheritance related features are incomplete, and can even be disabled entirely.

Extensions include:

- Functions parameters may have [types and defaults](Types.md).
- [Testing](Testing.md) support that leverages type-checking.
- Select features from newer versions of the standard (eg. Array **of** and **forEach**).
- Limited basic arrow functions: no default values, and single parameters may not be braced.
- Non-standard object functions such as **merge** are available, eg. **o = o1.merge({a:1,b:2});**

Goals
----
Following are principle goals  Jsi:

- Support embedded development using plain **C** (C99).
- But should also be compilable by **native GNU g++**, without use of *"extern C"*.
- Have as few dependencies as possible.
- Be generally free of value/object/memory leaks (verified with -fsanitize).
- Provide amalgamated source for simplified [application integration](Download.md#Embedding) .
- Low-level C-functions available in a **C-only** [Lite](C-API.md#jsi-lite) version.
- Come with a [Debugger](Debug.md).
- Support Web applications, particularly with database and websockets.
- Support [standalone applications](Builtins.md#zvfs) via self-mounting .zip.
- [Package](Coding.md) and extension support.

Compiling as C++ is supported, mostly for integrity checks.

**Note**:
    C-integration is the main priority in Jsi, not speed of script execution.

Shortcomings
----
Following is a partial list of things that are either incomplete or unimplemented:

- Creation of a complete test suite for code-coverage.
- Run applications directly from fossil.
- A PostgreSql extension.
- Extension for libmicrohttpd for use in post.
- Support for libevent/libev.


Rational
----
- Desktop applications are held hostage by their user interface, be it QT, GTK, IOS or .NET.
- Increasingly web browsers are becoming the GUI, usually over the Internet.
- Moderately complex applications often end up requiring some form of script support, eg. Lua.
- If an application employs a Web GUI, a script language already is being used: Javascript.
- Time, energy and resources can be saved by using the same language on both ends.
- In fact, the same scripts can even be run in both the browser and the app.
- JSON provides seamless data interchange, thus avoiding data structure compatibility issues.

