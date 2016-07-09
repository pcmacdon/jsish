/*
=!EXPECTSTART!=
{ a:[ { b:1, c:[ 2, 3, 5, 6 ], d:{ x:1, y:2 } }, 2, 3, 4 ], x:"yz" }
{ abc:1, cde:"123123", x:{ a:1, b:2 }, y:{ a:3, b:4 } }
1
2
[ "f", "x", "y" ]
[ "f", "x", "y" ]
=!EXPECTEND!=
*/

var x = {a:[{b:1,c:[2,3,5,6],d:{x:1,y:2}},2,3,4],x:"yz"};
puts (x);

var a = { abc:1,cde:"123123",x:{a:1,b:2},y:{a:3,b:4}};
puts(a);

var O = {
  x: 1, y: 2,
  f : function(n) { return n+1; }
};

var o = Object.create(O);

puts(o.x);
puts(o.f(1));
puts(o.keys());
puts(o.keys());

