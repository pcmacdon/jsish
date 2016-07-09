/*
=!EXPECTSTART!=
Howdy, my name isBob
Bob
object
false
true
object
number
=!EXPECTEND!=
*/

function Person(name, gender){

    this.name = name;
    this.gender = gender;
}

Person.prototype = { spake: function() { return puts(this.name); } };
Person.prototype.speak = function(){ puts("Howdy, my name is" + this.name); };


var person = new Person("Bob", "M");

person.speak();
person.spake();
puts(typeof person);
puts(person.hasOwnProperty('toString'));
puts(person.hasOwnProperty('name'));

var y = [1,2,3];
var x = typeof(y);
puts(x);
puts(typeof((9)));

