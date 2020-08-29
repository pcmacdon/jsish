Types
=====
<div id="sectmenu"></div>

Functions
=========

Standard
----
Functions are usable in the standard way:

    function foo(a) { return a+1; }
    var bar = function(a) { return a+1; };

See the [next section](#Types) for details on typed parameters.

Arrow
----
Basic arrow functions are also supported:

    a => a+1
    (a,b) => a+b
    () => { return true; }

Note that single parameters can not use parentheses, due to a limitation of the Jsi parser.

    (a) => a+1   // ERROR!

Also note that arrow functions is basic: it does not support typed, default-valued or destructured parameters.

Types
====

Types and default values can be used in function-parameters:

    function foo (a:number, b:string='ok'):number {
       return a+1;
    }
    foo('a', 9, 10);

which when run:

    jsish foo.jsi

gives warnings:

    /tmp/foo.js:4: warning: got 3 args, expected 1-2, calling function foo(a:number, b:string="ok")    (at or near "a")
    /tmp/foo.js:4: warning: type mismatch for argument arg 1 'a':  "string" is not a "number"    (at or near "a")
    /tmp/foo.js:4: warning: type mismatch for argument arg 2 'b':  "number" is not a "string"    (at or near "a")
    /tmp/foo.js:2: warning: type mismatch returned from 'foo':  "string" is not a "number"    (at or near "a")

**Note**: only function parameters/returns may use *types*: there are no *typed* variables.


Type Names
----
The list of available types includes:

| Type      | Description                                |
|-----------|--------------------------------------------|
| number    | A double floating point value              |
| boolean   | A boolean value                            |
| string    | A string value                             |
| function  | A javascript function                      |
| object    | A javascript object                        |
| array     | A javascript array                         |
| regexp    | A regular expression                       |
| userobj   | A userobj command, eg. from new Socket() |
| null      | A javascript null                          |
| undefined | A value that is undefined                  |
| void      | Argument ommitted/no value returned        |
| any       | Means any value is accepted/returned       |


Type Unions
----
It is not uncommon for a parameter to want to accept more than one type.
For that we can use type-unions, multiple types separated with a pipe "|" character. eg:

    function foo (a:number, b:number|string|boolean) {
        var typ = (typeof b);
        switch (typ) {
            case "number": return b+1;
            case "string": return b;
            case "boolean": return -1;
            default: throw "unhandled type: "+typ;
        }
    }
    foo(9,'a');
    foo(9,9);
    foo(9,true);

Similarly return types can also use unions:

    function foo (x):number|string {
        return x+x;
    }

**Note**:
    Union types are used extensively by [builtin commands](Reference.md).


Argument Count
----
In standard javascript any number of arguments may be passed in a function call,
irregardless of the parameter list.

This is also true in Jsi when a function has no types.
But the addition of at least one type activates checking for that function:

    function bar (a, b) { return a+1; }
    function foo (a, b):number { return a+1; }
    bar(9);
    foo(9);

==>

    /tmp/foobar.js:4: warning: incorrect arg count in function call "foo()": got 1 args, but expected 2

Extra argument warnings can be avoided by adding an ellipsis "...":

    function fool (a:number, b:number, ...) {
       return console.args.length;
    }
    foo(9,9,9);

It is also possible to enable argument count checking for untyped functions,
by setting typeCheck mode [all](#Checking).


Defaults
----
Default values allow functions to be called with fewer parameters, as in:

    function foo (a, b=1) {}
    function bar (a=1, b=2) {}
    foo(9);
    bar();

A default value must be one of the primitives:

| Type      | Description                   |
|-----------|-------------------------------|
| number    | A double floating point value |
| boolean   | A boolean value               |
| string    | A string value                |
| null      | The null value                |
| undefined | An undefined value/var        |
| void      | Argument may be ommitted      |

Also note that when a parameter is given a default value, all following it must as well,
possibly using void:

    function bar (a=1, b=true, c='ok', d=null, e=void, f=void) {}

Implicit Types
----
Assigning a default value also implicitly assigns a type (except void).
Thus the following are equivalent:

    function quick(a:number=1) {}
    function quick(a=1) {}

Types or-together, meaning the following accepts *string*, *number* or *boolean*:

    function duck(a:number|boolean='quack') {}


Any/Void/Undefined
----

The default-value **void** is used to indicate that an argument may be ommitted.

    function bag(a=void) {}
    var xyz;
    bag();
    bag(xyz);

==>

    /tmp/big.jsi:4: warning: call with undefined var for argument arg 1 'a

To allow calls with an undefined argument, give a default-value of *undefined*:

    function fig(a=undefined) {}
    var xyz;
    fig(xyz);


For return types, a function with a **void** must return nothing:

    function foo (a:number):void {
       return;
    }


Whereas one that can return anything uses **any**:

    function foo (n):any {
       return (n?99:"Ok");
    }


**Warning**:
    A function return type can not be **undefined**.



Strict Mode
----
Strict mode, which is enormously helpful in finding bugs, is enabled
by default.

Strict-mode will generates errors for:

- Functions accidentally createing a global variable (by forgetting to use var).
- Calling functions  with the wrong number of parameters or an *undefined* var.
- Use an *undefined* var is used in on LHS of a binary expression.

Strict-mode also enables *type-checking* for functions, eg:

    function foo (a:number, b:string='ok'):number {}

The various checking modes are described below.

Checking
----
The checking mode can be one or more of:

| Type    | Description                                                                     |
|---------|---------------------------------------------------------------------------------|
| parse   | Turn on parse-time checking                                                     |
| run     | Turn-on run-time checking                                                       |
| all     | Both parse and run, plus argument checking for untyped functions                |
| error   | Runtime warnings are errors                                                     |
| strict  | Same as all and error                                                           |
| funcsig | Emit warnings for named function calls with no prior declaration                |
| noundef | Disable strict warnings for calls to function with void passed an undefined var |
| nowith  | Disable using with expressions                                                  |
| asserts | Enable run-time assert() checking. See [assert](Logging.md#assert)              |
| Debug   | Enable LogDebug() output. See [logging](Logging.md)                             |
| Trace   | Enable LogTrace() output                                                        |
| Test    | Enable LogTest() output                                                         |


There are several ways type-checking is influenced:

- A file extension of ".jsi" will set it to **run**.
- The first line starting with **#!** will set it to **parse,run,all**.
- The first line (or second after a #!) is a **use** directive.
- Run in interactive mode, which uses **warn**.
- Run with the command-line option: `jsish --T strict ...`
- Run with environment var: `JIS_INTERP_OPTS='{typeCheck:XXX}' jsish ...`
- Inline code: `Interp.conf({typeCheck:XXX});`

Some examples are, from the command-line:

    jsish --T parse,run foo.jsi

or in-script with [Interp](Interp.md):

    Interp.conf({typeCheck:['error','run','parse']});

**Note**:
Type checking will silently disables itself after 50 warnings, or the value set using:

    Interp.conf({typeWarnMax:1000});



Limitations
----
Although parse time checking can be enabled,
this is no where close to the kind of type-checking available in C.

The first reason for this is that function arguments are frequently
javascript variables which are dynamically typed.
Thus only primitives arguments can be checked.

Secondly, at parse time calls to a function can not be checked prior to that functions definition.
One option is a forward declaration, as in:

    function f(a:number):number{} // Forward declaration.
    
    function b() {
        f(1,2);
    }
    
    function f(a:number):number {
        return a+1;
    }

Builtin commands have prototypes, which is basically why there is no problem statically checking them.


Miscellaneous
----
The rules for defining types in functions are:

- Zero or all parameters should be given types.
- Default values must be primitives.
- If a parameter is given a default value, all parameters following it must as well.
- Only functions with at least one type will be type-checked (except in mode all).

If a function has no types or defaults, it is by default treated as normal javascript,
and there will be no type-checking.

**Note**:
    Builtin commands make extensive use of typed parameters.

In the Jsi code-base, these apply to:

- C-Command declarations via Jsi_CmdSpec.
- C-Option declarations via Jsi_OptionSpec.
- Sqlite and MySql parameter bindings.
