/*
=!EXPECTSTART!=
b
c
d
a
=!EXPECTEND!=
*/


var showthis = function() {
    puts(this.name);
};

var a = { name: 'a', f: showthis };
var b = { name: 'b', f: showthis };
var c = { name: 'c', f: showthis };
var d = { name: 'd', f: showthis };

a.f(b.f(), c.f(), d.f());
