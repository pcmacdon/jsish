Coding
====
<div id="sectmenu"></div>

Modules
----
Jsi Modules can be used either from code or the command-line.

Here is a simple module:

    echo 'function hello() { return "Hello World!"; }; runModule(hello);' > hello.jsi
    $ jsish hello.jsi
 
==>

    Hello World!
   

more complete modules can be with [templates](#Templates).

### Arguments
Modules handle arguments via an array parameter:

    function add(args) { return args.join(', '); }
    jsish -m add.jsi Able Baker Charlie

==>

    Able, Baker, Charlie


### Options
A Jsi module that can handle switches uses <b>parseOpts</b>:

    function add(args, conf) {
        var self = {};
        parseOpts(self, {start:0, name:''}, conf);
        return self.name+args.slice(self.start).join(', ');
    }
    runModule(add);

This places leading switches into **self**:

    jsish foo3.jsi -name 'Dog: ' -start 1 Able Baker Charlie

==>

    Dog: Baker, Charlie

A Jsi module can:

- be invoked from the command-line, accepting arguments and switches, or
- be called programatically by other modules as a [package](#Packages).
- display it's available options when called with -h.


### Templates
A full-featured new module can be created with:

    $ jsish -m -create append
    Created module 'append' in append.jsi
    $ cat append.jsi
     
==>

    #!/usr/bin/env jsish
    
    function append(args:array|string=void, conf:object=void) {
        var options = { // Module description.
            rootdir      :''      // Root directory.
        };
        var self = {
        };
        parseOpts(self, options, conf);
        
        function main() {
            LogTest('Starting', args);
            switch (typeof(args)) {
                case 'string': args = [args]; break;
                case 'array': break;
                default: args = [];
            }
            if (self.rootdir === '')
                self.rootdir=Info.scriptDir();
            debugger;
            LogDebug('Done');
        }
        
        return main();
    }
    
    provide(append, 1);
    
    if (isMain()) {
        if (!Interp.conf('unitTest'))
            runModule(append);
        else {
    ;'append unit-test';
    ;append();
    ;//append('',{badArg:0});
        }
    }

And to test:

    $ jsish append.jsi -rootdir /tmp
    $ jsish --U ./append.jsi
    'append unit-test'
    append() ==> undefined
    append('',{badArg:0}) ==>/home/pmacdona/tmp/append.jsi:9: info: 
    Unknown option: "badArg" is not one of:
      rootdir
    
     PASS: undefined

Note the embedded unit-tests, which we want run only for the *main* module.
This is distinct from integration and functional testing where the focus is wider.

For web-server modules use:

    $ jsish -m -create httpd -web true
    Created module 'httpd' in httpd.jsi


### Help
Modules support "-h" and "-help":

    jsish add.jsi -h

==>

    /home/user/src/ii/add.jsi:3: help: ...
    .  Options are:
         -name -start
    Use -help for long help.

#### Long Help
For longer help we need a more complete module with type-checking and comments:

    function add(args:array, conf:object=void) {
        var self = {};
        var options = { // Concat args into list.
            start:0,    // Start position.
            name:''     // Name prefix.
        };
        parseOpts(self, options, conf);
        return self.name+args.slice(self.start).join(', ');
    }
    runModule(add);

Then comments will be extracted from **options**:

    jsish add.jsi -help

==>

    /home/user/src/ii/add.jsi:7: help: ...
    Concat args into list..  Options are:
        -name       ""      // Name prefix.
        -start      0       // Start position.
    
    Accepted by all .jsi modules: -Debug, -Trace, -Test.


#### Help Limitations

Help information gets extracted from options by parseOpts, with the following limitations:

- The package name will be extracted from the file base name, except when:
- All comments must use the // form.
- The first comment (which must be after the opening {) is the help title.
- There can be no closing } anywhere in the body of the options or comments.



### Structure
The various sections of a module are:

| Section     | Description                                                                                                           |
|-------------|-----------------------------------------------------------------------------------------------------------------------|
| function    | All code is wrapped in a function with the same name as the file basename, providing bindings for incoming arguments. |
| options     | The var options = object itemizes the all options that can come in via the opts parameter.                            |
| self        | Local object state is stored in the self object. All defined and incoming options will be copied into self.           |
| parseOpts   | The call parseOpts(self, options, opts) performs option parsing as well as implementing -h help output                |
| provide     | The provide statement makes the module available as a package.                                                        |
| Info.isMain | The isMain() command returns true if the current file was invoked from the command-line.                       |
| runModule   | The runModule() command parses command-line options and then invokes the current module.                              |



Files
----
Various commands are involved in dealing with files of source code.

    function source(val:string|array, options:object=void):void
    function load(shlib:string):void
    
### source


Include file of javascript.
First argument is either a file or array of files.

    source('scripts/foo.js');
    source(['foo.js'*,*'bar.js']);

A second parameter may be used for setting the following options:


| Option | Type | Description                           |
|--------|------|---------------------------------------|
| debug  | INT  | Debug level.</td> </tr>               |
| auto   | BOOL | Setup for load of autoload.jsi files. |
| isMain | BOOL | Force isMain() to true.               |


Various source related options can be configured/examined in the usual way.
This example changes the recursive maximum depth of source (from it's default of 100).

    Interp.conf({maxIncDepth:10});
    var cur = Info.script();
    var dir = Info.scriptDir();


### load

Load shared library extension into memory.

    On Windows, the extensions .dll is used instead of .so.

    load('libsqlitejs.so');


For more details see [Extensions](CData.md).

Packages
----

Packages are provisioned with **provide** and **require**:

    function provide(name:string, version:number=1):void
    function require(name:string=void, version:number=1):number|array|object

Versions are either a string or float.

- The string form is [MAJOR.MINOR.PATCH](https://semver.org/) where individual values are in the range *0-99*, eg: **"1.2.3"**.
- The number form is in the range *0.0000-99.9999*, eg: **1.0203**.

For example:

    "1.2" ==> 1.02
    "1.2.3"==> 1.0203

The following JS converts from string to number:

    var v = '1.2.3'.split('.').map(parseInt);
    v[0]+v[1]/100.0+v[2]/10000.0; // ==> 1.0203

Or use the Jsi utility:

    Util.verConvert(1.0203);  // ==> 1.2.3
    Util.verConvert('1.2.3'); // ==> 1.0203
    Util.verConvert('1.2');   // ==> 1.02

### provide

Used to make available the current file name for use by **require**.
This file is either:

- a .jsi script having a **provide**, or
- or shared library **.so/.dll** having a **Jsi_PkgProvide()**.

The file must be of the form **NAME.EXT** where
**NAME** may contain only the characters in **[_a-zA-Z0-9]**.

**Note**: require will also look within a self-named subdirectory, eg. **NAME**/**NAME.EXT**

### require

Require is used to load/query packages, where arguments are:

- zero arguments: returns the list of all loaded package names.
- one argument: loads package code (if necessary) and returns its version.
- two arguments: as above but checks the version and returns the version and loadFile in an object.

Package requires are searched for in:

- Any directory in the [pkgDirs](#pkgDirs) Interp option.
- The executable directory of jsish.
- The directory of the current script.

**Note**: a warning is issued if a version argument is given that is greater than the found package version.


### pkgDirs

Packages are searched for are specified by pkgDirs, which can be set pkgDirs using:

    Interp.conf({pkgDirs:['/usr/local/lib', '/usr/share']});

When a package is not found in pkgDirs, the executable and main script directories are checked.

**Note**:
    Sub-interps inherit pkgDirs from the parent.

### Auto-Load
Because it is sometimes desirable to defer loading code until first invocation,
Jsi provides a mechanism
using  member values in the object Jsi_Auto to evaluate.
For example, given the file myfunc.js:

    provide('myfunc');
    function double(n) { return n*2; };
    function half(n)   { return n/2; };

We can arrange for dynamic sourcing with:

    Jsi_Auto.double =
    Jsi_Auto.half   = 'require("myfunc");';
    
    double(4); // Load is triggered!
    half(4);

This also works for object commands:

    Jsi_Auto.MySql = "require('MySql')";
    //...
    var db = new MySql({user:'root', database:'jsitest'});


### Auto-Files

Jsi_Auto will demand-load files by
traversing the list **Interp.autoFiles**, eg:

    Interp.conf('autoFiles');
    [ "/zvfs/lib/autoload.jsi" ]

In addition, if an application is invoked from a **.zip** file, it's **autoload.jsi** file is appended.


#### Object Functions
Autoloading non-constructor object functions can use the following approach
(the example ignores the fact that JSON is not actually loadable):

    var JSON = {
       parse:     function() { require('JSON'); return JSON.parse.apply(this,arguments); },
       stringify: function() { require('JSON'); return JSON.stringify.apply(this,arguments); }
       //...
    }
    JSON.parse(str); // Load is triggered!

Although that works, it suffers from one problem: the first call will not type-check functions.

We could fix this shortcoming with:

    var JSON = {
        check: function(str:string, strict:boolean=false):boolean { require('JSON'); return JSON.check.apply(this,arguments); },
        //...
    };

But correctly transcribing function signatures is more complicated.

Fortunately there is a helper:
:  function Jsi_AutoMake(objName:string):string

    This can be used to generate the auto file:

    File.write( Jsi_AutoMake('JSON'), 'autoload.jsi');

The file autoload.jsi now contains calls and load and type-check correctly:

    var JSON = {
        check: function(str:string, strict:boolean=true):boolean { require('JSON'); return JSON.check.apply(this,arguments); },
        parse: function(str:string, strict:boolean=true):any { require('JSON'); return JSON.parse.apply(this,arguments); },
        stringify: function(obj:object, strict:boolean=true):string { require('JSON'); return JSON.stringify.apply(this,arguments); },
    };


Debugging
----

### Logging
LogDebug statements appear only when a module is run with "-Debug":

    function foo(args, conf) {
        var self = {};
        parseOpts(self, {start:0, name:''}, conf);
        LogDebug('Starting', args);
        return self.name+args.slice(self.start).join(', ');
    }
    runModule(foo);

    jsish foo.jsi -Debug true  a b

Output

    "DEBUG: Starting [ "a", "b" ]", foo.jsi:4, foo()
    a, b

Similarly for LogTrace and LogTest.

When logging is disabled, log op-Codes are ignored by Jsi, so no resources get used.


### Debugger
There are two debuggers.  The command-line:

    jsish -d myscr.jsi arg1

And the Web-GUI:

    jsish -D myscr.jsi arg1

See [debuggers](Debug.md).


### Execution Trace
Jsi provides a number of program tracing options.  Perhaps the easiest to use is from the command-line with:

    jsish --I traceCall tests/module.js

==>

    /home/user/jsi/jsi/tests/module.js:12 #1: > mod([])
    /home/user/jsi/jsi/tests/module.js:12 #1: < mod()  <-- { process:function (a) {...}, x:1, y:2 }
    /home/user/jsi/jsi/tests/module.js:22 #1: > process([ 9 ])
    /home/user/jsi/jsi/tests/module.js:17   #2: > sub([ 10 ])
    /home/user/jsi/jsi/tests/module.js:17   #2: < sub()  <-- 20
    /home/user/jsi/jsi/tests/module.js:22 #1: < process()  <-- 20
    20
    1
    /home/user/jsi/jsi/tests/module.js:36 #1: > fmod([])
    /home/user/jsi/jsi/tests/module.js:36 #1: < fmod()  <-- { process:function (a) {...}, x:1, y:2 }
    /home/user/jsi/jsi/tests/module.js:37 #1: > process([ 9 ])
    /home/user/jsi/jsi/tests/module.js:31   #2: > sub([ 10 ])
    /home/user/jsi/jsi/tests/module.js:31   #2: < sub()  <-- 20
    /home/user/jsi/jsi/tests/module.js:37 #1: < process()  <-- 20
    20
    1

The output may seem overly verbose, but is advantageous when executed
from within geany (or vim) in that we can click to navigate through the file.

If simpler traces are desired, try:

    jsish --I traceCall:funcs,args tests/module.js

Output

    #1: > mod([]) in module.js:12
    #1: > process([ 9 ]) in module.js:22
      #2: > sub([ 10 ]) in module.js:17
    20
    1
    #1: > fmod([]) in module.js:36
    #1: > process([ 9 ]) in module.js:37
      #2: > sub([ 10 ]) in module.js:31
    20
    1


### Code Profile
Jsi can output detailed execution profile information for functions using:

    jsish --I profile  SCRIPT

The following demonstrates this on unix:

    jsish --I profile  /tmp/while2.js   2>&1 | grep ^PROFILE: | sort -g -r -t= -k2

==>

    PROFILE: TOTAL: time=4.169039, func=3.099403, cmd=1.068323, #funcs=10003, #cmds=300001, cover=58.0%, #values=1860447, #objs=610397
    PROFILE:  self=3.000902  all=4.069200  #calls=10000     self/call=0.000300  all/call=0.000407  cover=100.0%  func=foo file=/tmp/while2.js:29
    PROFILE:  self=1.068298  all=1.068298  #calls=300000    self/call=0.000004  all/call=0.000004  cmd=Info.funcs
    PROFILE:  self=0.098484  all=4.167684  #calls=1         self/call=0.098484  all/call=4.167684  cover= 75.0%  func=bar file=/tmp/while2.js:44
    PROFILE:  self=0.000024  all=0.000024  #calls=1         self/call=0.000024  all/call=0.000024  cmd=puts
    PROFILE:  self=0.000017  all=4.167700  #calls=1         self/call=0.000017  all/call=4.167700  cover=100.0%  func=aa file=/tmp/while2.js:27
    PROFILE:  self=0.000002  all=0.000002  #calls=1         self/call=0.000002  all/call=0.000002  cover=  7.0%  func=cc file=/tmp/while2.js:7

All times are in seconds, and output is sorted by self time (descending).

Following is a list of fields in the PROFILE: TOTAL: line:

| Field   | Description                                     |
|---------|-------------------------------------------------|
| time    | Total amount of CPU used by the program run     |
| func    | Total mount of CPU used by functions            |
| cmd     | Total mount of CPU used by commands             |
| #funcs  | Total number of function calls                  |
| #cmds   | Total number of command calls (non-functions)   |
| cover   | Total code coverage in percent (functions only) |
| #values | Total number of Jsi_Value allocations           |
| #objs   | Total number of Jsi_Obj allocations             |

Following is a list of fields in each PROFILE line:

| Field     | Description                                            |
|-----------|--------------------------------------------------------|
| self      | Amount of CPU used by the function                     |
| all       | Amount of CPU used by function and it's descendants    |
| #calls    | Number of times function was called                    |
| self/call | Per-call CPU used by the function                      |
| all/call  | Per-call CPU used by the function and it's descendants |
| cover     | Code coverage for function, in percent                 |
| func      | Name of the function                                   |
| cmd       | Name of the command                                    |
| file      | File and line number of function                       |


### Code Coverage
In addition to the simple coverage statistics available with profile,
detailed code coverage can be obtained with --I coverage, eg:

    jsish --I coverage  /tmp/while2.js   2>&1 | grep ^COVERAGE: | sort -g -r -t= -k4

==>

    COVERAGE: func=bar  file=/tmp/while2.js:48  cover=75.0%  hits=6,  all=8,  misses=56-57
    COVERAGE: func=cc  file=/tmp/while2.js:7  cover=30.0%  hits=4,  all=13,  misses=10-13,18-22
    COVERAGE: func=bb  file=/tmp/while2.js:27  cover=0%

Output is produced only for functions with less than 100% coverage.
Uncalled functions are indicated by cover=0% with remaining fields omitted.

Following is a list of the COVERAGE fields:

| Field  | Description                                       |
|--------|---------------------------------------------------|
| func   | Name of the function                              |
| file   | File and line number for start of function        |
| cover  | Code coverage in percent for the function         |
| hits   | Number of distinct lines executed in the function |
| all    | Total number of executable lines in the function  |
| misses | List of line-ranges not executed                  |



Web
----

### Client
To download a file from the web we can use:

    jsish -w -O jsi-app.zip http://jsish.org/jsi-app/zip

### Server
The builtin web server can open it in a
local browser and serve out html to it, from the command-line:

    jsish -W index.html

and programmatically:

    Jsi_Websrv('index.html');
    update(-1);

Sqlite
----


### Usage

Basic Sqlite usage:

    var db = new Sqlite('mydata.db');
    db.query('SELECT * FROM tbl');
    Output
    [ { a:99, b:1 }, { a:95, b:2 }, { a:91, b:3 } ]

GUI
---
The database GUI is invoked using:

    jsish -S mydata.db

### Database Dump
To dump a database use:

    jsish -S -dump true mydata.db

which produces output like the sqlite .dump command.


Applications
----


### Ledger

    fossil clone http://jsish.org/jsi-app jsi-app.fossil
    jsish -a jsi-app.fossil Ledger

