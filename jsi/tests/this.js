/*
=!EXPECTSTART!=
a: 1
this.a: a in x
=!EXPECTEND!=
*/

function fock(a,b){
    puts("a: " + a);
    puts("this.a: " + this.a);
};

x = { test: fock, a: 'a in x', b: 'b in x' };

{ test: fock, a: 'a in x', b: 'b in x' }.test(1, 2);
