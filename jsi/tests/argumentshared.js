/*
=!EXPECTSTART!=
100
=!EXPECTEND!=
*/

function a(x) {
    arguments[0] = 100; 
    puts(x);
};

a(1);
