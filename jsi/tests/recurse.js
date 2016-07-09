/*
=!EXPECTSTART!=
3628800
=!EXPECTEND!=
*/

function jc(n)
{
    if (n <= 1) return 1;
    return jc(n - 1) * n;
};

puts(jc(10));
