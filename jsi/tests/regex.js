/*
=!EXPECTSTART!=
input the email
invalid email format
input the email
invalid email format
input the email
match at: abc@gmail.com
name: abc
domain: gmail
input the email
invalid email format
input the email
a c b c d b
a c c c d c
a c b c d b
[ "ain", "AIN", "ain", "ain" ]
5
14
5
[ "pm@gm.com", "pm", "gm.com" ]
null
[ "gm" ]
=!EXPECTEND!=
*/


while (1) {
    puts("input the email");
    email = console.input();
    if (email == undefined) break;
    if ((res = email.match(/([a-zA-Z0-9]+)@([a-zA-Z0-9]+)\.com/))) {
        puts("match at: " + res[0]);
        puts("name: " + res[1]);
        puts("domain: " + res[2]);
    } else {
        puts("invalid email format");
    }
}

var s = 'a b b c d b';
puts(s.replace('b','c'));
puts(s.replace(/b/g,'c'));
puts(s.replace(/b/,'c'));
var str="The rain in SPAIN stays mainly in the plain"; 
puts(str.match(/ain/gi));
puts(str.search(/ain/gi));
puts(str.search(/AIN/));
puts(str.search('ain'));

var s = 'pm@gm.com';
puts(s.match(/^([a-z]*)@([a-z.]*)$/));
puts(s.match(/^([t-z]*)@([a-z.]*)$/));

var r = new RegExp('gm');
puts(s.match(r));


