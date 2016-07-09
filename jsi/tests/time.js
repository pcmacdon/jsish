/*
=!EXPECTSTART!=
0
0
1970-01-01
=!EXPECTEND!=
*/
var s = strftime(0);
puts(strptime(s));
var s = strftime(0,{utc:true});
puts(strptime(s,{utc:true}));
var t = strptime(s,{utc:true,fmt:"%c"});
puts(strftime(t,{utc:true,fmt:"%Y-%m-%d"}));

