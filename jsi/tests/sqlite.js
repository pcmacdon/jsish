/*
=!EXPECTSTART!=
B=1, A=99
B=2, A=95
B=3, A=91
[ { a:99, b:1 }, { a:95, b:2 }, { a:91, b:3 } ]
CONF: { bindWarn:false, debug:[  ], errorCnt:0, forceInt:false, maxStmts:1000, mutex:"default", name:"", nocreate:false, numSort:0, numStep:2, numStmts:2, queryOpts:{ Cdata:null, callback:null, headers:false, limit:0, mapundef:false, mode:"rows", nocache:false, nullvalue:null, separator:null, table:null, typeCheck:"convert", values:null, varName:"", width:undefined }, readonly:false, vfs:null }
95
EXECING: select * from foo;
[ { a:99, b:1 }, { a:95, b:2 }, { a:91, b:3 } ]
EXECING: select * from foo;
99|1
95|2
91|3
EXECING: select * from foo;
a|b
99|1
95|2
91|3
EXECING: select * from foo;
<TR><TD>99</TD><TD>1</TD></TR>
<TR><TD>95</TD><TD>2</TD></TR>
<TR><TD>91</TD><TD>3</TD></TR>
EXECING: select * from foo;
<TR><TH>a</TH><TH>b</TH></TR>
<TR><TD>99</TD><TD>1</TD></TR>
<TR><TD>95</TD><TD>2</TD></TR>
<TR><TD>91</TD><TD>3</TD></TR>
EXECING: select * from foo;
99,1
95,2
91,3
EXECING: select * from foo;
a,b
99,1
95,2
91,3
EXECING: select * from foo;
a          b          99         1         
95         2         
91         3         
EXECING: select * from foo;
a          b           
---------- ----------
99         1         
95         2         
91         3         
EXECING: select * from foo;
[ {"a":99, "b":1}, {"a":95, "b":2}, {"a":91, "b":3} ]
EXECING: select * from foo;
[ ["a", "b"], [99, 1], [95, 2], [91, 3] ]
EXECING: select * from foo;
{ "names": [ "a", "b" ], "values": [ [99, 1 ], [95, 2 ], [91, 3 ] ] } 
EXECING: select * from foo;
99	1
95	2
91	3
EXECING: select * from foo;
a	b
99	1
95	2
91	3
EXECING: select * from foo;
    a = 99    b = 1
    a = 95
    b = 2
    a = 91
    b = 3
EXECING: select * from foo;
INSERT INTO table VALUES(99,NULL);
INSERT INTO table VALUES(95,NULL);
INSERT INTO table VALUES(91,NULL);
EXECING: insert into foo values(?,?);
EXECING: insert into foo values(?,?);
EXECING: insert into foo values(?,?);
TRACING: select bar(a) from foo where b == 2;
95.000
function (sql) {...}
=!EXPECTEND!=
*/


var db = new Sqlite('/tmp/testsql.db',{maxStmts:1000, readonly:false});
//var db = new Sqlite('/tmp/testsql.db');

db.eval('drop table IF EXISTS foo;');
try {
  db.query('drop table foo;');
} catch(e) {
  //puts("OK");
};

db.eval('drop table IF EXISTS foo; create table foo(a,b);');
//db.eval('drop table IF EXISTS foo; create table foo(a,b);');
var n = 0;
var x = 99;
while (n++ < 3) {
  db.query('insert into foo values(@x,@n);');
  x -= 4;

  //db.query('insert into foo values("x",' + n + ');');
}
db.query('select * from foo;', function (n) {
    puts("B="+n.b + ", A="+n.a);
});

puts(db.query('select * from foo;').toString());

db.query('select * from foo;');

puts("CONF: "+db.conf().toString());
db.profile(function(sql,time) { puts("EXECING: "+sql); });

puts(db.onecolumn('select a from foo where b == 2;'));
var s = {};
s.b = 2;

puts(db.query('select * from foo;'));
puts(db.query('select * from foo;',{mode:'list'}));
puts(db.query('select * from foo;',{mode:'list',headers:true}));
puts(db.query('select * from foo;',{mode:'html'}));
puts(db.query('select * from foo;',{mode:'html',headers:true}));
puts(db.query('select * from foo;',{mode:'csv'}));
puts(db.query('select * from foo;',{mode:'csv',headers:true}));
puts(db.query('select * from foo;',{mode:'column'}));
puts(db.query('select * from foo;',{mode:'column',headers:true}));
puts(x=db.query('select * from foo;',{mode:'json'}));
JSON.parse(x);
puts(x=db.query('select * from foo;',{mode:'json',headers:true}));
JSON.parse(x);
puts(x=db.query('select * from foo;',{mode:'json2'}));
JSON.parse(x);
puts(db.query('select * from foo;',{mode:'tabs'}));
puts(db.query('select * from foo;',{mode:'tabs',headers:true}));
puts(db.query('select * from foo;',{mode:'line'}));
puts(db.query('select * from foo;',{mode:'insert'}));

var binds = [91,3];
db.query('insert into foo values(?,?);', {varName:'binds'});
db.query('insert into foo values(?,?);', {values:binds});
db.query('insert into foo values(?,?);', {values:[91,3]});

db.func('bar',function(n) { return n+'.000'; });


db.trace(function(sql) { puts("TRACING: "+sql); });
puts(db.onecolumn('select bar(a) from foo where b == 2;'));
puts(db.trace());

//puts(db.version());
delete db;


/*
var res1 = db.query('select * from table foo(a,b);');
for (i in res1) {
  puts("CONS: "+i.toString()); 
}


var curs, n;
for (curs = db.query('select * from table foo(a,b);'),
    (n = db.getresult(curs))!=undefined,
    db.nextresult(curs)) {
    puts(n.toString());
}
db.endresult(curs);


*/
