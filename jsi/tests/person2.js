/*
=!EXPECTSTART!=
ZhangSan
=!EXPECTEND!=
*/

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
    
    Employee.prototype = new Person();
    Employee.prototype.getEmployeeID = function() {
        return this.employeeID;
    };
    var zhang = new Employee("ZhangSan", "man", "1234");
    puts(zhang.getName()); // "ZhangSan
