/*
=!EXPECTSTART!=
true
=!EXPECTEND!=
*/
var adder = function (x) {
    return function (y) {
        return x + y;
    };
};
add5 = adder(5);
add5(1) == 6
