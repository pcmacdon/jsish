/*
=!EXPECTSTART!=
the original string
he original string
g

tri

a
al
4
-1
6
16
A
d
2
6
=!EXPECTEND!=
*/

var a = "the original string";

puts(a.substr());
puts(a.substr(1));
puts(a.substr(-1, 1));

puts(a.substr(2, -1));
puts(a.substr(-5, 3));
puts(a.substr(100, 0));
puts(a.substr(10, 1));
puts(a.substring(10, 11));

puts(a.indexOf("ori"));
puts(a.indexOf("swer"));
puts(a.indexOf("i"));
puts(a.indexOf("i", 9));
puts(String.fromCharCode(65));
var s = 'abcdabcd';
puts(s.charAt(3));
puts(s.indexOf('cd'));
puts(s.lastIndexOf('cd'));

