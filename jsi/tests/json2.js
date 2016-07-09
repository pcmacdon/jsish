/*
=!EXPECTSTART!=
true
true
true
false
{ data:[ 6, 2961 ], label:"editTran", type:"" }
{ data:[ 6, 2961 ], label:"editTran", type:"" }
{ data:[ 6, 2961 ], label:"editTran", type:"" }
parse error (unexpected char in strict mode) at offset 32 "data : [6,2961]}"
{ "able":1 }
[ "A B", 1 ]
{ A:1, Columns:[ 1, 2, { Au0020B:1, B:[ 2, 3 ] }, 2 ] }
=!EXPECTEND!=
*/

var x = '{"type":"", "label":"editTran", "data" : [6,2961]}';
var x2 = '{"type":"", "label":"editTran", data : [6,2961]}';
puts(JSON.check(x));
puts(JSON.check(x,true));
puts(JSON.check(x2));
puts(JSON.check(x2,true));
puts(JSON.parseNS(x));
puts(JSON.parse(x));

puts(JSON.parseNS(x2));
try { puts(JSON.parse(x2)); } catch(e) { puts(e); }
var dat = { able:1, baker:undefined };
puts(JSON.stringify(dat));

var x = JSON.parse('["A\\u0020B",1]');
puts(x.toString());

JSON.parse('{ "Columns": [ 1, 2], "A" : 1 }');
var x = JSON.parse('{ "Columns": [ 1, 2, {"A\u0020B":1, "B":[2,3]}, 2], "A" : 1 }');
puts(x.toString());
