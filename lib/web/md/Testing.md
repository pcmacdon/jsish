Testing
====
[Back to Index](Index.md "Goto Jsi Documentation Index")

Jsi's builtin test facility (`-t` and `--T`) provides a simplistic approach to code verification.

## Scripts
A Jsi *test script* simply marks *testable lines* with leading/trailing semicolons:

``` js{.line-numbers}
// FILE: test.jsi
function test(n) { return "TEST: "+n; }
;test('this is a test');
;test('this to');
test('but not this');
```

Run with:

```
jsish --T test.jsi
test('this is a test') ==> TEST: this is a test
test('this to') ==> TEST: this to
```

```
jsish -t test.jsi
[PASS] test.jsi
```


In a simple test like this, we get a `[PASS]` simply because there were no errors.

### [PASS]

To make the test more useful run:

```
jsish -t -update 1 test.jsi
```

which appends an `EXPECT` comment:

``` js
/*
=!EXPECTSTART!=
test('this is a test') ==> TEST: this is a test
test('this to') ==> TEST: this to
=!EXPECTEND!=
*/
```

Now the output is still the same `[PASS]`, but now the output must be a match.


### [FAIL]
To cause a mismatch *(and failure)* change line **3** to:
``` js
;test('this is a BAD test');
```

to induce a `[FAIL]`:

```
jsish -t  test.jsi 
[FAIL] test.jsi
at line 1 of output:
	output:	<test('this is a BAD test') ==> TEST: this is a BAD test>
	expect:	<test('this is a test') ==> TEST: this is a test>
====================DIFFSTART
-test('this is a test') ==> TEST: this is a test
+test('this is a BAD test') ==> TEST: this is a BAD test
 test('this to') ==> TEST: this to
 
====================DIFFEND
```

To locate the source line number use `-find BAD`:
```
jsish -t -find BAD test.jsi
test('this is a BAD test') ==> /tmp/test.jsi:3:   "TEST: this is a BAD test", 
```

### Exceptions
Tests containing exceptions are somewhat verbose:

``` js
try {
    foo(); puts('[FAIL]!: expected a throw');
} catch(e) {
    puts('[PASS]!:',e);
};
```

In Jsi, exceptions can be more succinctly expressed using test-prefix `;//`:

``` js
// FILE: trys.jsi
function bar(n) {}
;//  foo();
;//  bar();
;//  bar(1);
```

which will rewrite to `try`/`catch`. Note that this test is a `[FAIL]` as the last line is not an exception as claimed. 

```
jsish --T trys.jsi
foo() ==>
[PASS]!: err = can not execute expression: 'foo' not a function
bar() ==>
[PASS]!: err = got 0 args, expected 1, calling function bar(n)
bar(1) ==>
[FAIL]!: expected a throw
```

To properly fail, change the last line to:
```
;//  bar(1,1);
```

Now the result throws as expected.  Unfortunately it is not yet testable:

```
jsish -t trys.jsi
[FAIL]:! trys.jsi: Exceptions require an EXPECT comment; use -update
```

As indicated, we need to run update:

```
jsish -t -update 1 trys.jsi
jsish -t trys.jsi
[PASS]
```
The reason for this limitation is simple: unless in test mode, exceptions are just comments
which do nothing.

### Comments
Comments that need to appear in the output are further delimited single-quotes: `;'  ';`.

``` js
;'This is a comment';
```

which outputs:

```
'This is a comment'
```

Placing comments throughout a large test file can help when tests fail:

``` js{.line-numbers}
// FILE: test3.jsi
function a(n) {
    var sum = 0;
    for (var i = 0; i < n; i++)
        sum = sum + i;
    return sum;
};

;'Small Tests';
;a(10);
;a(100);
;'\nBig Tests';
;a(1000);
;'End';
```
These now appear in the output:
```
jsish --T test3.jsi
'Small Tests'
a(10) ==> 45
a(100) ==> 4950
'
Big Tests'
a(1000) ==> 499500
'End'

```

This is more useful when used with `-echo` to get line numbers:
```
jsish -t -echo 1
/tmp/test3.jsi:9:   "'Small Tests'", 
a(10) ==> /tmp/test3.jsi:10:  "45", 
a(100) ==> /tmp/test3.jsi:11:  "4950", 
/tmp/test3.jsi:12:  "'
Big Tests'", 
a(1000) ==> /tmp/test3.jsi:13:  "499500", 
/tmp/test3.jsi:14:  "'End'",
```
### Restrictions


Test lines have the following limitations:

- There must be a semicolon at the start and end of line.
- Functions and var declarations are not allowed.
- Backticks are not allowed.



## Advanced

### Relative Dirs

To eliminate directory paths from error messages add:
```
;Interp.conf({logOpts:{ftail:true}});
```

### Time
To get execution time for a script use `-time`:

```
jsish -t -time true  /tmp/test2.jsi
[PASS] /tmp/test2.jsi 	 (13 ms)
```

To get execution-times + line-numbers for all *self-echo* lines, use:

```
jsish -t -times true  /tmp/test2.jsi
Test /tmp/test2.jsi 
#2: x=1 ==> 1
 (times = 0.000071 sec)
#3: ++x ==> 2
 (times = 0.000085 sec)
```

### Directories
Entire directories can be tested:

``` js
jsish -t tests/
[PASS] tests/49.jsi 	 (2.89599609375 ms)
[PASS] tests/99bottles.jsi 	 (14.162109375 ms)
[PASS] tests/alias.jsi 	 (5.173828125 ms)
[PASS] tests/alias2.jsi 	 (5.14599609375 ms)
[PASS] tests/apply.jsi 	 (2.119140625 ms)
...
[PASS] tests/yhsj.jsi 	 (3.77294921875 ms)
0 FAIL, 97 PASS: runtime 21551 ms
```

The return code is the number of failed tests (up to a maximum of 127):

```
echo $?
0
```

*Note*: When multiple test are run, there is a summary, but no DIFF (unless context>3).

### Inputs
A script needing *test-input* (as if read from stdin) can use:

``` js
/*
=!INPUTSTART!=
45
4950
=!INPUTEND!=
*/
```


### Arguments
A script needing *input-arguments* can use:

``` js
/*
=!ARGSSTART!=
-debug 1 able baker charlie
=!ARGSEND!=
*/
```

### Assert
Note that asserts are normally disabled in Jsi, but not in *test-mode*:

``` js
#!/usr/local/bin/jsish -t %s
Interp.conf({log:{assert:true}}); // note: test mode already enables this.

assert(true,'true');
assert(2*3 == 6,'math');
try {
    assert(false,'false');
} catch(e) {
    puts('caught error');
}
;Interp.conf({asserts:false});
var x = 1;
;x;
;assert(false,'false2');
;assert(false===true);
;Interp.conf({asserts:true});

var i=1, j=2;
;assert(function () { return (i < j); },'fail');

try {
    assert(false==true);
} catch(e) {
    puts('caught error2: '+e);
}
try {
;   assert(false,'false');
} catch(e) {
    puts('caught error2: '+e);
}

;assert(false,'this assert failed',{mode:'puts', noStderr:true});

;Interp.conf({assertMode:'puts', noStderr:true});

;assert(true===false);
;assert(false,'assert also failed');
```

As shown above, there are various ways to configure how *assert* behaves.

### Options
Here is options help:

``` js
jsish -t -h
/zvfs/lib/Testing.jsi:35: help: ...
Run script(s) as unit-tests; return code is the number of failed tests..  Options are:
	-args		""		    // Argument string to call script with
	-context	99		    // Number of context lines for DIFF (>3 forces dir diff).
	-echo		false		// Run with puts/assert output showing file:line number.
	-evalFile	""		    // File to source in subinterp before test.
	-exec		false		// Use exec instead of running test in a sub-interp.
	-expectFile	null		// File to use for expected output.
	-failMax	0		    // Quit after this many failed tests.
	-find		""		    // Enables echo and display only lines containing string or /REGEX/f.
	-inFile		null		// File to use for stdin input.
	-noproto	false		// Disable OOP keyword support: prototype, constructor, etc
	-silent		false		// Run quietly, showing only failures.
	-show		false		// If true, only shows output and omits PASS/FAIL check.
	-time		true		// Show execution time for each test file.
	-times		false		// Output execution time for each individual test line: no PASS/FAIL check.
	-update		false		// In-place update/create of EXPECT* from a run of test file(s).
	-verbose	false		// Echo values of inputs, outputs, and args.
```

### Embedding
Scripts can embedded tests that avoid execution outside of *test-mode*:

``` js
if (Interp.conf('testMode')) {
;    DoCall1();
;    DoCall2();
}
```

### Preprocess
*Test-mode* uses inline *preprocessing* for testing, but you can define your own for whatever purpose:

``` js
cat > input.jsi <<EOF
:abcd:
:1234:
EOF

cat > preprocess.jsi <<EOF
function preprocess(str:string) { return format("puts('PP:', \"%s\");", str); }
Interp.conf({jsppChars:'::', jsppCallback:preprocess, testMode:3});
source(console.args);
EOF

jsish preprocess.jsi input.jsi

==>

PP: abcd
PP: 1234
```

Keeping the reformat to a single line means that error and warning line numbers will line up.

Of course, you could achieve the same thing using **eval**, or generating a preprocessed file and **source**-ing it.

### Navigation

For Geany users, you can click-to-navigate output with F9 by
adding to top of script:

    #!/usr/local/bin/jsish -t -echo true %s

You can also try **-verbose** for even more detail.

### Builtins
To browse the builtin self-test files, look [here](https://jsish.org/fossil/jsi/dir?name=tests).


### Execution Trace
Jsi provides a number of program tracing options.  Perhaps the easiest to use is from the command-line with:

```
jsish --I traceCall tests/module.js
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
```

The output may seem overly verbose, but is advantageous when executed
from within geany (or vim) in that we can click to navigate through the file.

If simpler traces are desired, try:

```
jsish --I traceCall:funcs,args tests/module.js
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
```

### Code Profile
Jsi can output detailed execution profile information for functions using:

``` bash
jsish --I profile  SCRIPT
```

The following demonstrates this on unix:

```
jsish --I profile  /tmp/while2.js   2>&1 | grep ^PROFILE: | sort -g -r -t= -k2
PROFILE: TOTAL: time=4.169039, func=3.099403, cmd=1.068323, #funcs=10003, #cmds=300001, cover=58.0%, #values=1860447, #objs=610397
PROFILE:  self=3.000902  all=4.069200  #calls=10000     self/call=0.000300  all/call=0.000407  cover=100.0%  func=foo file=/tmp/while2.js:29
PROFILE:  self=1.068298  all=1.068298  #calls=300000    self/call=0.000004  all/call=0.000004  cmd=Info.funcs
PROFILE:  self=0.098484  all=4.167684  #calls=1         self/call=0.098484  all/call=4.167684  cover= 75.0%  func=bar file=/tmp/while2.js:44
PROFILE:  self=0.000024  all=0.000024  #calls=1         self/call=0.000024  all/call=0.000024  cmd=puts
PROFILE:  self=0.000017  all=4.167700  #calls=1         self/call=0.000017  all/call=4.167700  cover=100.0%  func=aa file=/tmp/while2.js:27
PROFILE:  self=0.000002  all=0.000002  #calls=1         self/call=0.000002  all/call=0.000002  cover=  7.0%  func=cc file=/tmp/while2.js:7
```

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

``` bash
jsish --I coverage  /tmp/while2.js   2>&1 | grep ^COVERAGE: | sort -g -r -t= -k4

==>

COVERAGE: func=bar  file=/tmp/while2.js:48  cover=75.0%  hits=6,  all=8,  misses=56-57
COVERAGE: func=cc  file=/tmp/while2.js:7  cover=30.0%  hits=4,  all=13,  misses=10-13,18-22
COVERAGE: func=bb  file=/tmp/while2.js:27  cover=0%
```

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



