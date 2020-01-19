#!/usr/local/bin/jsish -u %s

var a, b, c, d, e, f, g, h, i;
for (a = 1; a < 10; ++a) {
    for (b = 1; b < 10; ++b) {
        if (b == a) continue;
        for (c = 1; c < 10; ++c) {
            if (c == a || c == b) continue;
            for (d = 1; d < 10; ++d) {
                if (d == a || d == b || d == c) continue;
                for (e = 1; e < 10; ++e) {
                    if (e == a || e == b || e == c || e == d) continue;
                    for (f = 1; f < 10; ++f) {
                        if (f == a || f == b || f == c || f == d || f == e) continue;
                        for (g = 1; g < 10; ++g) {
                            if (g == a || g == b || g == c || g == d || g == e || g == f) continue;
                            for (h = 1; h < 10; ++h) {
                                if (h == a || h == b || h == c || h == d || h == e || h == f || h == g) continue;
                                for (i = 1; i < 10; ++i) {
                                    if (i == a || i == b || i == c || i == d || i == e || i == f || i == g || i == h) continue;
                                    
                                    if (a + b + c == 15 && 
                                        d + e + f == 15 &&
                                        g + h + i == 15 &&
                                        a + d + g == 15 &&
                                        b + e + h == 15 &&
                                        c + f + i == 15 &&
                                        a + e + i == 15 &&
                                        c + e + g == 15) {
                                        puts(a + " " + b + " " + c);
                                        puts(d + " " + e + " " + f);
                                        puts(g + " " + h + " " + i);
                                        puts("--------");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
/*
=!EXPECTSTART!=
2 7 6
9 5 1
4 3 8
--------
2 9 4
7 5 3
6 1 8
--------
4 3 8
9 5 1
2 7 6
--------
4 9 2
3 5 7
8 1 6
--------
6 1 8
7 5 3
2 9 4
--------
6 7 2
1 5 9
8 3 4
--------
8 1 6
3 5 7
4 9 2
--------
8 3 4
1 5 9
6 7 2
--------
=!EXPECTEND!=
*/
