Miscellaneous
=============
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


## License

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
- Provide amalgamated source for simplified [application integration](Builds.md#Embedding) .
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


## Index By Topic


- **Begin**: [Start](Start.md), [Download](Builds.md), [Build](Builds.md)
- **Docs**: [Builtins](Builtins.md), [Reference](Reference.md), [Index](Index.md), [License](Misc.md#License), [Language](Misc.md#Syntax), [Compatibility](Start.md#Compatibility)
- **Development**: [Types](Functions.md), [Debugging](Debug.md), [Logging](Logging.md)
- **Core**: [System](Builtins.md#System), [Info](Builtins.md#Info), [Interp](Interp.md), [Format](Builtins.md#format), [File-System](Builtins.md#File), [Events](Builtins.md#Event)
- **Integration**: [Modules](Start.md#Modules), [Require](Start.md#require), [Auto-Load](#auto-load)
- **Web**: [Server](WebSocket.md), [Preprocessing](WebSocket.md), [WebSocket](Builtins.md#WebSocket), [Markup](Reference.md#Util), [JSON](Builtins.md#JSON)
- **Miscellaneous**: [Extensions](Extensions.md), [Threads](Interp.md#Threads), [Signal](Builtins.md#Signal), [Sqlite](Sqlite.md), [MySQL](MySql.md), [Zvfs](Builtins.md#Zvfs), [Socket](Builtins.md#Socket), [WebSocket](Builtins.md#WebSocket)
- **Tools**: [Testing](Testing.md), [Tracing](Start.md#Tracing), [Profiling](Testing.md#Code-Profile), [Code-Coverage](Testing.md#Code-Coverage)
- **C/C++**: [Jsi-Lite](#jsi-lite), [C Extension](Extensions.md), [DString](Misc.md#DString), [CData](Extensions.md), [Options](Extensions.md#Options), [Sqlite-C](DBQuery.md), [JSON-C](Builtins.md#JSON)
- **Applications**: [SqliteUI](Builds.md#Apps)


## Packaging

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


## Extensions

Jsi is written in C. For embedded developers this provides an [Extensions](Extensions.md) interface to simplify
connecting low level (**C**) with high level (**GUI**) code.
The use of C-Structs is intrinsically integrated into Jsi at all levels,
and is the main mechanism used for module state, option parsing and
C extensions.
This direct
interfacing with C-structs can potential be used to process very
large amounts of data with little overhead.
This includes the Sqlite interface which also supports mass data transfers to/from structs,
which is of particular importance for embedded applications.

The C coding aspect of Jsi however is purely optional. 



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

Keys
====

Jsi provides limited support for iterators,
in the form of **for** loops and the method **keys()**:

``` js
cat > keys.jsi<EOF
var x = {a:1, b:2};
;Object.keys(x);
var y = [1,2];
;Ojbect.keys(y);
EOF
jsish --I unitTest:1 keys.jsi

==>

Object.keys(x) ==> [ "a", "b" ]
Object.keys(y) ==> [ 0, 1 ]
```

However this doesn't really buy you much.

``` js
var y = [1, 2];
for (var i in y) { puts(i); }
for (var i of Object.keys(y)) { puts(i); } // Same as above
```

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

## Expressions

Expression checking should be as simple as possible, eg:

    if (x) ...

is often preferred to:

``` js
if (x != '') ...
if (x != 0) ...
```

because it handles the case where x may be undefined.

## Conditionals

Jsi will check for undefined vars used on the left-hand side (LHS) of conditional expressions.  To such avoid errors, put the var on the RHS. 

``` js
var x;
if ('' === x) puts('x empty'); //OK.
if (x === '') puts('x empty');
/tmp/us.jsi:4: error: lhs value undefined in ===/!==
ERROR
```

For Loop
====
For loops are not always as simple as they seem.

## for...of

Use **for of** only for Arrays, not Objects, eg:

``` js
var x = [1,2];
for (var i of x) {}; // OK
var y = {a:1};
for (var i of y) puts(i);
error: operand not an array    (at or near "a")
```

However, if web-portability of code is desired, using it can break older browsers.

## for...in

For web-portability one might think that it is safe to use **for...in** instead of **for of** for Arrays, but there are some gotchas in some browsers:

- Indexes are not guaranteed to be in numerical order.
- Sometimes it will iterate over properties like **length**. 

Thus the safest way is using **for (;;)**.


## for (;;)

Use **for (;;)** for Arrays if you care about portability.

Chroot Setup
====

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


## Function Callbacks

When writing code in Jsi be aware that function callbacks are slower than **for** loops. For example, in:

``` js
x = [1,2,3], sum=0;
x.forEach(function(n) {sum+=n;});
for (var n of x) sum+=n;
```

the **forEach** function call is slower.

## Speed Comparison

The following shows just how much slower:

``` js
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

==>

Util.times(loop) ==> 49892
Util.times(func) ==> 206804
Util.times(noop) ==> 144220
```

As can be seen, even **noOp** call is slower due to overhead from function call setup.

## Case Study

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

``` js
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

jsish --I unitTest:1 /tmp/range.jsi

==>

range() ==> [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 ]
Util.times(range) ==> 1136
range1() ==> [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 ]
Util.times(range1) ==> 358
```

Both work, but the for loop is easier to read and 3-times faster.

### Conclusion
Callbacks may seem more elegant, but they are just not very efficient in Jsi.
And code ends up being harder to read.


Building SSL
====

How do you build-in SSL support to jsish?

Answer: yes.  make remake WITH_SSL=1

Note: Jsi now uses [lws](https://jsish.org/lws), a fork of LibWebsockets. 

## Notes

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

## Todo

Find a way to build-in SSL support statically with lib musl.

*bwtiffin*, might I suggest looking at mbedTLS?  It's our fave here for embedded work, and seems well supported in libwebsockets.

## Alternatives

Use an SSL web server that supports [Reverse Proxy](https://jsish.org/fossil/jsi2/wiki?name=Web+Reverse-Proxy).


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


## Debian

If you are on a debian system, you can build then install as a package:

    cd tools
    ./makedep.sh
    sudo dpkg -i jsish-*


FreeBSD

On FreeBSD use "gmake" instead of "make" and:

    pkg install fetch gmake bison



## Windows

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


## Standalone

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


## Amalgamation

Amalgamated source is the easiest way to incorporate Jsi into an existing application:
Here is a simple example:

``` clike
#include "jsi.c"

int main(int argc, char *argv[])
{
    Jsi_Interp *interp = Jsi_InterpNew(NULL);
    Jsi_EvalString(interp, "for (var i=1; i<=3; i++)  puts('TEST:',i);", 0);
    if (argc>1)
        Jsi_EvalFile(interp, Jsi_ValueNewStringKey(interp, argv[1]), 0);
}
```

which we compile with:

    gcc  myfile.c -lm -lz -ldl -lpthread


Another alternative that simplifies debugging Jsi is
using [jsiOne.c](https://jsish.org/jsi/doc/tip/src/jsiOne.c)


## Web


### Client
To download a file from the web we can use:

``` bash
jsish -w -O jsi-app.zip http://jsish.org/jsi-app/zip
```

### Server
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

## Sqlite


### Usage

Basic Sqlite usage:

``` js
var db = new Sqlite('mydata.db');
db.query('SELECT * FROM tbl');
Output
[ { a:99, b:1 }, { a:95, b:2 }, { a:91, b:3 } ]
```

## GUI

The database GUI is invoked using:

``` bash
jsish -S mydata.db
```

### Database Dump
To dump a database use:

``` bash
jsish -S -dump true mydata.db
```

which produces output like the sqlite .dump command.


## Applications


### Ledger

``` bash
fossil clone http://jsish.org/jsi-app jsi-app.fossil
jsish -a jsi-app.fossil Ledger
```

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

## C-API

Jsi has a sizeable C-API, best documented by the header file [jsi.h](https://jsish.org/jsi/file/src/jsi.h).

A portion of which can be used without the interpreter code by C programmers (Jsi-Lite).

## DString

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


### Sub-Interps
Any data sent between sub-[interps](Interp.md) will first be converted to/from JSON.
This because all data objects are private to an interp.

## Jsi-Lite


Jsi-Lite is a subset of the Jsi C source code which can be used without the script engine.


### Jsi_DString
[Jsi_DString](#dstring) is available in Jsi-Lite.


### Jsi_Hash
This pages describes how to use Jsi_Hash.
Search for Jsi_Hash in [jsi.h](https://jsish.org/jsi/file/jsi.h#Jsi_Hash) for details.

Hash provides simple hash table functionality.

``` clike
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
```

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

``` clike
    #define JSI_LITE_ONLY
    #include "jsi.c"
    //Your code goes here.
```

Following is some demo code:

- a minimal demo of Jsi-Lite: [litedemo.c](https://jsish.org/jsi/file/c-demos/litedemo.c).
- a demo of Jsi_List: [litedemo.c](https://jsish.org/jsi/file/c-demos/listdemo.c).
- a more comprehensive database demo: [dbdemo.c](https://jsish.org/jsi/file/c-demos/dbdemo.c).


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
    The Jsi header file [jsi.h](https://jsish.org/jsi/file/jsi.h#Jsi_OptionSpec) and source are best consulted for complete examples.

**Warning**:
    There are known concurrency issues with this feature.

## jsi-js

TODO
