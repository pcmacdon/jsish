/*
=!EXPECTSTART!=
A
fin
fock
=!EXPECTEND!=
*/

var a = {a:1, b:2};

try {
for (var n in a) {
    try {
        switch(n) {
            case "a":
                puts("A");
                continue;
            case "b":
                puts("B");
                throw("ex");
        }
    } catch(e) {
        puts(e);
        throw(e);
    } finally {
        puts("fin");
        throw("fock");
    }
}

} catch (e) {
    puts(e);
}
