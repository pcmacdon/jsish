/*
=!EXPECTSTART!=
[ "foo", "bar" ]
function myAlias(interp,name,arg1,arg2) {...}
[ #Interp_1, "foo" ]
myAlias2: 1 2
A
B
myAlias: #Interp_1 bb.fig 1 undefined
[ "bb.fig", "foo", "bar" ]
[ "bb.fig", "bar" ]
OK
=!EXPECTEND!=
*/
var i = new Interp({subthread:false});
//var i = new Interp();
function myAlias(interp,name,arg1,arg2) {
   puts('myAlias: '+interp+' '+name+' '+arg1+' '+arg2);
}

function myAlias2(arg1,arg2) {
   puts('myAlias2: '+arg1+' '+arg2);
}

i.alias('foo', myAlias, [i, 'foo']);
i.alias('bar', myAlias2,null);

puts(i.alias());
puts(i.alias('foo'));
puts(i.alias('foo', myAlias));

i.eval('bar(1,2);');

puts('A');
i.eval('var bb = {x:1};');
puts('B');
i.alias('bb.fig', myAlias, [i, 'bb.fig']);
i.eval('bb.fig(1)');

/*i.alias('bb.fig', myAlias, [i, 'bb.FIG']);
i.eval('bb.fig(1)');*/

puts(i.alias());
//i.alias('bb.fig', null, null); //Leaks memory.
i.alias('foo', null, null);
puts(i.alias());

//try { i.eval('bb.fig(1)'); } catch(e) { puts("CAUGHT ERROR: "+e); };
puts("OK");
