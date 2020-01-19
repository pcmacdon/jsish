#!/usr/local/bin/jsish -u %s

var i = 1;
ABC:
{
    i++;
    break ABC;
    puts("FOO");
}
puts("DONE");

/*
=!EXPECTSTART!=
DONE
FOO
=!EXPECTEND!=
*/

