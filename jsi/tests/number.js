/*
=!EXPECTSTART!=
9.1
9.12e+0
9.1
9.1
9.12e+0
9.1
12
=!EXPECTEND!=
*/
puts(Number.toPrecision(9.1234,2));
puts(Number.toExponential(9.1234,2));
puts(Number.toFixed(9.1234,1));
var j = new Number(9.1234);
puts(j.toPrecision(2));
puts(j.toExponential(2));
puts(j.toFixed(1));

var num = 15;
puts(String.replace(num, /5/, '2'));

