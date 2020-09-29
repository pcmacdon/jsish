Using Jsi
====
[Back to Index](Index.md "Goto Jsi Documentation Index")

Jsi is a **C**-extensible JS *-ish* interpreter
providing compact and self contained server-side web scripting
*(To get Jsi see [Readme](https://jsish.org/jsi))*.

## Basics

Jsi implements most of *Ecma-script 5.1* (see [Compatibility](#compatibility) and [Reference](Reference.md))
with some notable differences.

Unlike ecmascript, Jsi expressions and var statements require semicolons, ie:

``` js
function foo() {
    var x = 1
    return ++x
}
```
must be written as:
``` js
function foo() {
    var x = 1;
    return ++x;
}
```
in order to compile.

### Functions
Function arguments are either standard or **typed**:
``` js
function foo(a,b) {}
function bar(a:number, b:string='') {}
```
&#x1f6a9; More on [Typed-arguments](Types.md).

Arrow functions are also supported.
```
var f = (x,y) => { puts(x,y); };
var g = (x,y) => x*y;
```
+++ Arrow Limitations

**Note**: Jsi's simple parser requires single args to be written without braces, eg:
```
var f = x => { return x*2; };
var g = (x) => { return x*2; }; // ERROR!
```
Also, arrow function may not use types.

++++

### Variables

Variables may be declared with:
```
var x = 0;
const z = 0;
let y = 0;
```
However `let` in Jsi is just an alias for `var` *(not scoped)*.

### Objects

Object initializer must include a colon in the first element,
after which shorthand is accepted:
```
var b = 1, c = 2;
var o = {a:1, b, c, func() { return 1;} };
```

Object also support [freeze](Builtins.md#freeze).

### Input/Output

Following are examples of input and output in Jsi.

**Command-line arguments**:

``` js
jsish -e 'puts("hi", console.args);' -a 1 -b 2 x y z
```

```
hi [ "-a", "1", "-b", "2", "a", "b", "c" ]
```

**User input**:
``` js
var line = console.input();
puts(line);
```

**Output**:
``` js{.line-numbers}
// FILE: output.jsi
puts('puts outputs a string');
printf('printf omits newline\n');
log('log adds line info');
console.puts('console versions go to stderr');
```

```
puts outputs a string
printf omits newline
output.jsi:4:   "log adds line info", 
console versions go to stderr

```

**File I/O**:
``` fs
// FILE: myio.jsi
File.write('myfile.txt', 'some data');
var data = File.read('myfile.txt');
var ch = new Channel('myfile.txt');
puts(ch.read());
```

See [Output Details](#output-details) below.

## Inclusion

Include other code using: `source`, `import`, `load`, `require`:

### source

`source` includes a javascript file  and evaluates inline:
``` js
source('src1.jsi');
source(['src2.jsi','src3.jsi']);
var fun = source('fun.jsi');
fun(1);
```
Not the use of return:
``` js
// FILE: fun.jsi
return function(n) {n+1;}
```

### import

`import` is like source except it executes code in an function-closure.

``` js
var ret = import('exp.jsi');
ret.foo();
```

The `import`-ed code typically uses `export default`.

``` js
// FILE: exp.jsi
function foo() {}
function bar() {}
export default *
```

+++ Forms of Export:

Four forms of `export` are supported:

``` js
export default * // All functions
export default + // ALl functions and vars
export default {func1:func1, func2, func3, var1}

var exps = {func1:func1, func2, func3};
export default exps
```

**Note**:
- `export` must be the final statement.
- `export` is the same as `return`, but needs no semicolon.
- in the above, only the last two forms are ES 6 compatible.

+++

### load

Use `load` for compiled shared libraries:
``` js
load('Baker.so'); // or .dll
```

&#x1f6a9; See Loadable [Extensions](Extensions.md).

### require
`require` just invokes `source`/`load` and is described in [modules require](Modules.md#require).

## Modules

To facilitate option parsing, help, and command-line
invocation in Jsi we use `moduleRun`/`moduleOpts`.

``` js
// FILE: hello.jsi
function hello(args, ...) {
    var self = moduleOpts({a:1, b:2});
    return [args, self];
}

moduleRun(hello);
```

```
jsish hello.jsi -a 1 -b 2 x y z
[ [ "x", "y", "z" ], { a:1, b:2 } ]
```

```
jsish hello.jsi -h
/tmp/hello.jsi:2: help: ...
.  Options are:
	-a		1		
	-b		2		

Accepted by all .jsi modules: -Debug, -Trace, -Test.
```
Note that `moduleRun` invokes the function only if **"hello.jsi"** is first arg to **jsish**.
  
🚩 More on [Modules](Modules.md).

## Help
Jsi has command-line help:

```
jsish -h
USAGE:
  jsish [PREFIX-OPTS] [COMMAND-OPTS|FILE] ...

PREFIX-OPTS:
  --E CODE	Javascript to evaluate before program starts
  --I OPT=VAL	Interp option bits: equivalent to Interp.conf({OPT:VAL}); VAL defaults to true.
  --T 		SApply testMode transforms showing their output: a shortcut for --I testMode=1

COMMAND-OPTS:
  -a		Archive: mount an archive (zip, sqlar or fossil repo) and run module.
  -c		CData: generate .c or JSON output from a .jsc description.
  -d		Debug: console script debugger.
  -e CODE	Evaluate javascript and exit.
  -h ?CMD?	Help: show help for jsish or its commands.
  -m		Module: utility create/manage/invoke a Module.
  -s		Safe: runs script in safe sub-interp.
  -t		Testing of scripts or directories of scripts with .js/.jsi extension.
  -w		Wget: web client to download file from url.
  -v		Version: show version detail: add an arg to show only X.Y.Z
  -z		Zip: append/manage zip files at end of executable.
  -D		DebugUI: web-gui script debugger.
  -J		JSpp: preprocess javascript for web.
  -S		SqliteUI: web-gui for sqlite database file.
  -W		Websrv: web server to serve out content.

Interp options may also be set via the confFile.'
```

as well as interactive help:

```
jsish
Jsish interactive: see 'help [cmd]'.  \ cancels > input.  ctrl-c aborts running script.
$ help load
load(shlib:string):void
Load a shared executable and invoke its _Init call.
```

## Options

The `--I` is used to set internal [Interp](Interp.md) options:
```
jsish --I traceCall=func hello.jsi
```
Two forms of command-line eval include `-e`:

```
jsish -e 'var i = 0; while (i++<10); return i;'
```
which exits after the eval and `--E` which does not:
```
jsish --E 'require("Sqlite");` hello.jsi
```

Two [Debuggers](Debug.md) are available: command-line and GUI.

``` bash
jsish -d buggy.jsi
jsish -D buggy.jsi
```

The builtin [Testing](Testing.md) feature is invoked with `-t`:
```
jsish -t hello.jsi
[PASS] hello.jsi 	 (16.5791015625 ms)
```
or `--T` to just see output.
```
jsish --T hello.jsi 
[ [], { a:1, b:2, c:"a" } ]
```

Most other options invoke script application zip-appended
onto the **jsish** binary.

## Diagnostics

### Logging
Logging provides conditional output useful in diagnosing problems:
```
jsish -W -Trace true wspage.html 
Websrv.jsi:214:  "TRACE: PORT IS: 37667", OpenWebsock()
Websrv.jsi:506:  "TRACE: Listening on port: 0", main()
Websrv.jsi:276:  "TRACE: URL:  http://127.0.0.1:37667/Websrv/wspage.html", OpenBrowser()
Websrv.jsi:80:  "TRACE: FILTER: /Websrv/wspage.html true", WsFilter()
...
```
🚩 More on [Logging](Logging.md).

### Backtrace

Upon encountering an uncaught error, a *backtrace* is generated,
with most recent call last:

    jsish -W
    CALL BACKTRACE:
    #1: Websrv.jsi:531:  in moduleRun()
    #2: Websrv.jsi:526:  in Websrv( [], {} )
    #3: Websrv.jsi:472:  in main()
    
    /zvfs/lib/Websrv.jsi:472: error: throw near url file empty or not found: 
    ERROR: 


### Tracing
Enable tracing lets you see calls and returns:
```
jsish --I traceCall=func,ret hello.jsi
hello.jsi:2:   "#1: > hello() 
hello.jsi:2:   "#1: < hello() 
 <-- [ [], { a:1, b:2 } ][ [], { a:1, b:2 } ]
```

## Output Details

The following are used for output and input:

| Command      | Description                                          |
|--------------|------------------------------------------------------|
| puts         | Quotes arguments to stdout                           |
| printf       | Formatted output to stdout                           |
| log          | Like puts but including file/line info               |
| console.*    | Versions of above commands that output to stderr     |
| File         | File related commands                                |
| Channel      | Output to stderr/stdout channels                     |

See [System](Reference.md#System) and [console](Reference.md#console) for details.
All commands except printf automatically add a newline.

### puts
Quotes all arguments and outputs them to stdout.


``` js
puts("Batman begins");
var X = {a:1, b:"two"};
puts(X);
puts(X.a, X.b);
```

```
Batman begins
{ a:1, b:"two" }
1 two
```
    
### printf
Processes arguments according to format and outputs to stdout.

``` js
printf("Batman begins: %d %S\n", X.a, X.b);
```

### log
: Works much like **puts**, but includes the current file, line and function.

``` js
log("Batman begins");
"Batman begins", file.jsi:1, func()
```

In addition, when the first argument to **log** is a boolean, output appears only if true.

``` js
log(false, "Batman begins");
log(true, "Batman begins");
"Batman begins", file.jsi:2, func()
```

### console

The [console](Reference.md#console) commands output to stderr:

``` js
console.puts("Batman begins");
console.printf("Batman begins: %d %S\n", X.a, X.b);
console.log("Batman begins");
```

### File
The [File](Reference.md#file) commands also handle input/output:

``` js
File.write('myfile.txt', 'hello');
var x = File.read('myfile.txt');
```

### Channel

The [Channel](Reference.md#channel) commands handles program I/O:

``` js
File.write('stderr', 'hello');
var fp = new Channel('stderr');
fp.puts('hello');
```

## Compatibility

Jsi implements most of 
[Ecma-script 262 standard](http://www.ecma-international.org/ecma-262/5.1/ECMA-262.pdf)
version 5.1 with numerous deviations and extensions:

- Semicolons terminations are required for vars and expressions.
- Feature additions such as `for(x of y)`, `=>`, `let` (but unscoped like `var`), and `const`.
- `typeof[]` == **"array"**, not **"object"**.
- Template-strings *(backticks)* work like normal strongs (ie. no `${}`).
- Omits **Error** object: the argument to **catch()** is just a string.
- Omits **Date** object: use **strftime/strptime**.
- **delete** also works on a normal **var**.
- UTF supported in strings only, not code.
- Prototype-related features are intentionally incomplete/wrong.


*[JS]:Javascript Like
