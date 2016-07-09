/*
=!EXPECTSTART!=
{ type:"number" }
[ "xx", "x" ]
[ "xx", "x", "y" ]
[ "a" ]
[ "XX", "X" ]
[ "XX", "X", "Y" ]
F
[ "z" ]
[ "f", "g" ]
info.js
info.js
info.js
[ "load" ]
{ args:"str:string, ...", help:"Append one or more strings", maxArgs:-1, minArgs:0, name:"String.concat", retType:"string", type:"command" }
[ "P", "Q" ]
=!EXPECTEND!=
*/
var x = 1;
var xx = 2;
var y = 2;

puts(Info.vars('x'));
puts(Info.vars(/x/));
puts(Info.vars());

function X(a) {var jj=1, kk; return jj++;}
function XX(a) {}
function Y(a) {}

puts(Info.funcs('X').args);
puts(Info.funcs(/X/));
puts(Info.funcs());

var K = {};
K.f = function(z) { puts("F"); };
K.g = function(z) { puts("G"); };
K.f();
puts(Info.funcs(K.f).args);
puts(Info.funcs(K));

puts(File.tail(Info.script()));
puts(File.tail(Info.script(XX)));
puts(File.tail(Info.script(/.*/)[0]));

puts(Info.cmds(/^loa./));
puts(Info.cmds('String.concat'));

X.prototype.P = function(M) { return M; };
X.prototype.Q = function(M) { return M; };
puts(Info.funcs(X.prototype));

