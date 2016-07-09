/*
=!EXPECTSTART!=
{ a:"a" }
undefined
undefined
undefined
{ b:"b" }
1
2
undefined
{ c:"c" }
1
2
3
=!EXPECTEND!=
*/

this.top = 'top';

function a(a,b,c) {
    puts(this);
    puts(a);
    puts(b);
    puts(c);
};

a.apply({a:'a'});
a.apply({b:'b'}, [1,2]);
a.apply({c:'c'}, [1,2,3,4]);
