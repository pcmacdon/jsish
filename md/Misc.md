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
- Provide amalgamated source for simplified [application integration](Build.md#Embedding) .
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


Doc Index
----

- **Start**: [Download](Build.md), [Building](Build.md#Building), [Using](Build.md#Using), [Embedding](Build.md#Embedding)
- **Docs**: [Builtins](Builtins.md), [Reference](Reference.md), [Index](Docs.md), [License](Misc.md#License), [Language](Misc.md#Language Comparisons), [ECMA](Misc.md#ECMA Compatibilty)
- **Development**: [Types](Types.md), [Strict Mode](Types.md#Strict Mode), [Type Checking](Types.md#Checking), [Debugging](Debug.md), [Errors](Testing.md#Errors), [Logging](Logging.md)
- **Core**: [System](Builtins.md#System), [Info](Builtins.md#Info), [Interp](Interp.md), [Format](Logging.md#format), [File-System](Builtins.md#File), [Events](Builtins.md#Event)
- **Integration**: [Modules](Coding.md#Modules), [Packages](Coding.md#Packages), [Auto-Load](Coding.md#Auto-Load)
- **Web**: [Server](Web.md), [Preprocessing](Web.md), [WebSocket](Builtins.md#WebSocket), [Markup](Reference.md#Util), [JSON](Builtins.md#JSON)
- **Miscellaneous**: [CData](CData.md), [Threads](Interp.md#Thread-Interps), [Signal](Builtins.md#Signal), [Sqlite](Sqlite.md), [MySQL](MySql.md), [Zvfs](Builtins.md#Zvfs), [Socket](Builtins.md#Socket), [WebSocket](Builtins.md#WebSocket)
- **Tools**: [Testing](Testing.md), [Tracing](Coding.md#Execution Trace), [Profiling](Coding.md#Code Profile), [Code-Coverage](Coding.md#Code Coverage)
- **C/C++**: [Jsi-Lite](C-API.md#Jsi-Lite), [C Extension](CData.md), [DString](C-API.md#DString), [CData](CData.md), [Options](C-API.md#Options), [Sqlite-C](DBQuery.md), [JSON-C](C-API.md#JSON)
- **Applications**: [Ledger](Ledger.md), [SqliteUI](Build.md#Apps), [Web Server](Coding.md#Server)


Packaging
---------
<table class="mytbl"><tr>

<td>
A <a href="Deploy.md">Deploy</a> is a zip/sqlar archive or fossil
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

</td>
<td>
<!--
        *************************************
        *  .-----------------------------.  *
        *  |     Server running Nginx    |  *
        *  | .---------.      .-----.    |  *
        *  | |  Jsish  |<----+ Fossil|   |  *
        *  | '----+----'      '--+--'    |  *
        *  |      ^              ^       |  *
        *  '------+--------------+-------'  *
        *         |              |          *
        *         v       :      |          *
        *    .---------.  :   .--+--.       *
        *    | Browser |  :  | Fossil|      *
        *    '----+----'  :   '--+--'       *
        *       User      :  Developer      *
        *                 :                 *
        ************************************* -->

<svg class="diagram" xmlns="http://www.w3.org/2000/svg" version="1.1" height="240" width="288" style="margin:0 auto 0 auto;"><g transform="translate(8,16 )">
<path d="M 16,0 L 16,96 " style="fill:none;"/>
<path d="M 32,32 L 32,64 " style="fill:none;"/>
<path d="M 32,144 L 32,176 " style="fill:none;"/>
<path d="M 72,72 L 72,136 " style="fill:none;"/>
<path d="M 112,32 L 112,64 " style="fill:none;"/>
<path d="M 112,144 L 112,176 " style="fill:none;"/>
<path d="M 192,72 L 192,144 " style="fill:none;"/>
<path d="M 256,0 L 256,96 " style="fill:none;"/>
<path d="M 16,0 L 256,0 " style="fill:none;"/>
<path d="M 32,32 L 112,32 " style="fill:none;"/>
<path d="M 176,32 L 208,32 " style="fill:none;"/>
<path d="M 120,48 L 160,48 " style="fill:none;"/>
<path d="M 32,64 L 112,64 " style="fill:none;"/>
<path d="M 176,64 L 208,64 " style="fill:none;"/>
<path d="M 16,96 L 256,96 " style="fill:none;"/>
<path d="M 32,144 L 112,144 " style="fill:none;"/>
<path d="M 176,144 L 208,144 " style="fill:none;"/>
<path d="M 32,176 L 112,176 " style="fill:none;"/>
<path d="M 176,176 L 208,176 " style="fill:none;"/>
<path d="M 176,32 C 159.2,32 160,48 160,48 " style="fill:none;"/>
<path d="M 208,32 C 224.8,32 224,48 224,48 " style="fill:none;"/>
<path d="M 176,64 C 159.2,64 160,48 160,48 " style="fill:none;"/>
<path d="M 208,64 C 224.8,64 224,48 224,48 " style="fill:none;"/>
<path d="M 176,144 C 159.2,144 160,160 160,160 " style="fill:none;"/>
<path d="M 208,144 C 224.8,144 224,160 224,160 " style="fill:none;"/>
<path d="M 176,176 C 159.2,176 160,160 160,160 " style="fill:none;"/>
<path d="M 208,176 C 224.8,176 224,160 224,160 " style="fill:none;"/>
<polygon points="200,72 188,66.4 188,77.6 "  style="stroke:none" transform="rotate(270,192,72 )"/>
<polygon points="128,48 116,42.4 116,53.6 "  style="stroke:none" transform="rotate(180,120,48 )"/>
<polygon points="80,136 68,130.4 68,141.6 "  style="stroke:none" transform="rotate(90,72,136 )"/>
<polygon points="80,72 68,66.4 68,77.6 "  style="stroke:none" transform="rotate(270,72,72 )"/>
<g transform="translate(0,0)"><text text-anchor="middle" x="64" y="20">S</text><text text-anchor="middle" x="72" y="20">e</text><text text-anchor="middle" x="80" y="20">r</text><text text-anchor="middle" x="88" y="20">v</text><text text-anchor="middle" x="96" y="20">e</text><text text-anchor="middle" x="104" y="20">r</text><text text-anchor="middle" x="120" y="20">r</text><text text-anchor="middle" x="128" y="20">u</text><text text-anchor="middle" x="136" y="20">n</text><text text-anchor="middle" x="144" y="20">n</text><text text-anchor="middle" x="152" y="20">i</text><text text-anchor="middle" x="160" y="20">n</text><text text-anchor="middle" x="168" y="20">g</text><text text-anchor="middle" x="184" y="20">N</text><text text-anchor="middle" x="192" y="20">g</text><text text-anchor="middle" x="200" y="20">i</text><text text-anchor="middle" x="208" y="20">n</text><text text-anchor="middle" x="216" y="20">x</text><text text-anchor="middle" x="56" y="52">J</text><text text-anchor="middle" x="64" y="52">s</text><text text-anchor="middle" x="72" y="52">i</text><text text-anchor="middle" x="80" y="52">s</text><text text-anchor="middle" x="88" y="52">h</text><text text-anchor="middle" x="176" y="52">F</text><text text-anchor="middle" x="184" y="52">o</text><text text-anchor="middle" x="192" y="52">s</text><text text-anchor="middle" x="200" y="52">s</text><text text-anchor="middle" x="208" y="52">i</text><text text-anchor="middle" x="216" y="52">l</text><text text-anchor="middle" x="136" y="132">:</text><text text-anchor="middle" x="136" y="148">:</text><text text-anchor="middle" x="48" y="164">B</text><text text-anchor="middle" x="56" y="164">r</text><text text-anchor="middle" x="64" y="164">o</text><text text-anchor="middle" x="72" y="164">w</text><text text-anchor="middle" x="80" y="164">s</text><text text-anchor="middle" x="88" y="164">e</text><text text-anchor="middle" x="96" y="164">r</text><text text-anchor="middle" x="136" y="164">:</text><text text-anchor="middle" x="176" y="164">F</text><text text-anchor="middle" x="184" y="164">o</text><text text-anchor="middle" x="192" y="164">s</text><text text-anchor="middle" x="200" y="164">s</text><text text-anchor="middle" x="208" y="164">i</text><text text-anchor="middle" x="216" y="164">l</text><text text-anchor="middle" x="136" y="180">:</text><text text-anchor="middle" x="56" y="196">U</text><text text-anchor="middle" x="64" y="196">s</text><text text-anchor="middle" x="72" y="196">e</text><text text-anchor="middle" x="80" y="196">r</text><text text-anchor="middle" x="136" y="196">:</text><text text-anchor="middle" x="160" y="196">D</text><text text-anchor="middle" x="168" y="196">e</text><text text-anchor="middle" x="176" y="196">v</text><text text-anchor="middle" x="184" y="196">e</text><text text-anchor="middle" x="192" y="196">l</text><text text-anchor="middle" x="200" y="196">o</text><text text-anchor="middle" x="208" y="196">p</text><text text-anchor="middle" x="216" y="196">e</text><text text-anchor="middle" x="224" y="196">r</text><text text-anchor="middle" x="136" y="212">:</text></g></g></svg>

</td>
</tr>
</table>

Security
----
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


C-API
----
Jsi is written in C. For embedded developers this provides a [C-API](C-API.md) that simplifies
connecting low level (**C**) with high level (**GUI**) code.
The use of C-Structs is intrinsically integrated into Jsi at all levels,
and is the main mechanism used for module state, option parsing and
C extensions.
This direct
interfacing with C-structs can potential be used to process very
large amounts of data with little overhead.
This includes the Sqlite interface which also supports mass data transfers to/from structs,
which is of particular importance for embedded applications.

The C coding aspect of Jsi however is purely optional. [Ledger](Ledger.md) and the
other demo applications neither use nor require it.



Wiki Dump
====

Nginx
====

Although Jsi can serve web content directly, on the Internet
it is more common to reverse proxy to a web-server via localhost:

Following config is for setting up jsish as a reverse proxy under Nginx

    location /App00/Ledger { proxy_pass http://localhost:8800; include proxy_params; }

And jsish might be invoked as:

    jsish -a jsi-app.fossil Ledger -port 8800 -urlPrefix /App00/Ledger -noGui true

Or run via chroot/su.

Wget
====
Jsi_Wget used for in memory download.

What already worked was:

    jsish -w -O - http://jsish.org/ii.html
    hello

But report uncovered omissions in [Patch](info/d26981662c), which now supports:

    jsish -w -O . http://jsish.org/ii.html
    hello

BTW: note the new [js-demos/wsget.jsi](doc/tip/js-demos/wsget.jsi?mimetype=text/plain), that demonstrates overrides.


Keys
====

Jsi provides limited support for iterators,
in the form of **for** loops and the method **keys()**:

    cat > keys.jsi<EOF
    var x = {a:1, b:2};
    ;x.keys();
    var y = [1,2];
    ;y.keys();
    EOF
    jsish --U keys.jsi

which outputs:

    x.keys() ==> [ "a", "b" ]
    y.keys() ==> [ 0, 1 ]

However this doesn't really buy you much.

    var y = [1, 2];
    for (var i in y) { puts(i); }
    for (var i of y.keys()) { puts(i); } // Same as above

Moreover in ECMA **keys())** works very differently, eg:

    var k = Object.keys(x);

And this works only for Objects: Array does not support **keys()**.

In summary, iterators, other than **for**, are not really supported.

See [Iterators on Stackoverflow.com](https://stackoverflow.com/questions/14379274/how-to-iterate-over-a-javascript-object)
 

Undefined
====

Bugs due to **undefined** vars are one of the most annoying things about JS:
code thought to be working suddenly starts throwing errors.

Below are some of the things you can do to minimise this in Jsi.


Expressions
----
Expression checking should be as simple as possible, eg:

    if (x) ...

is often preferred to:

    if (x != '') ...
    if (x != 0) ...

because it handles the case where x may be undefined.

Conditionals
----
Jsi will check for undefined vars used on the left-hand side (LHS) of conditional expressions.  To such avoid errors, put the var on the RHS. 

    var x;
    if ('' === x) puts('x empty'); //OK.
    if (x === '') puts('x empty');
    /tmp/us.jsi:4: error: lhs value undefined in ===/!==
    ERROR

For Loop
====
For loops are not always as simple as they seem.

for...of
----
Use **for of** only for Arrays, not Objects, eg:

    var x = [1,2];
    for (var i of x) {}; // OK
    var y = {a:1};
    for (var i of y) puts(i);
    error: operand not an array    (at or near "a")

However, if web-portability of code is desired, using it can break older browsers.

for...in
----
For web-portability one might think that it is safe to use **for...in** instead of **for of** for Arrays, but there are some gotchas in some browsers:

- Indexes are not guaranteed to be in numerical order.
- Sometimes it will iterate over properties like **length**. 

Thus the safest way is using **for (;;)**.


for (;;)
----
Use **for (;;)** for Arrays if you care about portability.

Chroot Setup
====

Given it's small size, jsish is well suited for chroot deployments.

On the jsish.org site the user **jsiusr00** was created with directory contents:

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

To scale this up while saving space, multiple users "jsiusrNN" are created with readonly files hard-linked to "jsiusr00".

Finally "chattr +i" is used make them immutable.

Thus incremental size for each additional
user is really only the data file "ledgerjs.db".

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

All previous directories have no shell, so with the addition of quotas and ulimits we end up with a
deployment that is simple but secure.

**NOTE:** The slight bump in "jsiusr10" is due to the addition of "sh", to allow execing fossil in a chroot.


Function Callbacks
----
When writing code in Jsi be aware that function callbacks are slower than **for** loops. For example, in:

    x = [1,2,3], sum=0;
    x.forEach(function(n) {sum+=n;});
    for (var n of x) sum+=n;

the **forEach** function call is slower.

Speed Comparison
----
The following shows just how much slower:

    var x = new Array(10000).fill(1);
    
    function loop() {
       var sum=0;
       for (var n of x) sum+=n;
       return sum;
    }
    
    function func() {
       var sum=0;
       x.forEach(function(n) {sum+=n;});
       return sum;
    }
    
    function noop() {
       var sum=0;
       x.forEach(noOp);
       return sum;
    }
    
    ;Util.times(loop);
    ;Util.times(func);
    ;Util.times(noop);

which outputs:

    Util.times(loop) ==> 49892
    Util.times(func) ==> 206804
    Util.times(noop) ==> 144220

As can be seen, even **noOp** call is slower due to overhead from function call setup.

Case Study
----
Here is an example that considers the best way to implement a **range** function like:

    function range(size, start, step) {
        var foo = [];
        for (var i = start; i <= size; i+=step)
           foo.push(i);
        return foo;
    }
    range(10,1,1);


### Conventional JS
A quick look on Stackoverflow.com shows results for
[range](https://stackoverflow.com/questions/3895478/does-javascript-have-a-method-like-range-to-generate-a-range-within-the-supp)
and [sequence](https://stackoverflow.com/questions/3746725/create-a-javascript-array-containing-1-n).
Note the mind-bending ways of trying to accomplish this seemingly simple task.

A popular answer was:

    var N = 10; 
    Array.apply(null, {length: N}).map(Number.call, Number)

### Jsi Solution
Unfortunately the above doesn't work in Jsi and:

- It needs a large **EXPLANATION** section, even though it is only 1 line!
- Most other answers require ES6, which Jsi does not have.
- Many correctly observed you are probably better off using a for loop.

Here are 2 implementations that work in both Jsi and ECMA:

    cat > /tmp/range.jsi <<EOF
    function range(n=20, start=0, step=1) {
        return Array(n).fill(0).map(function(v,i,o) { return start+i*step; });
    }
    function range1(n=20, start=0, step=1) {
       var a = Array(n).fill(0);
       for (var i in a)
          a[i] = start+i*step;
       return a;
    }
    
    ;range();
    ;Util.times(range);
    
    ;range1();
    ;Util.times(range1);
    EOF
    
    jsish --U /tmp/range.jsi

Output:

    range() ==> [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 ]
    Util.times(range) ==> 1136
    range1() ==> [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 ]
    Util.times(range1) ==> 358

Both work, but the for loop is easier to read and 3-times faster.

### Conclusion
Callbacks may seem more elegant, but they are just not very efficient in Jsi.
And code ends up being harder to read.


Building SSL
====

How do you build-in SSL support to jsish? *(question asked in a [Forum post](forumpost/ea4abb2ab1))*

Currently SSL build is not supported by configure.

I *(pmacdona)* have built jsish with SSL before, 2 different ways:

- Linking in the Ubuntu built WebSockets (doesn't work anymore), and
- Build WebSockets with CMAKE and enable SSL there.

This second way should still work, and involves (from memory) editing the jsi Makefile and setting **USECMAKE=1**.  Then edit websockets/Makefile and set the various `*SSL*` flags to 1.  Note the result will not be static as it becomes dependent upon **gnutls** and/or **openssl** shared libs.

Note there is a side project to fix this by [forking Libwebsocket](info/d8fd524dcf)

Notes
-----
*bwtiffin*  First up, it was mostly flailing and failing.

Trying to get the first build, from the initial instructions above, and a make clean in jsi and jsi/websocket dirs.

src/jsiWebSocket.c:2763 missing a semi-colon
 
Then tried `make Websocket.so` in jsi/  that seemed to work, but symbols are probably not in place.

Added -lssl, -lcrypto to PROGLDFLAGS in jsi/Makefile, got further but `lws_dump_json_context` needed to be commented out in src/jsiWebSocket.c:2523 and line 2714, the libwebsockets I have in /usr/lib64 does not have the lws_dump_json_context symbols (or this is a sign that I've blended the make/build).

In case there is a hint here; last compile line completed with:

    gcc -I. -Isrc -Wall -Wsign-compare -Wtype-limits -Wuninitialized -DJSI__MAIN=1
    -g -Og -O0 -Iwebsocket/src/lib  -Iwebsocket/src/build -Iwebsocket/unix
    -Iwebsocket/build/unix -Isqlite/src -frecord-gcc-switches -fpic
    -DJSI__BASE64=1 -DJSI__CDATA=1 -DJSI__DEBUG=1 -DJSI__ENCRYPT=1 -DJSI__EVENT=1
    -DJSI__FILESYS=1 -DJSI__INFO=1 -DJSI__LOAD=1 -DJSI__MARKDOWN=1 -DJSI__MATH=1
    -DJSI__MD5=1 -DJSI__READLINE=1 -DJSI__SHA1=1 -DJSI__SHA256=1 -DJSI__SIGNAL=1
    -DJSI__STUBS=1 -DJSI__THREADS=1 -DJSI__ZVFS=1 -DJSI__MEMDEBUG=0 -DJSI__MINIZ=0
    -DJSI__REGEX=0 -DJSI__SANITIZE=0 -DJSI__SOCKET=1 -DJSI__SQLITE=1
    -DJSI__WEBSOCKET=1 -DJSI__WEBSOCKET=1 -DJSI__SQLITE=1
    -DJSI_PKG_DIRS=\""/home/btiffin/inst/langs/jsi/jsi/lib,/usr/local/lib/jsi"\"
    -DJSI_CONF_ARGS=\"\" src/jsiLexer.o src/jsiFunc.o src/jsiValue.o
    src/jsiRegexp.o src/jsiPstate.o src/jsiInterp.o src/jsiUtils.o src/jsiProto.o
    src/jsiFilesys.o src/jsiChar.o src/jsiString.o src/jsiBool.o src/jsiNumber.o
    src/jsiArray.o src/jsiLoad.o src/jsiHash.o src/jsiOptions.o src/jsiStubs.o
    src/jsiFormat.o src/jsiJSON.o src/jsiCmds.o src/jsiFileCmds.o src/jsiObj.o
    src/jsiSignal.o src/jsiTree.o src/jsiCrypto.o src/jsiDString.o src/jsiMath.o
    src/jsmn.o src/jsiZvfs.o src/jsiUtf8.o src/jsiUserObj.o src/jsiSocket.o
    src/jsiSqlite.o src/jsiWebSocket.o src/jsiMySql.o src/jsiCData.o
    src/jsiMarkdown.o src/jsiVfs.o src/parser.o src/linenoise.o src/jsiEval.o
    sqlite/build/unix/libsqlite3.a src/main.o -rdynamic -o jsish_ -lm -lssl
    -lcrypto websocket/build/unix/libwebsockets.a -ldl -lpthread -lz

(all one line, edited in Vim with word wrap and code block indent)

Completed, but jsish `Interp.conf('hasOpenSSL')` is still false.  Fairly lost in the forest.  Build passes all tests, but still not right somewhere.

Was unsure if this build is supposed to be EXT_WEBSOCKET or MOD_WEBSOCKET and kinda gave up for now.  Will require more reading through the build system first, as flailing can only get you so far.  I'm pretty sure I have a blendo set of make rules now.  Unsure if the goal is LWS in jsish or LWS from a loadable module.

*Feel free to remove this part of the wiki page after reading.  I'll try and make a better report after a read through with proper notes on steps taken, starting fresh.*

Todo
----
Find a way to build-in SSL support statically with lib musl.

*bwtiffin*, might I suggest looking at mbedTLS?  It's our fave here for embedded work, and seems well supported in libwebsockets.

Alternatives
----
Use an SSL web server that supports [Reverse Proxy](wiki/Web Reverse-Proxy).


Building Jsi on Linux
====

Jsi is written in C, but can be compiled as either native C, or native C++ (does not use **extern C**).

On Debian a few packages are required:

    sudo apt-get install build-essential bison libreadline-dev libsqlite3-dev libwebsockets-dev libncurses-dev cmake libmysqlclient-dev

**Note:** Some packages (eg. **cmake**) are required only for specific configurations.

To build the Linux target there are are two steps:

    ./configure

Do not be surprised to see comiler output from configure:

it compiles the stripped down shell "jsimin".

Next, run "make" to build the actual "jsish" executable.

    make

If you want, you can see other available options using:

    ./configure --help

**Note:** The directory **Configs/**, which contains a number of predefined configurations which can be copied to **make.conf**

The last step is to run the test suite (optional):

    make test


Debian
----
If you are on a debian system, you can build then install as a package:

    cd tools
    ./makedep.sh
    sudo dpkg -i jsish-*


FreeBSD
----
On FreeBSD use "gmake" instead of "make" and:

    pkg install fetch gmake bison



Windows
----
Jsi can be cross compiled from Linux to Windows using the Mingw32 package:

    sudo apt-get install gcc-mingw-w64

The [sqlite](https://sqlite.org/download.html) and
[libwebsockets](https://libwebsockets.org/) source needs to be downloaded and unpacked in "../sqlite"
and "../websockets".  This now happens automatically.

Then configure using:

    ./configure --config=win

**WARNING:** Certain features (eg. signals) are disabled in the 
Windows build. There are also differences in some of the file-system 
access functions.


Standalone
----
The **standalone** build produces a static binary that contains no external library references.
This is useful when you need a standalone executable with no external dependancies.

To create a static image, Jsi uses the Musl library.

The first step is to download [Musl](http://www.musl-libc.org) and unpack it.
Then change to the **musl** dir and run configure/make, eg:

     ./configure --prefix=$HOME/usr && make install

Ensure that *~/usr/bin* is in your path with export PATH=$PATH:$HOME/usr/bin.
Then back in the **jsi** dir do the following:

    echo '#define __P(x) x' > ~/usr/include/sys/cdefs.h
    echo '#include <miniz/zlib.h>' >  ~/usr/include/zlib.h
    cp -pr miniz ~/usr/include/

The sqlite and libwebsockets source needs to be downloaded and unpacked in **../sqlite**
and **../websockets**.

The static jsish can then be built with:

    ./configure --config=musl
    make


Amalgamation
----
Amalgamated source is the easiest way to incorporate Jsi into an existing application:
Here is a simple example:

    #include "jsi.c"
    
    int main(int argc, char *argv[])
    {
        Jsi_Interp *interp = Jsi_InterpNew(NULL);
        Jsi_EvalString(interp, "for (var i=1; i<=3; i++)  puts('TEST:',i);", 0);
        if (argc>1)
            Jsi_EvalFile(interp, Jsi_ValueNewStringKey(interp, argv[1]), 0);
    }

which we compile with:

    gcc  myfile.c -lm -lz -ldl -lpthread


Another alternative that simplifies debugging Jsi is
using [jsiOne.c](https://jsish.org/jsi/doc/tip/src/jsiOne.c)




