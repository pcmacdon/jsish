/*
=!EXPECTSTART!=
Here is some random
Text.
=!EXPECTEND!=
*/

try {
    var f = new Channel('tests/filetest.txt');
    if (f) {
        while((n = f.gets())!=undefined) {
                puts(n);
        }
    }
} catch(e) {
   puts('Can not open tests/filetest.txt');
}
