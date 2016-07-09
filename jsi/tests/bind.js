/*
=!EXPECTSTART!=
81
9
81
=!EXPECTEND!=
*/
var x = 9; 
var module = {
  x: 81,
    getX: function() { return this.x; }
};

puts(module.getX()); // 81

var getX = module.getX;
puts(getX()); // 9, because in this case, "this" refers to the global object

// create a new function with 'this' bound to module
var boundGetX = getX.bind(module);
puts(boundGetX()); // 81
