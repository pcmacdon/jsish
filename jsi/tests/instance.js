/*
=!EXPECTSTART!=
true
false
true
=!EXPECTEND!=
*/

function f() {};
function j() {};
var x = new f();
puts(x instanceof f);
puts(x instanceof j);
puts(x instanceof Object);
