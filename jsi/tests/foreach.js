/*
=!EXPECTSTART!=
a
b
c
c
b
a
[ "c", "a" ]
[ "c", "a" ]
=!EXPECTEND!=
*/
var a = ["a", "b", "c"];
a.forEach(function(entry) {
    puts(entry);
});
a = a.reverse();
for (var i in a) {
    puts(a[i]);
}
var b = a.filter(function(x) { return (x != 'b'); });
puts(b);
var c = [];
for (var i in a) {
  x = a[i];
  if (x != 'b') c.push(x);
}
puts(c);
