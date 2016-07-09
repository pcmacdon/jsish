/*
=!EXPECTSTART!=
try1
finally2
fock
finally1
{ b:{ c:0 }, x:0 }
=!EXPECTEND!=
*/

var a = {
    b: {
        c: 0
    },
    x: 0
};

for (var i = 0; i < 10; ++i) {
    try {
        puts("try1");

        with(a.b) {
            c = i;
            try {
                if (i == 5) throw("shit");
            } catch (e) {
                puts(e);
                with (a) {
                    x = 'shit';
                    throw("sadf");
                }
            } finally {
                puts("finally2");
                throw("fock");
            }
        }
    } catch(e) {
        puts(e);
        break;
    } finally {
        puts("finally1");
    }
}

puts(a);
