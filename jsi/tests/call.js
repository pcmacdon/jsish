/*
=!EXPECTSTART!=
{ a:1 }
{ a:1 }
1
2
3
3
{ fock:32 }
4
5
6
9
1
3
{ a:1 }
4
6
{ a:1 }
1
3
{ fn:4 }
4
6
{ fm:8 }
=!EXPECTEND!=
*/

this.a = 1;
puts(this);

function f(x, y, z) {
    var a = arguments[0] + arguments[1];
    puts(this);
    puts(x);
    puts(y);
    puts(z);
    puts(a);
    var ff = function (a) {
        puts(x);
        puts(z);
        puts(this);
    };
    return ff;
};

var fn = f(1, 2,3);

fm = f.call({fock:32}, 4,5,6);

fn(456);
fm(789);

fn.call({fn:4}, 456);
fm.call({fm:8}, 55667);

