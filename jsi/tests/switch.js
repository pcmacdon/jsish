/*
=!EXPECTSTART!=
default
1
=!EXPECTEND!=
*/

switch(3) {
    default:   puts("default");
    case 1:
        puts("1");
        break;
    case 2:
        puts("2");
    continue;
}
