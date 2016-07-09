/*
=!EXPECTSTART!=
{  }
     should be {}
{ a:123 }
     should be { a:123 }
123
     should be 123
shot
     should be shot
{ a:123 }
     should be { a:123 }
6
     should be 6
ZhangSan
18
defaultName
true
false
=!EXPECTEND!=
*/

puts(Object.prototype);
puts("     should be {}");

Object.prototype.a = 123;
//Object.prototype = 123;

puts(Object.prototype);
puts("     should be { a:123 }");

var a = { b:1, c:2 };
puts(a.a);
puts("     should be 123");
a.a = 'shot';
puts(a.a);
puts("     should be shot");

puts(Object.prototype);
puts("     should be { a:123 }");

Number.prototype.fock = function() {
    puts(this / 2);
};

var x = 12;

x.fock();
puts("     should be 6");

function Person(name, sex) {
   this.name = name;
   this.sex = sex;
};

Person.prototype = {
   getName: function() {
       return this.name;
   },
   getSex: function() {
       return this.sex;
   },
   age: 18
};

function Employee(name, sex, employeeID) {
    this.name = name;
    this.sex = sex;
    this.employeeID = employeeID;
};

Employee.prototype = new Person("defaultName", "defaultSex");
Employee.prototype.getEmployeeID = function() {
    return this.employeeID;
};

var zhang = new Employee("ZhangSan", "man", "1234");
puts(zhang.getName()); // "ZhangSan
puts(zhang.age);            //18
delete zhang.name;
puts(zhang.name);       //defaultName

function f() {}
function g() {}
g.prototype = new f();
var h = new g();
puts(g.prototype.isPrototypeOf(h));
puts(g.prototype.isPrototypeOf(f));
