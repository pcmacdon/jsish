Functions
=====
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

Functions in Jsi can optionally use types to provide *checking* of arguments.

``` js
function func(a:number):number { return a+1; }
```

## Checking


Here is a type checked function call:

``` js{.line-numbers}
// FILE: foo.jsi
function foo (a:number, b:string):number {
   return a+1;
}
foo('a', 9);
```
Which kicks an error at runtime:

```
jsish foo.jsi
/tmp/foo.jsi:5: parse: type mismatch for argument arg 2 'foo': expected "string" but got "number", in call to 'foo'
ERROR: 
```

For more warnings use `noerror`:
```
jsish --I noError  /tmp/foo.jsi
/tmp/foo.jsi:5: warn: type mismatch for argument arg 2 'foo': expected "string" but got "number", in call to 'foo'
/tmp/foo.jsi:5: warn: type mismatch for argument arg 1 'a': expected "number" but got "string", in call to 'foo' declared at foo.jsi:2.2 <a>.
/tmp/foo.jsi:5: warn: type mismatch for argument arg 2 'b': expected "string" but got "number", in call to 'foo' declared at foo.jsi:2.2 <9>.
/tmp/foo.jsi:5: warn: type mismatch returned from 'foo': expected "number" but got "string", in call to 'foo' declared at foo.jsi:2.2 <a1>.
```

**Note**: Since variables can not be *typed*, checking is limited to run-time only.


## Defaults

Parameters can also be given default values:

``` js
function foo (a, b=1) {}
function bar (a=1, b=2) {}
foo(9);
bar();
```

A default value can be one of the primitives:

| Type      | Description                    |
|-----------|--------------------------------|
| number    | A double floating point literal|
| boolean   | A boolean literal              |
| string    | A string literal               |
| null      | The keyword null               |
| undefined | The keyword undefined          |
| void      | The keyword void               |

Note: if a parameter is given a default value, all subsequent are as well:

``` js
function bar (a=1, b=true, c='ok', d=null, e=void, f=void) {}
```

## Unions

A type-union *parameter* has multiple types separated by "|". eg:

``` js
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
```

Similarly the *return* can be a union:

``` js
function foo (x):number|string {
    return x+x;
}
```


## Argc

Using at least one type in a function enables **argc** checking:

``` js;line-numbers;
function bar (a, b) { return a+1; }
function foo (a, b):number { return a+1; }
bar(9);
foo(9);
/tmp/foobar.js:4: warning: incorrect arg count in function call "foo()": got 1 args, but expected 2
```

To avoid such warnings add an ellipsis "...":

``` js
function fool (a:number, b:number, ...) {
   return console.args.length;
}
foo(9,9,9);
```

Argument checking can be globally disabled with:

``` bash
jsish --I noCheck
```


## Implicit Types

Assigning a default value also implicitly assigns a type (except void).
Thus the following are equivalent:

    function quick(a:number=1) {}
    function quick(a=1) {}

Types or-together, meaning the following accepts *string*, *number* or *boolean*:

    function duck(a:number|boolean='quack') {}

## Supported Types

Here is a list of available types:

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


## Any / Void


The default-value **void** is used to indicate that an argument may be ommitted.

``` js
function bag(a=void) {}
var xyz;
bag();
bag(xyz);
/tmp/big.jsi:4: warning: call with undefined var for argument arg 1 'a
```

To allow calls with an undefined argument, give a default-value of *undefined*:

``` js
function fig(a=undefined) {}
var xyz;
fig(xyz);
```

For return types, a function with a **void** must return nothing:

``` js
function foo (a:number):void {
   return;
}
```

Whereas one that can return anything uses **any**:

``` js
function foo (n):any {
   return (n?99:"Ok");
}
```

**Warning**:
    A function return type can not be **undefined**.

                                                       |


There are several ways type-checking is influenced:

- Run with the command-line option: `jsish --T noError ...`
- Run with environment var: `JIS_INTERP_OPTS='{typeCheck:XXX}' jsish ...`
- Inline code: `Interp.conf({noCheck:true});`

Some examples are, from the command-line:

    jsish --I typeCheck=noreturn foo.jsi

or in-script with [Interp](Interp.md):

    Interp.conf({typeCheck:['noreturn','nowith']});

**Note**:
Type checking will silently disables itself after 50 warnings, or the value set using:

    Interp.conf({typeWarnMax:1000});



## Limitations

Although parse time checking can be enabled,
it is no where close to the kind of type-checking available in C.

The first reason is that function arguments may
be variables which are untyped.
Thus only primitives arguments can be checked.

Secondly, at parse time calls to a function can not be checked prior to that functions definition.
One option is a forward declaration, as in:

``` js
function f(a:number):number{} // Forward declaration.

function b() {
    f(1,2);
}

function f(a:number):number {
    return a+1;
}
```

Builtin commands have prototypes, which is basically why there is no problem statically checking them.

## Arrow

Arrow functions work in Jsi:

``` js
a => a+1
(a,b) => a+b
() => { return true; }
```

One limitation is that single parameters can not use parentheses.

    (a) => a+1   // ERROR!

Also types and default-values are not supported.

    (a:number) => a+1 // ERROR!


## Miscellaneous

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
