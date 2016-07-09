/*
=!EXPECTSTART!=
7
write access denied
subinterps disallowed
=!EXPECTEND!=
*/
var ii = new Interp();
puts (ii.eval('3+4'));
ii.eval('exit(0)');
delete ii;


var ii = new Interp({isSafe:true, safeWriteDirs:['/tmp'], safeReadDirs:['/tmp']});
ii.eval("File.write('/tmp/xx.txt','hi');");
try {
  ii.eval("File.write('~/xx.txt','bye');");
} catch (e) {
  puts(e);
}
delete ii;


var ii = new Interp({noSubInterps:true});
try {
  ii.eval("new Interp()");
} catch (e) {
    puts(e);
}
delete ii;



