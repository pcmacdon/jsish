/*
=!EXPECTSTART!=
140
623
=!EXPECTEND!=
*/

function abc(a, b, c) {
    var x = 123;
    function fx(a, b) {
        return (function (f) {
            return x + a + b + c + f;
        });
    };
    return fx(a, b);
};

var fn = abc(3, 5, 6);

var fsh = abc(100, 100, 100);

var x = fn(3);

var y = fsh(200);

puts(x);

puts(y);
