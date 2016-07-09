/*
=!EXPECTSTART!=
abc@gmail.com
=!EXPECTEND!=
*/


if (console.args.length < 1) {
    puts("Usage: jsi grep.ss <PATTERN>");
    exit(-1);
}

var reg = new RegExp(console.args[0]);
line = "";
while (1) {
    line = console.input();
    if (line == undefined) break;
    if (line.match(reg)) {
        puts(line);
    }
}

