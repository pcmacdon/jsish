Sqlite
====
<div id="sectmenu"></div>
Starting
====
The simplest use of [Sqlite](Reference.md#Sqlite) is with **%s** and an [object](#Objects):

    var db = new Sqlite();
    var lite = {set:null, x:1, y:'big', z:false};
    db.query('CREATE TABLE Bag %s', 'lite');
    db.query('INSERT INTO  Bag %s', 'lite');
    puts(db.query('SELECT * FROM Bag'));

==>

    [ { set:null, x:1, y:"big", z:false } ]
    
Query
====

    function query(sql:string, options:function|string|array|object=void)

The first argument to **query** is an sql statement.
An optional second argument is:

|Type|Option|Description
|----|----|----
|string|objName|Object var name for CREATE/INSERT: replaces %s with fields in query.
|array|values|Values for ? bind parameters.
|function|callback|Function to call with each row result. @function(values:object).
|object||Options|

Options
----
Query [options](./Reference.md#Sqlite.queryOptions) can be controlled either of two ways.  Per query, as in:

    db.query('SELECT * FROM Bag', {mode:'json'});

or per connection, as in this where the default output is changed to **json**.

    db.conf({queryOpts:{mode:'json'}});



Output
----
The output **mode**, or what gets returned value from a query, is one of:

| Mode    | Description                                      | Purpose |
|---------|--------------------------------------------------|---------|
| array1d | Flat array of values                             | script  |
| arrays  | An array of row-arrays                           | script  |
| column  | Column aligned text                              | text    |
| csv     | Comma (or separator) separated values            | export  |
| html    | Html table rows                                  | browser |
| insert  | Sql insert statements                            | export  |
| json    | JSON string as an array of objects               | browser |
| json2   | JSON string with names/values in separate arrays | browser |
| line    | One value per line in name=value form            | export  |
| list    | The default sqlite3 output                       | text    |
| none    | No output                                        |         |
| rows    | An array of row-objects (the default)            | script  |
| tabs    | Tab separator delineated values                  | script  |


Here is an example:

    db.query('SELECT * FROM Bag', {mode:'column',header:true});

==>

    set        x          y          z             
    ---------- ---------- ---------- ----------
               1          big        false     

**Note**:
    The **headers** and **separator** options do not apply to all modes.


### JSON
The json modes are useful
when data is destined to be sent to a web browser, eg. via [websockets](Builtins.md#websocket).

    CREATE TABLE foo(a,b);
    var n = 0, x = 99;
    while (n++ < 3) {
        db.query('INSERT INTO foo VALUES(@x,@n)');
        x -= 4;
    }
    x=db.query('SELECT * FROM foo',{mode:'json'});

==>

    [ {"a":99, "b":1}, {"a":95, "b":2}, {"a":91, "b":3} ]

Where large amounts of data are involved, the headers option can be used to reduce size:

    db.query('SELECT * FROM foo',{mode:'json', headers:true});
    
==>

    [ ["a", "b"], [99, 1], [95, 2], [91, 3] ]

Or use **json2** to split headers and values out:

    db.query('SELECT * FROM foo',{mode:'json2'});

==>

    { "names": [ "a", "b" ], "values": [ [99, 1 ], [95, 2 ], [91, 3 ] ] }


### Callback
Normally, query() will execute an entire query before returning the result.
Alternatively use **callback** to get rows one at a time:

    function myfunc(n) { puts("a=" + n.a + ", b=" + n.b); }
    db.query('SELECT * FROM foo', {callback:myfunc});

If the callback function returns false, evaluation will terminate.

    db.query('SELECT * FROM foo', function (n) {
        puts("a=" + n.a + ", b=" + n.b);
        if (a>1) return false;
      });



Input
----
Sqlite input binding is supported as follows.

### Arrays

Array elements can be bound using **values**:

    db.query('INSERT INTO foo VALUES(?,?)', [11,12]); // or...
    db.query('INSERT INTO foo VALUES(?,?)', {values:[11,12]});


### Objects
This binding method
uses the name of an object variable with **%s**,
which expands to the appropriate schema for CREATE, or bind for INSERT.

#### CREATE
A *CREATE* binds expands **%s** into a schema:

    var lite = {w:null, x:1, y:'big', z:false};
    db.query('CREATE TABLE IF NOT EXISTS Bag %s', 'lite');

The resulting schema here is:

    CREATE TABLE Lite ('set' TEXT,x NUMERIC,y TEXT,z BOOLEAN)

Note it is quoted and typed such that input/output will work correctly.

#### INSERT
An *INSERT* bind expands **%s** to *names* and *values*:

    db.query('INSERT INTO  Bag %s', 'lite');
    puts(db.query('SELECT * FROM Bag'));

==>

    [ { set:null, x:1, y:"big", z:false } ]

We can also do the same thing using **objName**:

    db.query('INSERT INTO  Bag %s', {objName:'lite',mode:'json'});

The generated schemas can be dumped using [echo](./Reference.md#Options for new Sqlite)

    var db = new Sqlite(null, {echo:true});
    var lite = {set:null, x:1, y:'big', z:false};
    db.query('CREATE TABLE Bag %s', 'lite');
    db.query('INSERT INTO  Bag %s', 'lite');

==>

    info: SQL-ECHO: CREATE TABLE Lite ('set' TEXT,x NUMERIC,y TEXT,z BOOLEAN) 
    info: SQL-ECHO: INSERT INTO  Lite ('set',x,y,z)  VALUES($lite(set),$lite(x),$lite(y),$lite(z))

#### CHARJSON
Fields containing array or object (non-primitives) will CREATE with
column type **CHARJSON**.  Then upon INSERT the item is *JSON-stringified*,
and upon SELECT *JSON-parsed*:

    var db = new Sqlite();
    var mix = {ar:[1,'b',true], ob:{x:9,k:'b'}};
    db.query('CREATE TABLE Bin %s', {objName:'mix',echo:true});
    db.query('INSERT INTO  Bin %s', 'mix');
    var out = db.query('SELECT * FROM Bin')[0];
    puts(out.ob.x + out.ar[0]);

==>

    info: SQL-ECHO: CREATE TABLE Bin (ar CHARJSON,ob CHARJSON)
    10

Note this works only for binds using the **$()** form, and can
be disabled with [noJsonConv](./Reference.md#Options for new Sqlite).

### Variables
Named-bindings to variables begin with: **:**,  **@**, and **$**, eg:

    var x1=24.5, x2="Barry", x3="Box";
    db.query('INSERT INTO test2 VALUES( :x1, @x2, $x3 )');

The **$()** bind references object members, eg:


    var y = {a:4, b:"Perry", c:"Pack"};
    db.query('INSERT INTO test2 VALUES( $y(a), $y(b), $y(c) )');


or arrays:

    var Z = 2;
    var y = [9, 'Figgy', 'Fall'];
    db.query('INSERT INTO test2 VALUES( $y(0), $y(1), $y([Z]) )');

and even:

    var y = [
        {a:4, b:"Perry", c:"Pack"},
        {a:9, b:'Figgy', c:'Fall'}
    ];
    for (var i=0; i < y.length; i++)
        db.query('INSERT INTO test2 VALUES($y([i].a), $y([i].b), $y([i].c)');

Bindings are enumerated here:

| Binding        | Variable    | Comment                         |
|----------------|-------------|---------------------------------|
| :X             | X           |                                 |
| @X             | X           |                                 |
| $X             | X           |                                 |
| $X(a)          | X.a         | Implicit object member          |
| $X(9)          | X&lsqb;9]   | Implicit array (leading digits) |
| $X(&lsqb;a])   | X&lsqb;a]   | Explicit array                  |
| $X(a.b)        | X.a.b       | Compound object                 |
| $X(&lsqb;a].b) | X&lsqb;a].b | Compound array + object, etc    |


### Types
Types are mostly useful for MySql, but are also supported in Sqlite:  [see MySql Types](./MySql.md#Types).

Eval
====

    function eval(sql:string):number

The eval() method is used to execute simple Sql.
It takes no options, and returns number of rows changed (sqlite3_changed).

It can also be used to execute multiple semicolon-separated statements:

    db.eval('CREATE TABLE foo(a,b);'+
    'INSERT INTO foo VALUES(1,2);'+
    'INSERT INTO foo VALUES("X","Y")');

This makes it useful for bulk loading.


Oncolumn
====

	function onecolumn(sql:string)
    
onecolumn() provides no inputs or outputs.  It simply returns the first column
of the first row.

    var maxid = db.oncolumn('SELECT max(id) FROM foo');

Miscellaneous
====

User Functions
----
SQL functions can be defined in javascript using func():

    db.func('bar',function(n) { return n+'.000'; });
    puts(db.onecolumn('SELECT bar(a) FROM foo WHERE b == 2;'));


Timestamps
----
Sqlite performs internal time calculations based on the Gregorian calendar.

But Javascript time functions use the unix epoch to store the number of milliseconds
since Jan 1, 1970 UTC.

We can create a table with a DEFAULT date field as a number using:

    CREATE TABLE mtable(
      name,
      mytime DATETIME DEFAULT(round((julianday('now') - 2440587.5)*86400000.0))
    );

We can output this as a text time stamp using:

    SELECT strftime('%Y-%m-%d %H:%M:%f',mytime/1000.0,'unixepoch') FROM mytable;
    2015-01-12 13:46:40.252
    SELECT strftime('%Y-%m-%d %H:%M:%f',mytime/1000.0,*'unixepoch'*,'localtime') FROM mytable;
    2015-01-12 18:46:40.252


Caching
----
Size of the compiled query cache is controlled by the maxStmts option,
although you can disable caching for individual queries using **nocache**,
or by beginning a query with **';'**.


Building
----
The Sqlite driver comes (by default) builtin to Jsi.

It can also be built the shared library can be built (for unix) with:

    make libmysql


C-API
====

See [DbQuery](DBQuery.md) for a Sqlite C-API.

