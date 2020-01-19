#!/usr/local/bin/jsish -u %s

function a(a,b,c) {
    for (var x = 0; x < arguments.length; ++x) {
        puts(x + ":" + arguments[x]);
    }
    arguments.callee = 1;
    for (var x = 0; x < arguments.length; ++x) {
        puts(x + ":" + arguments[x]);
    }
    puts(arguments.callee);
};

a(4,5,6,8);

/*
=!EXPECTSTART!=
0:4
1:5
2:6
3:8
0:4
1:5
2:6
3:8
1
=!EXPECTEND!=
//diff from firefox, arguments.callee can not be changed
*/
