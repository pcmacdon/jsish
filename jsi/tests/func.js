/*
=!EXPECTSTART!=
4950
=!EXPECTEND!=
*/

function a(n) {
    sum = 0;
    for ( i = 0; i < n; i++) {
        sum = sum + i;
    }
    puts(sum);
    return sum;
};

a(100);

