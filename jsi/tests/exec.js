/*
=!EXPECTSTART!=
baker
A=/tmp
B=/tmp

C=
D=127
E={ code:127, data:"sh: 1: gobbledeygook: not found", status:0 }
F=1
G=0
H=0
I=0
J=0
=!EXPECTEND!=
*/

a = exec('ls -d /tmp');
puts("A="+a);
b = exec('ls -d /tmp',{noTrim:true});
puts("B="+b);
c = exec('gobbledeygook', null);
puts("C="+c);
d = exec('gobbledeygook', {retCode:true});
puts("D="+d);
e = exec('gobbledeygook 2>&1', {retAll:true});
puts("E="+e.toString());
f = exec('grep -q bakker', {input:"able\nbaker\ncharlie\n"});
puts("F="+f);
g = exec('grep baker', "able\nbaker\ncharlie\n");
puts("G="+g);
h = exec('ls /tmp', {retCode:true});
puts("H="+h);
i = exec('sleep 1&');
puts("I="+i);
j = exec('sleep 1', {bg:true});
puts("J="+j);
