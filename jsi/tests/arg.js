/*
=!EXPECTSTART!=
3
[ "g", "2", "3" ]
argv[0] = g
argv[1] = 2
argv[2] = 3
=!EXPECTEND!=
*/
puts(console.args.length);
puts(console.args);

for (i = 0; i < console.args.length; ++i) {
    puts ("argv[" + i + "] = " + console.args[i]);
}



