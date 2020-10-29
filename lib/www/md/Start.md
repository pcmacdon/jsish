Start
====

[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

Jsi is a **javascript**-ish interpreter with builtin websocket-server, sqlite and **C**-extensibility.
Binary downloads for linux/win are available:

    wget http://jsish.org/bin/jsish &&  chmod u+x jsish
    wget http://jsish.org/bin/jsish.exe

Or you can build from source:
 
    wget http://jsish.org/jsi.zip && unzip jsi.zip;  cd jsi;  make
 
&#x1f6a9; See [Building](Building.md).

## Basics

Jsi implements most of **Ecma-script 5.1** (see [Compatibility](#compatibility))
with deviations.

### Functions
Function arguments *may* use **types** / **defaults**:
``` js
function foo(a:number, b:string='') {}
function bar(a,b) {}
```
&#x1f6a9; See [Functions](Functions.md).

Arrow functions are also supported.
```
var f = (x,y) => { puts(x,y); };
var g = (x,y) => x*y;
```
+++ Arrow Limitations

Jsi's simple parser requires that single-args have no braces:
```
var e =  (x,y)  => { return x*y; }; // CORRECT
var f =  x  => { return x*2; };     // CORRECT
var g = (x) => { return x*2; };     // ERROR!
```

Also, arrow function parameters can not have types.


++++

### Variables

Variables can be declared using:
```
var x = 0;
const z = 0;
let y = 0;
```
Note however that `let` does the same thing as `var`: it is not scoped.

### Objects

Shorthand object initializers are accepted, but the first element
must use a colon:
```
var b = 1, c = 2;
var o = {a:1, b, c, myfunc() { return 1;} };
```

Also, an extended Object.[freeze](Builtins.md#freeze) is supported. 

## Modules

Using `module`/`moduleOpts` provides option parsing, help, and command
invocation.

``` js
// FILE: add.jsi
function add(args, ...) {
    const options = { // Concat args into list.
        name:'',    // Name prefix.
        start:0,    // Start position.
    };
    var self = {
        max:4,
    };
    moduleOpts(options, self);
    return [args, self];
}
module(add);
```

```
jsish add.jsi a b c -name dog
[ [ "a", "b", "c", "-name", "dog" ], { max:4, name:"", start:0 } ]
```

```
jsish add.jsi -h
/tmp/add.jsi:9: help: ...
Concat args into list.
Options are:
	-name		""		// Name prefix. {}
	-start		0		// Start position.

Accepted by all .jsi modules: -Debug, -Trace, -Test, -Assert.
```
  
🚩 See [Modules](Modules.md).

### Output

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
jsish output.jsi
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

See [Input/Output](#input/output) below.

## Includes

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
🚩 See [Logging](Logging.md).

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

## Input/Output

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
version 5.1 with numerous additions, ommissions and deviations:

- **Additions**:
    - Typed function parameters.
    - ES6 features: `=>`, `for(x of y)`, `const`, and `let` (but unscoped like `var`).
    - `Info`, `Interp`, and other system builtins.
- **Omissions**: 
    - no *Automatic Semicolon Insertion*.
    - no **Error** *(arg to `catch` is a string)*.
    - no **Date** *(supplanted by `strftime`/`strptime`)*.
- **Deviations**:
    - `typeof[]` == **"array"** and `typeof null` ==  **"null"**, 
    - Backticks work like normal strings (ie. ignores `${}`).
    - UTF supported in strings, not code.
    - Prototype implementation is incomplete.

The main syntactic difference is semicolons are required:

``` js
function foo() {
    var x = 1;
    return ++x;
}
```
**not**
``` js
function foo() {
    var x = 1
    return ++x
}
```

### Conditionals

Jsi will check for undefined vars used on the left-hand side (LHS) of conditional expressions.
To such avoid errors, put the var on the RHS. 

``` js
// FILE: us.jsi
var x;
if ('' === x) puts('x empty'); //OK.
if (x === '') puts('x empty');
/tmp/us.jsi:4: error: lhs value undefined in ===/!==
ERROR
```

## Shortcomings

The following is a known memory leak:

``` js
var x = {};
x.x=x;
```

Solving this would require python style garbage collection: http://www.arctrix.com/nas/python/gc/.


*[JSish]:JS-ish, or Jsi for short, is a Javascript-Like Interpreter

<!-- meta:{"file":{"index":501, "navindex":1}, "project":{"label":"JSI", "title":"jsish.org home page", "href":"https://jsish.org" }} -->
