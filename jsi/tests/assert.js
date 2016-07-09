/*
=!EXPECTSTART!=
caught error
K
caught error2
=!EXPECTEND!=
*/
assert(true,'true');
assert(2*3 == 6,'math');
try {
 assert(false,'false');
} catch(e) {
puts('caught error');
}
Interp.conf({nDebug:true});
assert(false,'false');
Interp.conf({nDebug:false});

var i=1, j=2;
assert(function () { return (i<j); },'fail');
puts("K"); 
try {
assert(false,'false');
} catch(e) {
puts('caught error2');
}

