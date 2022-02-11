Builtins
====
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

Examples and expansions upon [generated reference](Reference.md).

## Channel

A Channel object can be created for reading and/or writing individual files.
Generally an object is instantiated to open the file and then read/write operations are performed.

``` js
var f = new Channel('tests/filetest.txt');
if (!f)
    puts('Can not open tests/filetest.txt');
else {
    while((n = f.gets())!=undefined) {
        puts(n);
    }
}
```

## CData

Used by C extensions functions and for interfacing with C-struct data: See the [Extensions page](Extensions.md).

## Event

Uses Ecmascript standard functions, but
**update** needs to be explicitly called so events get serviced:

### setTimeout

- `function setTimeout(callback:function, msdelay:number):number`

Invoke function after msdelay. Return an event id.

### setInterval

- `function setInterval(callback:function, msdelay:number):number`

Invoke function every msdelay milliseconds. Returns an event id.

### clearInterval

- `function clearInterval(id:number)`

Delete setInterval/setTimeout. 


``` js
function foo() {
    puts("FOO:");
}
setInterval(foo, 1000);
update(-1);
```

Calling update(-1) causes **foo** to be called once a second.

### update

- `function update(options:number|object=void):number`

update/process all pending scheduled events

- With no arguments, update will process all events in the queue then return.
- If a number argument is given, it is treated as the minTime option below.
- If an object argument is given, it may contain any of the options in the following table

| Option    | Description                                                                 |
|-----------|-----------------------------------------------------------------------------|
| maxEvents | Maximum number of events to process                                         |
| maxPasses | Maximum passes through event queue                                          |
| minTime   | Minimum milliseconds before returning, or -1 to loop forever (Default is 0) |
| sleep     | Sleep time between event checks in milliseconds (Default is 1)              |

Events are processed until one of **minTime**, **maxEvents**, or **maxPasses** is exceeded.

The returned value is the number of events processed.


### event

`function Info.event(id:number=void):array|object`

Return event information, which is one of:

- With no args, a list of all events.
- With one arg, the information for the given event.

In Jsi we use Info.event() to query queued events:

``` js
function foo() {
    puts("FOO: " + i++);
    if (i>=3) exit(0);
}
var i=0, id = setInterval(foo,1000);
var evs = Info.event();
for (var i in evs) {
  puts('EV(' + i + '): ' + Info.event(evs[i]).toString());
}
update();
```

+++ Click here to see output:

``` bash
EV(0): { built-in:false, count:0, initial:1000, once:false, type:"timer", when:1000 }
FOO: 0
FOO: 1
FOO: 2
```

+++

The return value:

- With no arg, is the list of ids for queued events.
- With one arg, is the information for the given event id.

Also see [Info](#info).


## File

 methods are used for accessing various
attributes and information on files where just basic IO is not necessarily the goal.

``` js
File.mkdir('XX1');
File.mkdir('XX1/AA');
File.mkdir('XX1/BB');
File.rename('XX1/BB'*,*'XX1/CC.txt');
puts(File.glob(null,'XX1').sort());
puts(File.dirname('XX1/AA'));
puts(File.rootname('XX1/CC.txt'));
puts(File.tail('XX1/CC.txt'));
puts(File.type('XX1/CC.txt'));
puts(File.extension('XX1/CC.txt'));
//puts(File.realpath('XX1/CC.txt'));
puts(File.writable('XX1/CC.txt'));
puts(File.readable('XX1/CC.txt'));
puts(File.exists('XX1/CC.txt'));
puts(File.isdir('XX1/CC.txt'));
puts(File.isfile('XX1/CC.txt'));
File.remove(*'XX1',true);
```

+++ Script output:

```
[ "AA", "CC.txt" ]
XX1
XX1/CC
CC.txt
directory
.txt
true
true
true
true
false
file
```

+++

### glob

- `function glob(pattern:regexp|string|null='*', options:function|object|null=void):array`

The File glob method returns an array of matching files according to the rules:

- With no arguments (or null) returns all files/directories in current directory.
- If first argument is a pattern (either a glob or regexp) just files are returned.
- If second argument is a string, it denotes the directory to search in.
- If second argument is a function, this function is called with each path.
- Otherwise second argument is a set of options.

For example:

``` js
File.glob();
File.glob("*.c");
File.glob(/^jsi.*\.c$/);
File.glob(''*,'tests');
File.glob('.js'*,{dir:'tests', recurse:true});
```

## Info

The info sub-methods are used for accessing various internal attributes and information about Jsi.

``` js
Info.vars('*');
Info.funcs('*');
```

This internal self-inspection capability is referred to as Introspection.
The Reference documentation for Jsi in is generated via the Info command.

See also [Introspection](Misc.md#Introspection)

## Interp

Interface into the interpreter state: See [interps](Interp.md).


## JSON

JSON (JavaScript Object Notation) is an open standard format that
uses human-readable text to transmit data objects consisting of attribute–value pairs.
It is the primary means of exchanging data with web-browsers.

JSON is a non-validating parser providing:

- `function stringify(val:any, strict:boolean=true):string`

Stringify converts a javascript data object to a string:

``` js
var obj = { a:1, b:2, c:"able", d:[ 1, 2, 3 ] };
var str = JSON.stringify(obj);
//RETURNS: '{"a":1, "b":2, "c":"able", "d":[1,2,3]}';
```

- `function parse(str:string, strict:boolean=true):any`

Parse converts a string into javascript data:

``` js
var str = '{"a":1, "b":2, "c":"able", "d":[1,2,3]}';
var obj = JSON.parse(str);
//RETURNS: { a:1, b:2, c:"able", d:[ 1, 2, 3 ] }
```

When strict is false then parse does not require quoted names.

``` js
var str = '{a:1, b:2, c:"able", d:[1,2,3]}';
var obj = JSON.parse(str, false);
```

Non-strict parsing is particularly helpful when encoding [JSON](#json) in C.


## MySql

See [MySql](MySql.md).

## Object
### freeze
`freeze` takes several arguments; this example shows the defaults:
```
var s = {a:1, b:2 };
Object.freeze(s);
var opts = Object.freeze(s,null); // Query freeze state.
s.a = 2; // OK to modify
s.c = 3; // ERROR to assign new key
s.d;     // ERROR to lookup undefined
```
Note that [moduleOpts](Modules.md#moduleopts) implicitly performs a `freeze` on self.

+++ Freeze Arguments

The function signature for `freeze` is:
```
Object.freeze(obj:object, freeze:boolean=true, modifyok:boolean=true, readcheck:boolean=true)
```
The various settings are illustrated in this example:

```
var s = {a:1, b:2 };
Object.freeze(s);
Object.freeze(s,false); // Unfreeze
s.c = 3; // OK
Object.freeze(s,true,false); // disallow  modify
s.a++; // ERROR!
Object.freeze(s,true,true,false); // allow undefined lookup
s.z;   // OK
```

+++


## Signal

Signal is used to send signals to processes, or setup handlers
for receiving signals.  It is currently only supported on unix.

The following signal commands are available:

| Method                 | Description                                 |
|------------------------|---------------------------------------------|
| alarm(secs)            | Setup alarm in seconds                      |
| callback(func,sig)     | Setup callback handler for signal           |
| default(?sig,sig,...?) | Set named signals to default action         |
| handle(?sig,sig,...?)  | Set named signals to handle action          |
| ignore(?sig,sig,...?)  | Set named signals to ignore action          |
| kill(pid?,sig?)        | Send signal to process id (default SIGTERM) |
| names()                | Return names of all signals                 |


## Socket

The Socket extension provides access to TCP and UDP sockets, both client and server.

Following is a simple example.  First we run a server:

``` js
var s = new Socket({server:true,onRecv:puts});
while (1) update();
```

and in another terminial, a client.

``` js
var s = new Socket({noAsync:true});
s.send('hello world\n');
```

For another socket example see [sockdemo.jsi](https://github.com/pcmacdon/jsish/blob/master/js-demos/sockdemo.jsi).

## Sqlite

See [Sqlite](Sqlite.md).


## System

The System object contains all the built-in toplevel commands in Jsi
and are also exported as globals.


### exec

- `function exec(val:string, options:string|object=void):any`

The exec command is used to invoke operating system commands:

``` js
var a = exec('ls -d /tmp');
```

A second argument may be given with the following options:

| Option   | Type   | Description                                                  |
|----------|--------|--------------------------------------------------------------|
| bg       | BOOL   | Run command in background using system() and return OS code. |
| inputStr | STRING | Use string as input and return OS code.                      |
| noError  | BOOL   | Suppress OS errors.                                          |
| noTrim   | BOOL   | Do not trim trailing whitespace from output.                 |
| retAll   | BOOL   | Return the OS return code and data as an object.             |
| retCode  | BOOL   | Return only the OS return code.                              |


The first argument is treated as follows:

- If the command ends with '&', set the 'bg' option to true.
- If the second argument is null, set the 'noError' option to true.
- If the second argument is a string, the 'inputStr' option is set.
- By default, returns the string output, unless the 'bg', 'inputStr', 'retAll' or 'retCode' options are used

In C, when the bg option is true, system() is called, otherwise popen() is used.

For more examples see [exec test](https://github.com/pcmacdon/jsish/blob/master/tests/exec.jsi).


### format

- `function format(format:string, ...):string`

The sprintf command adds printf like functionality to javascript, eg.

``` js
var s, me = 'Me', cnt = 9;
s = format('Help %s = %d!', me, cnt);
```

This sometimes more convenient than the traditional:

``` js
s = 'Help '* + me + *' = ' + cnt + '!';
```

which ends up dealing with a lot of quotes and plus signs.
Format also simplifies aligned output:

``` js
s = format('Help %-20s = %d!', me, cnt);
```

### source

- `function source(val:string|array, options:object=void):any`

Include and interpret javascript files.

``` js
source('myfile.jsi');
source(['file1.jsi', 'file2.jsi']);
```

## WebSocket

See [WebSocket](WebSocket.md).


## Zvfs

Zvfs stands for Zip Virtual File System, which is used by Jsi to read and write zip files.

There are two important uses of Zvfs:

- Running scripts directly from .zip files.
- Running scripts zipped into the jsish binary (zero-install).


In both cases the Zip archive can contain all files (scripts, web pages, images, etc)
required by an application, and thus
can deploy complete working standalone applications.


### Executing Zips
Jsi can execute a zip archive that contains main.jsi  or  lib/main.jsi, eg:

``` bash
jsish DebugUI.zip my.jsi arg1 arg2
```

Jsi mounts the .zip file on /zvfs1, then executes main.jsi (or lib/main.jsi) therein.

Any other resources contained in the archive are also available to Jsi.


### Zero-Install
It is also possible to zip "main.jsi (and other files) directly onto the end of the jsish binary itself.
This lets jsish be used to deploy Zero-Install, standalone application.

The simplest way to create a Zero-Install application is to copy your scripts into lib/ then use:

``` bash
make jsize
```

The direct approach uses the script "tools/mkjsize.js":

``` bash
cp jsish jsize
tools/mkjsize.js create jsize zipdir
```

For further working example applications see [Install#getting/examples].


### Writing
To create a zip archive use:

``` js
Zvfs.create('arch.zip', file.glob('*.js'))
```

This creates a zip file containing all the .js files in the current directory.


### Reading
Jsi can mount .zip files as local filesystem:

``` js
var dir = Zvfs.mount('arch.zip');
File.glob('*', dir);
```

**Note**:
    If a mount point is not given, it is generated in the pattern /zvfsN, where N=1,2,3,...


## console

The Ecmascript-like functions provide output to the console.

### log

- `function log(val, ...):void`

Prints arguments to stderr.

Each argument is quoted (unlike the built-in string concatenation).
If called with 0 or 1 argument, a newline is output, otherwise stderr is flushed.


### puts

- `function puts(val, ...):void`

Same as console.log, except
prints arguments to stdout.
Also, available in [System](Reference.md#System) (and therefore the toplevel).

The puts method outputs a string to stdout.
With 1 or 0 arguments a newline is appended.

``` js
puts("Hello World");
puts("Hello"," World\n");
```

Scripts also used in web browsers can add the following for compatibility:

``` js
if (puts === undefined)
    var puts = console.log.bind(console);
```

### input

- `function input():string`

User input can be obtained using console.input():

``` js
puts("Enter your name: ", "");
var str = console.input();
```


### args
Retrieve program arguments:

``` js
for (var i in console.args) {
   puts(console.args[i]);
}
```

