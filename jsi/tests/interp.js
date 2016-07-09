/*
=!EXPECTSTART!=
foo
foo
foo
0 1
2 3
2 4
=!EXPECTEND!=
*/

function foo(n) {
  puts('foo');
};

foo(1);
eval('foo(1);');
var x1 = Info.interp();
eval('foo(1);');
var x2 = Info.interp();
puts(x1.codeCacheHits+' '+x2.codeCacheHits);
puts(x1.funcCallCnt+' '+x2.funcCallCnt);
puts(x1.cmdCallCnt+' '+x2.cmdCallCnt);
