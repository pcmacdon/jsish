/*
=!EXPECTSTART!=
'xx', functions are: bad big ugly.
[ "bad", "big", "ugly" ]
[ "a", "b" ]
[ "x" ]
=!EXPECTEND!=
*/
var x = {a:1, b:2};
x.big = function() {};
x.bad = function() {};
x.ugly = function() {};
try {
  x.xx();
} catch(e) {
  puts(e);
}
puts(Info.funcs(x));
puts(Info.data(x));
puts(Info.data());

