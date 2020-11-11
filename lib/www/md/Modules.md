Modules
====
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

## Usage

Using [module](#module)/`[moduleOpts](#moduleopts) provides option parsing, help, and command
invocation.

### Minimal
Consider the module `hello.jsi`:

``` js
function hello(args, ...) {
    var self = moduleOpts( {m:0}, {a:1, b:2} );
    return [args, self];
}

module(hello);
```

invoked from the command-line:
```
jsish hello.jsi -a 3 x y z
[ [ "x", "y", "z" ], { a:3, b:2 } ]
```
or programatically:
```
jsish -e 'require("hello"); hello(["x"])'
Hello [ "x"] { a:1, b:2: m:0 }
```
or help via **-h**:
```
jsish hello.jsi -h
/hello.jsi:2: help: ...
.  Options are:
    -a      1       
    -b      2       

Accepted by all .jsi modules: -Debug, -Trace, -Test.
```

### Expands

Expand options to get help from comments:

``` js
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
add.jsi -h
/tmp/add.jsi:9: help: ...
Concat args into list.
Options are:
    -name       ""      // Name prefix. {}
    -start      0       // Start position.

Accepted by all .jsi modules: -Debug, -Trace, -Test, -Assert.
```

### Logging
Modules support logging via `LogDebug`, `LogTrace`, `LogTest`
``` js
function buggy(args, ...) {
    moduleOpts();
    LogDebug('Starting', args);
    var r = 'buggy: '+ args.join(', ');
    LogTrace('Returning', r);
    return r;
}
LogTrace('buggy:', console.args);
module(buggy);
```
Output appears if enabled, in-module:

```
jsish buggy.jsi -Trace true  a b
buggy.jsi:5:   "TRACE: Returning buggy: a, b", buggy()
"buggy: a, b"
```

or globally:

```
jsish --I log=Trace,Debug buggy.jsi a b
buggy.jsi:8:   "TRACE: buggy: [ "a", "b" ]", 
buggy.jsi:3:   "DEBUG: Starting [ "a", "b" ]", buggy()
buggy.jsi:5:   "TRACE: Returning buggy: a, b", buggy()
"buggy: a, b"
```

Importantly, when not enabled arguments are elided and do not evaluate.

🚩 See [Logging](Logging.md).

### Freeze

A side effect of using `moduleOpts`, is `self` gets [frozen](Builtins.md#freeze).

``` js{.line-numbers}
// FILE: frost.jsi
function frost(...) {
    var self = moduleOpts({a:1, b:2}, {m:0});
    self.b += self.m++;
    if (self.a)
        self.m = self.x;
    self.y = 1;

module(frost);
```

```
jsish frost.jsi
CALL BACKTRACE:
#1: frost.jsi:2:  in moduleRun()
#2: frost.jsi:6:  in frost( [], {} )

/tmp/frost.jsi:6: error: object freeze: read undefined "x"
ERROR: 
```

```
jsish frost.jsi -a 0
CALL BACKTRACE:
#1: frost.jsi:2:  in moduleRun( "-a", "0" )
#2: frost.jsi:7:  in frost( [], { a:0 } )

/tmp/frost.jsi:7: error: object freeze: attempted assign "y"
ERROR: 
```



## Directives

### module
``` js
function module(cmd:string|function, version:number|string=1, options:object=void):void
```

`module` is like [provide](#provide), but also invokes when `isMain`==true.

``` js
function hello2(args, ...) {
    return "Hello World: "+args.join(', ');
};

module(hello2);
```

### moduleOpts
`moduleOpts` handles leading switch options:

``` js
function hello3(args, conf) {
    var self = moduleOpts({start:0, name:''});
    return [self, args];
}
module(hello3);
```

which are checked and placed into object **self**:

```
jsish hello3.jsi -name 'Dog: ' -start 1 Able Baker Charlie
[ { name:"Dog: ", start:1 }, [ "Able", "Baker", "Charlie" ] ]
```

To recap, a module:

- may be invoked from the command-line, accepting arguments and switches.
- can be called invoked programatically by other modules as a package.
- uses **-h** display dump its options.



### module

`module` defines and/or invokes a module:

``` js
function hello2(args, ...) {
    return "Hello World: "+args.join(', ');
};
provide(hello2);
if (Info.isMain())
    module('hello2', ['-num' 99, 'a',2] );
```

### provide

``` js
function provide(cmd:string|function, version:number|string=1, options:object=void):void
```

Packages defined **provide** and
versions are either a string or float:

```
provide('buggy', 1.0101); // or...
provide('buggy', "1.1.1");
```

An optional 3rd argument can be used:

``` js
provide(buggy, 1, {logmask:'debug,trace', info:{rel:'prod'}});
```
which is retrievable using `require` with version **0**:
```
jsish -e 'require("buggy", 0)'
{ conf:{ coverage:false, info:{ rel:"prod" }, log:[], logmask:[ "debug", "trace" ], 
    nofreeze:false, profile:false, traceCall:[], udata:null }, func:"function buggy(args) {...}",
    lastReq:0, loadFile:"/tmp/buggy.jsi", name:"buggy", status:null, verStr:"2.0.0", version:2 }
```

+++ Provide Details

Version format:

- The string form is [MAJOR.MINOR.PATCH](https://semver.org/) where individual values are in the range *0-99*, eg: **"1.2.3"**.
- The number form is in the range *0.0000-99.9999*, eg: **1.0203**.

For example:

``` js
"1.2" ==> 1.02
"1.2.3"==> 1.0203

// The following JS converts from string to number:
var v = '1.2.3'.split('.').map(parseInt);
v[0]+v[1]/100.0+v[2]/10000.0; // ==> 1.0203

// Or use the Jsi utility:
Util.verConvert(1.0203);  // ==> 1.2.3
Util.verConvert('1.2.3'); // ==> 1.0203
Util.verConvert('1.2');   // ==> 1.02
```

Used to make available the current file name for use by **require**.
This file is either:

- a .jsi script having a **provide**, or
- or shared library **.so/.dll** having a **Jsi_PkgProvide()**.

The file must be of the form **NAME.EXT** where
**NAME** may contain only the characters in **[_a-zA-Z0-9]**.

**Note**: require will also look within a self-named subdirectory, eg. **NAME**/**NAME.EXT**

+++

### require
``` js
function require(name:string=void, version:number=1):number|array|object
```
Require loads a package made available with `provide` or `moduleRun`,
and returns its version or, when version arg=**0**, the package details.
 
Note we can also enable features such as logging via require:
```
jsish --E 'require("buggy", 1, {log:"debug"});' buggy.jsi A B C
```


+++ Require Details

Require is used to load/query packages, where arguments are:

- zero arguments: returns the list of all loaded package names.
- one argument: loads package code (if necessary) and returns its version.
- two arguments: as above but checks the version and returns the version and loadFile in an object.

Package requires are searched for in:

- Any directory in the `pkgDirs` Interp option.
- The executable directory of jsish.
- The directory of the current script.

**Note**: a warning is issued if a version argument is given that is greater than the found package version.

+++

## Help
Modules help information extracted from options by `moduleOpts` where:

- Comments must use `//`, not `/* */`.
- The help title comes from the first comment after the opening `{`.

<!-- meta:{"file":{"index":5}} -->
