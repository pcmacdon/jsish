/*
=!EXPECTSTART!=
a b c
a b c
a b c,.
,.a b c
[ "a", "b", "c" ]
a,b,c
1
=!EXPECTEND!=
*/


var A = " a b c ";
puts(A.trim());
var B = ",.a b c,.";
puts(B.trim(',.'));
puts(B.trimLeft(',.'));
puts(B.trimRight(',.'));

var a = 'a,b,c';
var b = a.split(',');
var c = b.join(',');
puts(b);
puts(c);

var d = '{ a: 1, b : [ 9, "dog", 11 ] }';
eval('var e = '+d+';');
//puts(d.a);
puts(e.a);


