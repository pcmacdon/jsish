/*
=!EXPECTSTART!=
ZhangSan
ChunHua
18
20
18
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
    var zhang = new Person("ZhangSan", "man");
    puts(zhang.getName()); // "ZhangSan"
    var chun = new Person("ChunHua", "woman");
    puts(chun.getName()); // "ChunHua"
    
    puts(zhang.age);        //18
    zhang.age = 20;
    puts(zhang.age);        //20
    delete zhang.age;
    puts(zhang.age);
