Debug
=====
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

## Debuggers


Two debuggers come builtin to Jsi: command-line and GUI.

``` bash
jsish -d myscr.jsi arg1
jsish -D myscr.jsi arg1
```

The debugger will stop at **debugger** statements, and anytime an error or warning occurs (ie. in "strict mode").


### Commands
The usual debugger commands are supported:

| Command  | Argument           | Description                                          |
|----------|--------------------|------------------------------------------------------|
| break    | file:line/func     | Break execution at a named function, line or file    |
| continue |                    | Continue execution                                   |
| delete   | id                 | Delete one or all breakpoints                        |
| disable  | id                 | Disable one or all breakpoints                       |
| down     | count              | Move down one or more stack levels                   |
| enable   | id                 | Enable one or all breakpoints                        |
| eval     | expression         | Evaluate expression in program context               |
| finish   |                    | Run till return of current function                  |
| help     | string             | Display command usage                                |
| info     | args/locals/bp/var | Show status info                                     |
| list     | proc/line count    | List file lines                                      |
| next     |                    | Single-step over function calls                      |
| print    | name               | Print value of variable                              |
| quit     |                    | Quit debugging current program and exit              |
| step     |                    | Single-step into function calls                      |
| tbreak   |                    | Set a temporary breakpoint that is disabled when hit |
| up       | count              | Move up one or more stack levels                     |
| where    |                    | Display current location                             |
| xeval    | expression         | Does eval in debugger context                        |

### Example Session

For example:

``` js
// Simple test script for the debugger.
function foo2() {
  debugger;
}

function foo1() {
  var x = 99;
  foo2();
}

function foo() {
  var x = 88;
  foo1();
}

foo();
puts("SIMPLE");
```

a sample debugging session is:


``` bash
jsish -d /tmp/simple.js
#1:/tmp/simple.js:16  foo();
#1==> s
CMD: step
#2:/tmp/simple.js:12    var x = 88; <in function foo()>
#2==>
#2:/tmp/simple.js:13    foo1(); <in function foo()>
#2==>
#3:/tmp/simple.js:7    var x = 99; <in function foo1()>
#3==> c
CMD: continue
#4:/tmp/simple.js:3    debugger; <in function foo2()>
#4==> up
CMD: up
#3:/tmp/simple.js:8    foo2(); <in function foo1()>
#3==> p x
CMD: print
RESULT= "99"
#3==> up
CMD: up
#2:/tmp/simple.js:13    foo1(); <in function foo()>
#2==> p x
CMD: print
RESULT= "88"
#2==>
```

Note that just like gdb, an empty commands repeats the previous command.


### Listing
Source file lines can be displayed with the list command:

``` bash
jsish -d /tmp/simple.js
#1:/tmp/simple.js:16  foo();
#1==> c
CMD: continue
#4:/tmp/simple.js:3    debugger; <in function foo2()>
#2==> l 1 20
CMD: list
FILE: /tmp/simple.js:1
1    : // Simple test script for the debugger.
2    : function foo2() {
3    :   debugger;
4    : }
5    :
6    : function foo1() {
7    :   var x = 99;
8    :   foo2();
9    : }
10   :
11   : function foo() {
12   :   var x = 88;
```


With no arguments the default is to list 10 lines starting from the current line
in the current file.
Optional arguments are **startLine|func numLines file**



### Print
The print command can be used to display the value of variables:

``` bash
jsish -d /tmp/simple.js
#1:/tmp/simple.js:16  foo();
#1==> s
CMD: step
#2:/tmp/simple.js:12    var x = 88; <in function foo()>
#2==> s
CMD: step
#2:/tmp/simple.js:13    foo1(); <in function foo()>
#2==> p x
CMD: print
RESULT= "88"
```

### Eval
The eval command can be used to evaluate expresssions or modify values:

``` bash
jsish -d /tmp/simple.js
#1:/tmp/simple.js:16  foo();
#1==> c
CMD: continue
#4:/tmp/simple.js:3    debugger; <in function foo2()>
#4==> up
CMD: up
#3:/tmp/simple.js:8    foo2(); <in function foo1()>
#3==> p x
CMD: print
RESULT= "99"
#3==> ev x=9
CMD: eval
RESULT= 9
#3==> p x
CMD: print
RESULT= "9"
```

### Breakpoints
Breakpoints can be set, cleared and queried as in the following example:

``` bash
jsish -d /tmp/simple.js
#1:/tmp/simple.js:16  foo();
#1==> s
CMD: step
#2:/tmp/simple.js:12    var x = 88; <in function foo()>
#2==> b
CMD: break
breakpoint #1 set: /tmp/simple.js:12
#2==> b foo
CMD: break
breakpoint #2 set: foo
#2==> c
CMD: continue
Stopped at breakpoint #2
#2:/tmp/simple.js:13    foo1(); <in function foo()>
#2==> info
CMD: info
#1    : enabled=true, hits=0, file=/tmp/simple.js:12
#2    : enabled=true, hits=1, func=foo
#2==> dis 1
CMD: disable
#2==> info
CMD: info
#1    : enabled=false, hits=0, file=/tmp/simple.js:12
#2    : enabled=true,  hits=1, func=foo
```

Note that the debugger will stop whenever a debugger statement is encountered.


### Where
The where command outputs a stack backtrace:

``` bash
jsish -d /tmp/simple.js
#1:/tmp/simple.js:16  foo();
#1==> c
CMD: continue
#4:/tmp/simple.js:3    debugger; <in function foo2()>
#4==> wh
CMD: where
{ fileName:"/tmp/simple.js", funcName:"foo2", level:4, line:3 }
{ fileName:"/tmp/simple.js", funcName:"foo1", level:3, line:8 }
{ fileName:"/tmp/simple.js", funcName:"foo", level:2, line:13 }
{ fileName:"/tmp/simple.js", funcName:"", level:1, line:16 }
#4==> up 2
CMD: up
#2:/tmp/simple.js:13    foo1(); <in function foo()>
#2==> wh
CMD: where
{ fileName:"/tmp/simple.js", funcName:"foo", level:2, line:13 }
{ fileName:"/tmp/simple.js", funcName:"", level:1, line:16 }
#2==>
```

### Implementation

The debugger runs the target script in a [sub-interpreter](Interp.md).

The sources are here:

- [Command-line](https://jsish.org/fossil/jsi/file/lib/Debug.jsi)
- [Gui](https://jsish.org/fossil/jsi/file/lib/DebugUI/DebugUI.jsi)

**Warning**:
    Jsi bends the memory management rules of Sub-interps a bit to make debugging mode work,
    which means a small amount of memory may leak
    during debug sessions.

## Debugging Aids



### Error Diagnostics
Jsi does not generate tracebacks upon error.  Instead it provides gcc style warnings that
contain the file and line number.
For example:

``` js
var x = 1;
foo(x)
/home/user/myjsi/foo.js:2: error: 'foo', sub-commands are: Array Boolean Date File
   Function Interp JSON Math Number Object RegExp Sqlite String Websocket alert
   assert clearInterval console decodeURI encodeURI exit file format include info
   isFinite isNaN load parseFloat parseInt puts quote setInterval setTimeout signal
   sys util zvfs.
```

The file and line number is reported, as well as an enumeration of known commands
in the given scope.
This allows errors to be [parsable by IDE's](Building.md#Editors).


### Method Introspection
Upon error, an objects sub-methods are enumerated, if possible:

``` js
Info.xx()
Output
error: 'info', sub-commands are: cmds data error events executable funcs
    named platform revisions script vars version.    (at or near string "xx")
```

### Displayed Arguments

And arguments to builtin methods part of the diagnostic:

``` js
format();
error: missing args, expected "format(format?,arg,arg?)"
```
