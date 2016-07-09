/*
=!EXPECTSTART!=
true
true
=!EXPECTEND!=
*/
var b = new Boolean(false);
puts(b.constructor == Boolean );
puts(Boolean == b.constructor );

function f() {};
function g() {};
g.prototype = new f();
var h = new g();
