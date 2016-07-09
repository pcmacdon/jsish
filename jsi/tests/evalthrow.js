/*
=!EXPECTSTART!=
abc
finally
=!EXPECTEND!=
*/

try {
    eval("throw('abc');");
} catch(e) {
    puts(e);
} finally {
    puts("finally");
}

