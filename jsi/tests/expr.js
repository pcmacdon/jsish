/*
=!EXPECTSTART!=
undefined
-1
0
NaN
-Infinity
-1
-123
0
3
3.6
1.212.3
5
2342true
12
NaN
NaNNaN
NaN
NaN
================
16
16777216
0
-1
===============
true
undefined
false
true
false
false
true
true
false
false
true
true
false
true
true
true
===============
true
false
false
true
false
true
true
===================
false
false
false
true
true
============
31
106
1190
===============
16
12
3
12
0
0
0
1
1fock
{ a:124 }
{ a:120 }
{ a:30 }
{ a:120 }
{ a:0 }
{ a:0 }
{ a:0 }
{ a:1 }
{ a:"1fock" }
=!EXPECTEND!=
*/

//test void expr
puts(void 1);

//test - expr
puts(-1);
puts(-0);
puts(-NaN);
puts(-Infinity);
puts(- true);
puts(- "123" );

//wrong: NaN, not 0
puts(- {a:1});

//test + expr
puts(1 + 2);
puts(1.3 + 2.3);
puts(1.2 + "12.3");
puts(4 + true);
puts("2342" + true);
puts({} + 12);
puts(NaN + NaN);
puts(NaN + "NaN");
puts(Infinity - Infinity);
puts(NaN + 3);

puts("================");
puts(1<<4);
puts(1<<344.3);
puts(2>>4);
puts(-200000 >> -4);

puts("===============");
puts(1.0 < 2.3);
puts(NaN < NaN);
puts(Infinity < -Infinity);
puts(Infinity > -Infinity);
puts(10000.456 < 10000.456);
puts(10000.456 > 10000.456);
puts(10000.456 <= 10000.456);
puts(10000.456 >= 10000.456);
puts("10000.456" < "10000.456");
puts("10000.456" > "10000.456");
puts("10000.456" <= "10000.456");
puts("10000.456" >= "10000.456");
puts("a" > "b");
puts("a" >= "a");
puts("a" < "aa");
puts("a" < "b");
puts("===============");
puts(1 == 1);
puts(2 == 1);
puts(NaN == NaN);
puts("2" == 2);
puts(true == null);
puts("234234" == "234234");
puts(true == 1);

puts("===================");
puts(1 === true);
puts(1 === "1");
puts(NaN === NaN);
puts("abc" === "abc");
puts(3.1415926 === 3.1415926);

puts("============");
puts(1 | 2 | 4 | 8 | 16);
puts(123 & 234);
puts(3456 ^ 2342);

puts("===============");
var a = 12;
a += 4;
puts(a);
a -= 4;
puts(a);
a /= 4;
puts(a);
a *= 4;
puts(a);
a %= 4;
puts(a);
a <<= 4;
puts(a);
a >>= 3;
puts(a);
a += true;
puts(a);
a += "fock";
puts(a);

a = { a: 120 };
a.a += 4;
puts(a);
a.a -= 4;
puts(a);
a.a /= 4;
puts(a);
a.a *= 4;
puts(a);
a.a %= 4;
puts(a);
a.a <<= 4;
puts(a);
a.a >>= 3;
puts(a);
a.a += true;
puts(a);
a.a += "fock";
puts(a);
