#!/usr/local/bin/jsish -u %s

var a, b, c, d, e, f, g, h, i;
for (a = 1; a < 10; ++a) {
    for (b = 1; b < 10; ++b) {
        if (b == a) continue;
        for (c = 1; c < 10; ++c) {
            if (c == a) continue;
            if (c == b) continue;
            for (d = 1; d < 10; ++d) {
                if (d == a) continue;
                if (d == b) continue;
                if (d == c) continue;
                for (e = 1; e < 10; ++e) {
                    if (e == a) continue;
                    if (e == b) continue;
                    if (e == c) continue;
                    if (e == d) continue;
                    for (f = 1; f < 10; ++f) {
                        if (f == a) continue;
                        if (f == b) continue;
                        if (f == c) continue;
                        if (f == d) continue;
                        if (f == e) continue;
                        for (g = 1; g < 10; ++g) {
                            if (g == a) continue;
                            if (g == b) continue;
                            if (g == c) continue;
                            if (g == d) continue;
                            if (g == e) continue;
                            if (g == f) continue;
                            for (h = 1; h < 10; ++h) {
                                if (h == a) continue;
                                if (h == b) continue;
                                if (h == c) continue;
                                if (h == d) continue;
                                if (h == e) continue;
                                if (h == f) continue;
                                if (h == g) continue;
                                for (i = 1; i < 10; ++i) {
                                    if (i == a) continue;
                                    if (i == b) continue;
                                    if (i == c) continue;
                                    if (i == d) continue;
                                    if (i == e) continue;
                                    if (i == f) continue;
                                    if (i == g) continue;
                                    if (i == h) continue;
                                    
                                    if (a + b + c != 15) continue;
                                    if (d + e + f != 15) continue;
                                    if (g + h + i != 15) continue;
                                    if (a + d + g != 15) continue;
                                    if (b + e + h != 15) continue;
                                    if (c + f + i != 15) continue;
                                    if (a + e + i != 15) continue;
                                    if (c + e + g != 15) continue;
                                    
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

