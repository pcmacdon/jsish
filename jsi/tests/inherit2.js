/*
=!EXPECTSTART!=
LN:=Smith
LN2:=Doe
LN3:=Smith
LN4:=Smith
=!EXPECTEND!=
*/

var john = {firstName: 'John', lastName: 'Smith'};
var jane = {firstName: 'Jane'};
Object.setPrototypeOf(jane, john);
puts("LN:="+jane.lastName);
jane.lastName = 'Doe';
puts("LN2:="+jane.lastName);
puts("LN3:="+john.lastName);
delete jane.lastName;
puts("LN4:="+jane.lastName);
