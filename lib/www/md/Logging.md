Logging
====
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

Debugging output is provided by: `assert LogDebug LogTrace LogTest LogInfo LogWarn LogError`


By default the first 4 of these are elided as **no-ops** and output nothing:

``` js{.line-numbers}
// FILE: logtest.jsi
LogDebug("Start");
LogTrace("Middle");
LogTest ("End");
LogInfo ('Done');
```

```
jsish logtest.jsi
logtest.jsi:5, "INFO:  Done",
```

Output can be enabled globally using `--I log`:

```
jsish --I log=debug,trace logtest.jsi
logtest.jsi:2, "DEBUG:  Start",
logtest.jsi:3, "TRACE:  Middle",
logtest.jsi:5, "INFO:  Done",
```

## Elides
A key feature of `LogDebug`, `LogTrace`, `LogTest`, and `assert`
is that when disabled **they and their arguments** are truly ignored.

```
jsish -e 'var cnt = 0; LogDebug("one:", cnt++); return cnt;'
0
```

In the above this means the first **cnt++** in `LogDebug` did not evaluate and so outputs "**0**".
In contrast to:
```
jsish --I log=debug -e 'var cnt = 0; LogDebug("one:", cnt++); return cnt;'
:1:   "DEBUG: one: 1", LogDebug()
1
```

This encourages *leaving-in debug statements*.



## Modules
[Modules](Start.md#modules) have their own logging environment:

``` js{.line-numbers}
// FILE: mymod.jsi
function mymod(args, ...) {
    moduleOpts();
    LogDebug("MYLOC 1:", args.join(','));
    LogTrace("MYLOC 2:", args.join(','));
}

LogDebug("MYGLOB 1");
LogTrace("MYGLOB 2");

module(mymod);
```

Logging can be enabled via module-options:

```
jsish mymod.jsi -Debug true A B C
mymod.jsi:4:   "DEBUG: MYLOC 1: A,B,C", mymod()

jsish --I log=debug mymod.jsi -Trace true A B C
mymod.jsi:8:  "DEBUG: MYGLOB 1", 
mymod.jsi:4:  "DEBUG: MYLOC 1: A,B,C", mymod()
mymod.jsi:5:  "TRACE: MYLOC 2: A,B,C", mymod()
```

## Mask/Enable

When a package is released into production, we typically
mask out global logging: 

``` js
provide(Websrv, '1.2.3', {logmask:'debug,trace'});
```

Likewise, we can enable logging using `require`:

```
jsish --E 'require("mymod", 1, {log:"debug"});' mymod.jsi A B C
```

This also works with C-extensions.

``` js{.line-numbers}
// FILE: dbapp.jsi
var a=1, b=2, x, db = new Sqlite();
db.eval('CREATE TABLE foo(a,b)');
db.query('INSERT INTO foo(a,b) VALUES(@a,@b)');
x = db.query('SELECT * FROM foo');
puts(x);
```

```
jsish --E 'require("Sqlite", 1, {log:"debug,trace"});' dbapp.jsi 
dbapp.jsi:2: debug: Starting DB    (c-extn [Sqlite])
dbapp.jsi:3: trace: SQL-EVAL: CREATE TABLE foo(a,b)    (c-extn [Sqlite])
dbapp.jsi:4: trace: SQL-QUERY: INSERT INTO foo(a,b) VALUES(@a,@b)    (c-extn [Sqlite])
dbapp.jsi:5: trace: SQL-QUERY: SELECT * FROM foo    (c-extn [Sqlite])
[ { a:1, b:2 } ]
```


## Tracing
The interp option [logOpts](Reference.md#interp-logopts) is used to control when *Log*
outputs details such as the current file, line and function.

If you are debugging a program and need to find where a certain puts is coming from, try adding to the top:

``` js
Interp.conf({tracePuts:true});
```


The same can be done from the command-line:

```
jsish --I tracePuts tests/assert.js
"caught error" --> assert.js:16
"K" --> assert.js:24
"caught error2" --> assert.js:28
"this assert failed" --> assert.js:31
"assert also failed" --> assert.js:34
"done" --> assert.js:36
```

## assert
The assert command is used for constraint checking.

- When disabled (the default) none of it's arguments will evaluate.
- When enabled and the expression evaluates to false, an error is thrown.
- When expression is a function, its returned value is used instead.

eg.

``` js
Interp.conf({log:'assert'});
var m = 1, n = -1;
assert(m>0, "too small");
assert(n>=0 && n<100, "passed bad n", true);
```

There are several control options for **assert**,
including using *puts* instead of throwing an error:
    
``` js
// FILE: myassert.jsi
var m = 0;
assert(++m<0, 'bad stuff', {mode:'puts'});
puts(m);
Interp.conf({assertMode:'puts'});
assert(++m<0, 'not bad');
assert(++m<0);
Interp.conf({log:'!asserts'});
assert(false, 'ignored');
```

```
jsish --I log=assert myassert.jsi
bad stuff
1
not bad
assert(++m<0)
```

## Conditionals

**Log** commands are further filterable when the first argument a boolean,
where false filters it out:

``` js{.line-numbers}
// FILE: conds.jsi
var i = 0;
LogInfo(false, 'wont');
LogInfo(true, 'will');
LogTest(i++>0, 'one ', i);
LogInfo(i++>0, 'two:', i);
LogInfo(i++>0, "three:", i);
```

```
jsish conds.jsi;
conds.jsi:4:   "INFO:  will", LogInfo()
conds.jsi:7:   "INFO:  three: 2", LogInfo()
```
## Navigation
[Geany](Building.md#geany) can navigate
through *Log* messages the same as with compiler warnings.

By making your script executable, you can run it directly from Geany with F9.

You can then employ jsish as an replacement for /usr/bin/env,
with support for arguments:

``` js
#!/usr/local/bin/jsish --I log=debug %s -Trace true myinput1.txt
puts(console.args.join(' '));
```

See [Shell](Building.md#shell)

<!-- meta:{"file":{"index":9}} -->
