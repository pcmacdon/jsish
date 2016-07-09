/*
=!EXPECTSTART!=
undefined
================
a:1
b:2
c:3
d:4
================
a:1
b:2
c:3
d:4
shite:fock
================
a:1
b:2
c:3
d:4
fock:shite
=!EXPECTEND!=
*/

a = {a: 1,b:2,c:3,d:4};
puts(a.e);

puts("================");
for(var s in a) { puts(s + ":" + a[s]); }

a.shite = "fock";

puts("================");
for(var s in a) { a.fock = "shite"; puts(s + ":" + a[s]); }

puts("================");
for(var s in a) { 
    puts(s + ":" + a[s]);
    delete a.shite; 
}
