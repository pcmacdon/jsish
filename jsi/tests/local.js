/*
=!EXPECTSTART!=
11
30
130
1
2
7
12
112
130
=!EXPECTEND!=
*/

var a = 1;
var b = 2;

function abc(a,b) {
    var c = a+b;
    var d = a * b;
    puts(c);
    puts(d);
    return d;
};

puts(x = abc(5,6) + 100);

puts(a);
puts(b);
puts(abc(3,4)+100);

puts(x);
