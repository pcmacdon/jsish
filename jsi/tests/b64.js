/*
=!EXPECTSTART!=
1234 ---> MTIzNA==
=!EXPECTEND!=
*/
var j = b64encode('1234');
puts(b64decode(j) + ' ---> ' + j);
