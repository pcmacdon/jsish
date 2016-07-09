/*
=!EXPECTSTART!=
{ b:{ c:2 } }
undefined
=!EXPECTEND!=

diff from ecma, var should make var in with
*/

var  a = {
    b: {
        c: 1
    }
};

with (a.b) {
    c = 2;
        eval("var d = 4;");
}
puts(a);
puts(a.b.d);

