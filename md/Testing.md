Testing
====
<div id="sectmenu"></div>
Testing
----
*Testing* involves running scripts in one of Jsi's *test-modes*, eg:

    cat > /tmp/test1.jsi <<EOF
    puts('FOO');
    EOF
    
    jsish -u /tmp/test1.jsi

==>

    [PASS] /tmp/test1.jsi

The ***PASS*** here simply means there was no error in the script.
While this is not that useful per-se, read on...

### Modes
There are 3 options for running **jsish** in *test-mode*:

- **-u** : run a full *Unit-Test*: output is PASS or FAIL.
- **--U**: run in *test-mode*, but just show the output.
- **--V**: like **--U**, but show file/line number of test.

Rewrites
----
*Test-mode* provides several lines-rewrite facilities, which echo an expression and it's result.

### Semicolon

The simplest *test-mode* rewrite is for lines starting and ending with semicolons:

    cat > /tmp/test2.jsi <<EOF
    var x;
    ;x=1;
    ;++x;
    EOF
    
    jsish --U /tmp/test2.jsi

==>

    x=1 ==> 1
    ++x ==> 2


Note that:

- semicolons must be in the first and last column
- backtick strings are not allowed,
- and no output appears when not in *test-mode*.

### Comments
Comments are lines inside semicolon/single-quote pairs:

    ;'This is a comment';

which outputs:

    'This is a comment'

Placing comments throughout a large test file can help when tests fail:

    cat > /tmp/test3.jsi <<EOF
    function a(n) {
        var sum = 0;
        for (var i = 0; i < n; i++)
            sum = sum + i;
        return sum;
    };
    
    ;'===Begin Test===';
    ;a(10);
    ;a(100);
    ;a(1000);
    ;'===End Test===';
    
    /*
    =!EXPECTSTART!=
    '===Begin Test==='
    a(10) ==>  45
    a(100) ==>  4950
    a(1000) ==>  499500
    '===End Test==='
    =!EXPECTEND!=
    */
    EOF
    
    jsish --U /tmp/test3.jsi

==>

    '===Begin Test==='
    a(10) ==>  45
    a(100) ==>  4950
    a(1000) ==>  499500
    '===End Test==='


### Try-Catch
When a test is known to throw an error, try/catch is needed:

    printf('foo() ==> ');
    try { foo(); puts('FAIL'); } catch(e) { puts('PASS:',e); };

*test-mode* provides an alternative form of **try/catch** test, one wrapped in **";//     ;"**:

    cat > /tmp/trys.jsi <<EOF
    #!
    function bar(n) {}
    ;//  foo();
    ;//  bar();
    ;//  bar(1);
    EOF
    jsish --U /tmp/trys.jsi

==>

    foo() ==>
    PASS!: err = can not execute expression: 'foo' not a function
    bar() ==>
    PASS!: err = got 0 args, expected 1, calling function bar(n)
    bar(1) ==>
    FAIL!

A comment is used because it does nothing when not in *test-mode*.
Otherwise, it is wrapped in a **try/catch** for execution.

**Note**: *trys* are for testing exceptions, so the **FAIL** above is 
because *"bar(1)"* is not an error. 

Results
----

### Expects
To make *unit-test* more useful, test-scripts can provide *expect*-ed output in a special comment,
which can be automatically created (or updated) with:

    jsish -u -update true /tmp/test1.jsi
    cat /tmp/test1.jsi

==>

    puts('FOO');
    /*
    =!EXPECTSTART!=
    FOO
    =!EXPECTEND!=
    */

Updating the *semicolon* example:

    jsish -u -update true /tmp/test2.jsi
    cat /tmp/test2.jsi

==>

    var x;
    ;x=1;
    ;++x;
    
    /*
    =!EXPECTSTART!=
    x=1 ==> 1
    ++x ==> 2
    =!EXPECTEND!=
    */

### Fails
If test output fails the match, a *diff* is output:

    cat > /tmp/test4.jsi <<EOF
    function foo() { return true; }
    function bar() { return true; }
    
    ;bar();
    puts('foo() ==> ', foo());
    puts('DONE');
    
    /*
    =!EXPECTSTART!=
    bar() ==>  true
    foo() ==>  true
    DONE
    =!EXPECTEND!=
    */
    EOF
    
    jsish -u /tmp/test4.jsi

==>

    [FAIL] /tmp/test4.jsi
    at line 1 of output:
        output:	<bar() ==> true>
        expect:	<bar() ==>  true>
    ====================DIFFSTART
    -bar() ==>  true
    +bar() ==> true
     foo() ==>  true
     DONE
     
    ====================DIFFEND

### Analysis
A failed test with a lot of output may need some analysis:

    jsish -u -echo true /tmp/test4.jsi

==>

    Test /tmp/test4.jsi 
    bar() ==> /tmp/test4.jsi:4:   "true", 
    /tmp/test4.jsi:5:   "foo() ==>  true", 
    /tmp/test4.jsi:6:   "DONE", 

Or ...

    jsish --V /tmp/test4.jsi

==>

    bar() ==> "true", test4.jsi:4, 
    "foo() ==>  true", test4.jsi:5, 
    "DONE", test4.jsi:6, 


Run
----

### Tracing
If you wish to see the output using *Unit-test*:

    jsish -u -echo true /tmp/test2.jsi

==>

    Test /tmp/test2.jsi 
    x=1 ==> /tmp/test2.jsi:2:   "1", 
    ++x ==> /tmp/test2.jsi:3:   "2", 

Or use **--V**, which is a traced version of **--U**:

    jsish --V /tmp/test2.jsi

==>

    x=1 ==> "1", test2.jsi:2, 
    ++x ==> "2", test2.jsi:3, 


### Time
To get execution time for a script use:

    jsish -u -time true  /tmp/test2.jsi

==>

    [PASS] /tmp/test2.jsi 	 (13 ms)

To get execution-times + line-numbers for all *self-echo* lines, use:

    jsish -u -times true  /tmp/test2.jsi

==>

    Test /tmp/test2.jsi 
    #2: x=1 ==> 1
     (times = 0.000071 sec)
    #3: ++x ==> 2
     (times = 0.000085 sec)

### Directories
Entire directories of **.js** and **.jsi** files can be tested:

    jsish -u tests/
    [PASS] tests/49.js
    [PASS] tests/99bottles.js
    [PASS] tests/alias.js
    [PASS] tests/apply.js
    [FAIL] tests/arg.js
    at line 9 of output:
        output: <4>
        expect: <5>
    [FAIL] tests/arg2.js
    at line 9 of output:
        output: <1>
        expect: <2>
    [PASS] tests/alias.js
    Test tests/array.js
    ...
    [PASS] tests/yhsj.js
    2 FAIL, 97 PASS: runtime 21551 ms

The return code is the number of failed tests (up to a maximum of 127):

    echo $?
    2

*Note*: When multiple test are run, there is a summary, but no DIFF (unless context>3).

Config
----
The following configuration comments are available:

### Expect
An expected input comment:
    /*
    =!EXPECTSTART!=
    foo
    bar
    =!EXPECTEND!=
    */

As mentioned above expected output can be generated with **"jsish -u -update true"**.


### Inputs
A script needing *test-input* (as if read from stdin) can use:

    /*
    =!INPUTSTART!=
    45
    4950
    =!INPUTEND!=
    */


### Arguments
A script needing *input-arguments* can use:

    /*
    =!ARGSSTART!=
    -debug 1 able baker charlie
    =!ARGSEND!=
    */


Misc
----

### Assert
Note that asserts are normally disabled in Jsi, but not in *test-mode*:

    #!/usr/local/bin/jsish -u %s
    "use asserts"; // note: test mode already enables this.
    
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

As shown above, there are various ways to configure how *assert* behaves.

### Options
There is options help for *test-mode*:

    jsish -u -h
    Run script(s) as unit-tests setting return code to number of failed tests.
    
    Options/defaults:
        -args       ""      // Argument string to call script with
        -context    3       // Number of context lines for DIFF (>3 forces dir diff).
        -debug      false   // Enable debugging messages.
        -echo       false   // Run with puts/assert output showing file:line number.
        -evalFile   ""      // File to source in subinterp before test.
        -exec       false   // Use exec instead of running test in a sub-interp.
        -expectFile null    // File to use for expected output.
        -failMax    0       // Quit after this many failed tests.
        -inFile     null    // File to use for stdin input.
        -silent     false   // Run quietly, showing only failures.
        -time       false   // Show timing for all tests.
        -times      false   // Show timing for each tests.
        -update     false   // In place update or create of EXPECT section.
        -verbose    false   // Echo values for inputs/outputs/args.


### Embedding
Scripts can embedded tests that avoid execution outside of *test-mode*:

    if (Interp.conf('unitTest')) {
    ;    DoCall1();
    ;    DoCall2();
    }

### Preprocess
*Test-mode* uses inline *preprocessing* for testing, but you can define your own for whatever purpose:

    cat > input.jsi <<EOF
    :abcd:
    :1234:
    EOF

    cat > preprocess.jsi <<EOF
    function preprocess(str:string) { return format("puts('PP:', \"%s\");", str); }
    Interp.conf({jsppChars:'::', jsppCallback:preprocess, unitTest:3});
    source(console.args);
    EOF
    
    jsish preprocess.jsi input.jsi

==>

    PP: abcd
    PP: 1234

Keeping the reformat to a single line means that error and warning line numbers will line up.

Of course, you could achieve the same thing using **eval**, or generating a preprocessed file and **source**-ing it.

### Navigation

For Geany users, you can click-to-navigate output with F9 by
adding to top of script:

    #!/usr/local/bin/jsish -u -echo true %s

You can also try **-verbose** for even more detail.

### Builtins
To browse the builtin self-test files, look [here](https://jsish.org/fossil/jsi/dir?name=tests).
