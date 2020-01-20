Builtins
====
<div id="sectmenu"></div>
Builtins expands on the [generated reference](./Reference.md),
which each section header points to.

[Channel](Reference.md#Channel)
----
A Channel object can be created for reading and/or writing individual files.
Generally an object is instantiated to open the file and then read/write operations are performed.

    var f = new Channel('tests/filetest.txt');
    if (!f)
        puts('Can not open tests/filetest.txt');
    else {
        while((n = f.gets())!=undefined) {
            puts(n);
        }
    }
    

[CData](Reference.md#CData)
----
Used by C extensions functions and for interfacing with C-struct data: See the [CData page](CData.md).

[Event](Reference.md#Event)
----

As in Ecmascript, events are scheduled using the standard functions:

### setTimeout

Invoke function after msdelay.
Returns the event handle.

### setInterval

Invoke function every msdelay milliseconds.
Returns the event handle.

### clearInterval

Delete function invocation.
Updates needs to be explicitly called so that events will be serviced:

    function foo() {
        puts("FOO:");
    }
    setInterval(foo, 1000);
    update(-1);

Calling update(-1) causes **foo** to be called once a second.

### update

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

Return event information, which is one of:

- With no args, a list of all events.
- With one arg, the information for the given event.

In Jsi we use Info.event() to query queued events:

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

==>

    EV(0): { built-in:false, count:0, initial:1000, once:false, type:"timer", when:1000 }
    FOO: 0
    FOO: 1
    FOO: 2

The return value:

- With no arg, is the list of ids for queued events.
- With one arg, is the information for the given event id.

Also see [Info](#Info).


[File](Reference.md#File)
----
 methods are used for accessing various
attributes and information on files where just basic IO is not necessarily the goal.

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

==>

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


### glob
The File glob method returns an array of matching files according to the rules:

- With no arguments (or null) returns all files/directories in current directory.
- If first argument is a pattern (either a glob or regexp) just files are returned.
- If second argument is a string, it denotes the directory to search in.
- If second argument is a function, this function is called with each path.
- Otherwise second argument is a set of options.

For example:

    File.glob();
    File.glob("*.c");
    File.glob(/^jsi.*\.c$/);
    File.glob(''*,'tests');
    File.glob('.js'*,{dir:'tests', recurse:true});


[Info](Reference.md#Info)
----
The info sub-methods are used for accessing various internal attributes and information about Jsi.

    Info.vars('*');
    Info.funcs('*');

This internal self-inspection capability is referred to as Introspection.
The Reference documentation for Jsi in is generated via the Info command.

See also [Introspection](Misc.md#Introspection)

[Interp](Reference.md#Interp)
----
Interface into the interpreter state: See [interps](Interp.md).


[JSON](Reference.md#JSON)
----
JSON (JavaScript Object Notation) is an open standard format that
uses human-readable text to transmit data objects consisting of attribute–value pairs.
It is the primary means of exchanging data with web-browsers.

The JSON object provides the following methods:

function stringify(val:any, strict:boolean=true):string

The stringify() method converts a javascript data object to a string:

    var obj = { a:1, b:2, c:"able", d:[ 1, 2, 3 ] };
    var str = JSON.stringify(obj);
    //RETURNS: '{"a":1, "b":2, "c":"able", "d":[1,2,3]}';

function parse(str:string, strict:boolean=true):any

The parse() method converts a string into javascript data:

    var str = '{"a":1, "b":2, "c":"able", "d":[1,2,3]}';
    var obj = JSON.parse(str);
    //RETURNS: { a:1, b:2, c:"able", d:[ 1, 2, 3 ] }

When strict is false then parse()
that does not require quoting of names.

    var str = '{a:1, b:2, c:"able", d:[1,2,3]}';
    var obj = JSON.parse(str, false);

Non-strict parsing is particularly helpful when encoding [JSON](#JSON) in C.

**Warning**:
    The underlying parser is not a validating parser.


[MySql](Reference.md#MySql)
----
Used to access MySql databases: See [js-mysql](MySql.md).


[Signal](Reference.md#Signal)
----
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


[Socket](Reference.md#Socket)
----
The Socket extension provides access to TCP and UDP sockets, both client and server.

Following is a simple example.  First we run a server:

    var s = new Socket({server:true,onRecv:puts});
    while (1) update();

and in another terminial, a client.

    var s = new Socket({noAsync:true});
    s.send('hello world\n');

For another socket example see [sockdemo.jsi](../js-demos/sockdemo.jsi?mimetype=application/javascript).

[Sqlite](Reference.md#Sqlite)
----
Used to access Sqlite3 databases: See [Sqlite page](Sqlite.md).


[System](Reference.md#System)
----
The System object contains all the built-in toplevel commands in Jsi
and are also exported as globals.


### exec
The exec() command is used to invoke operating system commands:

    var a = exec('ls -d /tmp');

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

For more examples see [exec test](../tests/exec.jsi?mimetype=application/javascript).


### format
The sprintf command adds printf like functionality to javascript, eg.

    var s, me = 'Me', cnt = 9;
    s = format('Help %s = %d!', me, cnt);

This sometimes more convenient than the traditional:

    s = 'Help '* + me + *' = ' + cnt + '!';

which ends up dealing with a lot of quotes and plus signs.
Format also simplifies aligned output:

    s = format('Help %-20s = %d!', me, cnt);


### source
Include and interpret javascript files.

    source('myfile.jsi');
    source(['file1.jsi', 'file2.jsi']);

[WebSocket](Reference.md#WebSocket)
----
The WebSocket extension uses libwebsockets to implement
bidirectional socket communication with web-browsers.

When used in conjunction with [web](Web.md), [Sqlite](Sqlite.md) and [JSON](#JSON),
it is easy to implement browser based applications.


The following creates a minimal client and server using WebSockets.
First the server file ws.js:

    function ws_input(data, id) {
        puts("ws_input: "* + id + *": " + data);
    };
    
    var ws = new WebSocket({callback:ws_input});
    var msg = { str:"whos there?", cnt:0 };
    while (true) {
        update(1);
        if ((msg.cnt++ % 10) == 0)
           ws.send(JSON.stringify(msg));
    }

Next the client file: wsc.js:

    function wsc_input(data) {
        puts("wsc_input: " + data);
    };
    
    var ws = new WebSocket({client:true, callback:wsc_input});
    var msg = { str:"knock knock", cnt:0 };
    
    while (true) {
        msg.cnt++;
        ws.send(JSON.stringify(msg));
        update(1);
    }

Which we run with:

    jsish ws.js &
    jsish wsc.js

There are several ways to use Web in Jsi, all of which ultimately use
the builtin WebSocket api


[Zvfs](Reference.md#Zvfs)
----
Zvfs stands for Zip Virtual File System, which is used by Jsi to read and write zip files.

There are two important uses of Zvfs:

- Running scripts directly from .zip files.
- Running scripts zipped into the jsish binary (zero-install).


In both cases the Zip archive can contain all files (scripts, web pages, images, etc)
required by an application, and thus
can deploy complete working standalone applications.


### Executing Zips
Jsi can execute a zip archive that contains main.jsi  or  lib/main.jsi, eg:

    # jsish DebugUI.zip my.jsi arg1 arg2

Jsi mounts the .zip file on /zvfs1, then executes main.jsi (or lib/main.jsi) therein.

Any other resources contained in the archive are also available to Jsi.


### Zero-Install
It is also possible to zip "main.jsi (and other files) directly onto the end of the jsish binary itself.
This lets jsish be used to deploy Zero-Install, standalone application.

The simplest way to create a Zero-Install application is to copy your scripts into lib/ then use:

    make jsize

The direct approach uses the script "tools/mkjsize.js":

    cp jsish jsize
    tools/mkjsize.js create jsize zipdir

For further working example applications see [Install#getting/examples].


### Writing
To create a zip archive use:

    Zvfs.create('arch.zip', file.glob('*.js'))

This creates a zip file containing all the .js files in the current directory.


### Reading
Jsi can mount .zip files as local filesystem:

    var dir = Zvfs.mount('arch.zip');
    File.glob('*', dir);

**Note**:
    If a mount point is not given, it is generated in the pattern /zvfsN, where N=1,2,3,...


[console](Reference.md#console)
----


log
---
Same as puts, except
prints arguments to stderr.

Each argument is quoted (unlike the built-in string concatenation).
If called with 0 or 1 argument, a newline is output, otherwise stderr is flushed.


### puts
Same as console.log, except
prints arguments to stdout.
Also, available in [System](Reference.md#System) (and therefore the toplevel).

The puts method outputs a string to stdout.
With 1 or 0 arguments a newline is appended.

    puts("Hello World");
    puts("Hello"," World\n");

Scripts also used in web browsers can add the following for compatibility:

    if (puts === undefined)
        var puts = console.log.bind(console);


### input
User input can be obtained using console.input():

    puts("Enter your name: ", "");
    var str = console.input();


### args
Program arguments are available using console.args:

    for (var i in console.args) {
       puts(console.args[i]);
    }

C-API
-----
An API for C programmers: See [C-API page](C-API.md).
