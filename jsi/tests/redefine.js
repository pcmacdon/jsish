/*
=!EXPECTSTART!=
ORIG FOO: [ 99 ]
LOAD
DONE
FOO: [ 99 ]
=!EXPECTEND!=
*/

function Load() {
  puts("LOAD");
  //delete Foo;
  Foo = function (a) { puts("FOO: "+arguments.toString()); };
  puts("DONE");
}

function Foo() {
  puts("ORIG FOO: "+arguments.toString());
  Load();
  return Foo.apply(this,arguments);
}

var j = new Foo(99);

