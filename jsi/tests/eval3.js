/*
=!EXPECTSTART!=
{ a:1 }
{ a:1, b:2, c:"abc", d:[ 1, 2, 3 ], e:{  } }
abc
=!EXPECTEND!=
*/

var a = {};
eval("a.a = 1;");
puts(a);


var y = eval("{ a:1, b:2, c:'abc', d:[1,2,3], e: {}}");
puts(y);


var n = 'abc';
eval("(y = n)");

puts(y);

