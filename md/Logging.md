Logging
====
<div id="sectmenu"></div>
# Logging
Several facilities are available for logging program output to *stdout* or *stderr*.

## Output
The following commands are useful for *unconditional* program output:

| Command      | Description                                          |
|--------------|------------------------------------------------------|
| puts         | Quotes arguments to stdout                           |
| printf       | Formatted output to stdout                           |
| log          | Like puts but including file/line info               |
| console.*    | Versions of above commands that output to stderr     |
| Channel      | Output to stderr/stdout channels                     |

See [System](Reference.md#System) and [console](Reference.md#console) for details.
All commands except printf automatically add a newline.

### puts
Quotes all arguments and outputs them to stdout.


    puts("Batman begins");
    var X = {a:1, b:"two"};
    puts(X);
    puts(X.a, X.b);

==>

    Batman begins
    { a:1, b:"two" }
    1 two
    
### printf
Processes arguments according to format and outputs to stdout.

    printf("Batman begins: %d %S\n", X.a, X.b);


### log
: Works like **puts**, but includes the current file, line and function.

    log("Batman begins");

==>

    "Batman begins", file.jsi:12, func()

### console.*

The console commands output to stderr:

    console.puts("Batman begins");
    console.printf("Batman begins: %d %S\n", X.a, X.b);
    console.log("Batman begins");


### Channel

The standard channels may also be used for program output:

    File.write('stderr', 'hello');
    var fp = new Channel('stderr');
    fp.puts('hello');


## Conditional

These **Log** commands are for *conditional* output,
and includes the file and line number:

    LogError LogWarn LogInfo LogDebug LogTrace LogTest

Only the first 3 of these are enabled by default, eg.

    var i = 0;
    while  (i++ < 3) {
        LogInfo("test loop:", i);
    }
    LogDebug("Done");

==>

    mytest.jsi:3, "INFO:  test loop: 1",
    mytest.jsi:3, "INFO:  test loop: 2",
    mytest.jsi:3, "INFO:  test loop: 3",


### Debug
The latter 3 commands, **LogDebug**, **LogTrace**, and **LogTest**, are disabled by default:

    var i = 0;
    while  (i++ < 2) {
        LogDebug("test loop:", i);
        LogTrace("test loop:", i);
        LogTest("test loop:", i);
    }

Output can be made to appear with these options:

    jsish --T Debug,Trace,Test mydebug.jsi

==>

    mydebug.jsi:3, "DEBUG: test loop: 1",
    mydebug.jsi:4, "TRACE: test loop: 1",
    mydebug.jsi:3, "DEBUG: test loop: 2",
    mydebug.jsi:4, "TRACE: test loop: 2",

Note, that these commands are special because,
they incur [zero overhead](#Overhead) (their op-codes are ignored) when disabled.

### Control
Logging can be controlled via options from command-line:

    jsish --T Debug,Trace mydebug.jsi

or programatically with a ***use*** directive or [Interp.conf](Interp.md):

    $ jsish
    "use !Debug";
    LogDebug("can't appear");
    Interp.conf({logOpts:{Debug:true, Info:false}});
    LogDebug("can appear");
    LogInfo("can't appear");


### Modules
Larger applications in Jsi make use of [modules](Coding.md#modules),
which has builtin support for **Log**:

    function mycall(args:array=void, conf:object=void) {
        var options = {};
        var self = { cnt:0 };
        parseOpts(self,options,conf);
    
        LogDebug("MYLOC 1:", args.join(','));
        LogTrace("MYLOC 2:", args.join(','));
    }
    
    LogDebug("MYGLOB 1");
    LogTrace("MYGLOB 2");
    provide();
    if (isMain())
        runModule(mycall);

Note logging can be enabled within function:

    $ jsish mycall.jsi -Debug true A B C
    mycall.jsi:6,   "DEBUG: test 1: A,B,C", mycall()

Or globally:

    $ jsish --T Debug mycall.jsi -Trace true A B C
    mycall.jsi:10,  "DEBUG: MYGLOB 1"
    mycall.jsi:6,   "DEBUG: MYLOC 1: A,B,C", mycall()
    mycall.jsi:7,   "TRACE: MYLOC 2: A,B,C", mycall()


### Alternatives
A fallback way to enable **Log**
(other than globally enabling it with **"--T"**)
is with **require**:

    jsish -e 'require("mycall", 1, {Debug:true});' mycall.jsi A B C

This lets you enable logging on a per-module basis.

### Overhead
An important aspect of **LogDebug**, **LogTrace**, **LogTest**, **assert**
is that they *and their arguments* incur no overhead until enabled.

This at first may be confusing, as in this example:

    var i = 0, cnt = 0;
    while  (i++ < 3) {
        LogDebug("test loop:", i, ":", cnt++);
    }
    printf("cnt=%d\n", cnt);

where **cnt++** does not evaluate when run with:

    $ jsish myincr.jsi
    cnt=0

but does evaluate, when run with **Debug**:

    $ jsish --T Debug myincr.jsi
    myincr.jsi:3, "DEBUG: test loop: 1 : 0",
    myincr.jsi:3, "DEBUG: test loop: 2 : 1",
    myincr.jsi:3, "DEBUG: test loop: 3 : 2",
    cnt=3

This zero-overhead feature is meant to encourage their widespread use.

### Tracing
The interp option [logOpts](Reference.md#logOptsOptions) is used to control when *Log*
outputs details such as the current file, line and function.

If you are debugging a program and need to find where a certain puts is coming from, try adding to the top:

    Interp.conf({tracePuts:true});


The same can be done from the command-line:

    $ jsish --I tracePuts tests/assert.js
    "caught error" --> assert.js:16
    "K" --> assert.js:24
    "caught error2" --> assert.js:28
    "this assert failed" --> assert.js:31
    "assert also failed" --> assert.js:34
    "done" --> assert.js:36


### Navigation
[Geany](Download.md#geany) can navigate
through *Log* messages the same as with compiler warnings.

By making your script executable, you can run it directly from Geany with F9.

You can then employ jsish as an replacement for /usr/bin/env,
with support for arguments:

    #!/usr/local/bin/jsish --T Debug %s --Trace true myinput1.txt
    puts(console.args.join(' '));

See [Shell](Download.md#shell)

## assert
The assert command is used for constraint checking.

- When disabled (the default) none of it's arguments will evaluate.
- When enabled and the expression evaluates to false, an error is thrown.
- When expression is a function, its returned value is used instead.

eg.

    "use asserts";
    var m = 1, n = -1;
    assert(m>0, "too small");
    assert(n>=0 && n<100, "passed bad n", true);

There are several control options for **assert**,
including using *puts* instead of throwing an error:
    
    $ jsish --T asserts
    var m = 0;
    assert(++m<0, 'bad stuff', {mode:'puts'});
    puts(m);
    Interp.conf({assertMode:'puts'});
    assert(++m<0, 'not bad');
    assert(++m<0);
    Interp.conf({asserts:false});
    assert(false, 'ignored');
    
==>

    bad stuff
    1
    not bad
    assert(++m<0)



