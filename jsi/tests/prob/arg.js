// Memory leak to to argument alias.
function foo() {
  var args = arguments;
  for (i=0; i<args.length; i++)
    puts(args[i]);
}
foo(1,2,3,4);
