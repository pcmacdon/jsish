/*
=!EXPECTSTART!=
in b
=!EXPECTEND!=
*/
function a() {
    puts('in a');
};

function b() {
    puts('in b');
};

var c = a;

c((c = b), 2);
