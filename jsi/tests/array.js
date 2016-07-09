/*
=!EXPECTSTART!=
A
5
SHIFTED: 5
4
0 = pig
1 = cat
2 = 99
3 = 4
4 = 3
5 = 2
6 = 1
10 = -1
11 = dog
A = pig,cat,99,4,3,2,1,-1,dog
IDX = 11
S = cat,99,4,3
SLICE0 = cat
SLICE* = cat,99,4,3
T: pig,cat,99,4,3,2,1,-1,dog,SICK!!!,cat,99,4,3
Z: 5,4,A,B,1 ==> 3,2
Q: Aaa,aAb,AAc,Bba,bBb
=!EXPECTEND!=
*/
var y = [ '2', '1' ];
var x = { '0': 'A', '1':'B' };
puts(x[0]);
var a = new Array(5,4,3,2,1);
a.dog = 9;
puts(a[0]);
var b = a.shift();
puts("SHIFTED: "+b);
puts(a[0]);
a[7] = -1;
a[8] = 'dog';
a.unshift('pig', 'cat', 99);
for (var i in a) { puts(i+" = "+a[i]); }
puts('A = '+a.join(','));
puts("IDX = "+a.indexOf('dog'));
var s = a.slice(1,4);
puts('S = '+ s.join(','));
puts("SLICE0 = "+s[0]);
puts("SLICE* = "+s.join(','));

var t = a.concat('SICK!!!',s);
puts('T: '+t.join(','));


var z = new Array(5,4,3,2,1);
var za = z.splice(2,2,'A','B');
puts('Z: '+z.join(',') + " ==> "+za.join(','));


//var q = new Array(5,4,2,3,1);
var q = new Array('Aaa','aAb','AAc','Bba','bBb');
var qq = q.sort();

puts('Q: '+qq.join(','));


