/*
=!EXPECTSTART!=
{ x:[ 4, 5, 6 ], y:{ a:1, b:2 } }
{ a:1, b:2 }
1
=!EXPECTEND!=
*/

var a = { "x":[4,5,6], "y":{a:1, b:2}};
var b = { n:a, m:a.y };
puts (b.n);
puts (b.m);

function a() { return {x:1, y:{a:1,b:[]}}; };
puts(a().y.a);

