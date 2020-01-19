#!/usr/local/bin/jsish -u %s

var  a = {
    b: {
        c: 1
    }
};

with (a.b) {
    c = 2;
        eval("var d = 4;");
}
;a;
;a.b.d;
;d;

/*
diff from ecma, var should make var in with

=!EXPECTSTART!=
a ==> { b:{ c:2 } }
a.b.d ==> undefined
d ==> 4
=!EXPECTEND!=

*/

