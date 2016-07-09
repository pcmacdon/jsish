/*
=!EXPECTSTART!=
0:1
1:2
2:3
4:5
5:6
a:
1
b:
2
c:
3
d:
[ 4, 3, 2, 1 ]
e:
{ x:"x", y:"y", z:"z" }
=!EXPECTEND!=
*/

var x = [1,2,3,4,5, 6,7,8];
for (var i in x) { 
    if (i == 3) continue;
    puts(i + ":" +x[i]); 
    if (i > 4) break;
}

var obj = { a: 1, b:2, c:3, d:[4,3,2,1], e:{x:"x", y:"y", z:"z"}};
for (i in obj) {
    puts(i + ":");
    puts( obj[i]);
}
