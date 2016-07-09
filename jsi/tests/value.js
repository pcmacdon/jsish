/*
=!EXPECTSTART!=
true
aa
{ a:1, b:3 }
function Boolean() { [native code] }
function Boolean() { [native code] }
true
=!EXPECTEND!=
*/

var b = new Boolean(true);
puts(b.valueOf());

var s = new String('aa');
puts(s.valueOf());

var x = { a:1, b:3 };
puts(x.valueOf());

puts(b.constructor);
puts(Boolean);
puts(Boolean == b.constructor);

