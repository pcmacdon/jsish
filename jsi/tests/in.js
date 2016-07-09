/*
=!EXPECTSTART!=
K = true
true
false
true
false
a:1
c:3
d:4
=!EXPECTEND!=
*/

var w = {a:1,b:2,c:3};
var x = [1,2,3];
var z = 2;
var k = (z in x);
puts('K = '+k);
puts(2 in x);
puts(4 in x);
puts('a' in w);
puts('d' in w);



a = {a: 1,b:2,c:3,d:4};
for(var s in a) { 
	puts(s + ":" + a[s]);
	delete a.b; 
}
