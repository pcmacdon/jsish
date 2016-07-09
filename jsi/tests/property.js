/*
=!EXPECTSTART!=
true
true
false
9
undefined
undefined
false
true
=!EXPECTEND!=
*/

var x = {x:1};
puts(x.propertyIsEnumerable('x'));
var x = {a:1, b:2};
puts(x.hasOwnProperty('a'));
puts(x.hasOwnProperty('c'));

function f() {}
f.a = 9;
function g() {}
g.prototype.x = new f();
var h = new g();
puts(f.a);
puts(g.a);
puts(h.a);

var s = new String("Sample");
puts(s.hasOwnProperty("split"));
puts(String.prototype.hasOwnProperty("split"));

