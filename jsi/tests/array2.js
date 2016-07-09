/*
=!EXPECTSTART!=
[ undefined ]
[ undefined, 2 ]
5
3
2
1
2
2
undefined
undefined
undefined
0
=!EXPECTEND!=
*/

var a = new Array(1);

puts(a);
a[1] = 2;
puts(a);

puts(a.push(1,2,3));
puts(a.pop());
puts(a.pop());
puts(a.pop());
puts(a.length);
puts(a.pop());
puts(a.pop());
puts(a.pop());
puts(a.pop());
puts(a.length);

