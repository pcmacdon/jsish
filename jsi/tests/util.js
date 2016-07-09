/*
=!EXPECTSTART!=
100000
=!EXPECTEND!=
*/
var x = 0;
function foo(n) { return n+1; }
function bar() { x += foo(0); }
var tim = times(bar,100000);
puts(x);

