MySql
=====
<div id="sectmenu"></div>
The MySql driver provides three basic commands for executing sql:

- eval() for running simple sql requiring no input/output.
- onecolumn() like eval, but returns a single value (first column of first row).
- query() for complex, parameterized queries.

A simple session might look like:

    var db = new MySql({user:'root', password:'', database:'jsitest'});
    db.eval('CREATE TABLE players(name VARCHAR(50),age INTEGER);');
    db.query('INSERT INTO players VALUES(?,?);', {values:["Barry",44]});
    var age = db.onecolumn('SELECT age FROM players WHERE name = "Barry";');

A more complete example is [mysql.jsi](../js-demos/mysql.jsi?mimetype=application/javascript).

Options passed in the object argument to new MySql(), may specify any of the following:

| Option      | Type    | Description                                                | Default |
|-------------|---------|------------------------------------------------------------|---------|
| bindWarn    | BOOL    | Treat failed variable binds as a warning.                  | false   |
| database    | STRKEY  | Database to use.                                           |         |
| debug       | CUSTOM  | Enable debug trace for various operations.                 |         |
| enableMulti | BOOL    | Enable muilti-statements for eval().                       |         |
| queryOpts   | CUSTOM  | Default options for exec.                                  |         |
| forceInt    | BOOL    | Bind float as int if possible.                             | true    |
| host        | STRING  | IP address or host name for mysqld (default is 127.0.0.1). |         |
| maxStmts    | INT     | Max cache size for compiled statements.                    |         |
| name        | DSTRING | Name for this db handle.                                   |         |
| password    | STRKEY  | Database password..                                        |         |
| port        | INT     | IP port for mysqld.                                        |         |
| reconnect   | BOOL    | Reconnect.                                                 |         |
| sslKey      | STRING  | SSL key.                                                   |         |
| sslCert     | STRING  | SSL Cert.                                                  |         |
| sslCA       | STRING  | SSL CA.                                                    |         |
| sslCAPath   | STRING  | SSL CA path.                                               |         |
| sslCipher   | STRING  | SSL Cipher.                                                |         |
| user        | STRKEY  | Database user name. Default is current user-name..         |         |

Some of these options can later be changed
using the conf() method, eg.

    db.conf({maxStmts:100});

Refer to the [Reference](Reference.md#MySql)</li> for details.


Eval
----
The eval() method is used to execute simple Sql.
It takes no options, and returns no values.

It can also be used to execute multiple semicolon-separated statements:

    db.conf({enableMulti:true});
    db.exec('CREATE TABLE foo(a,b);'+
    'INSERT INTO foo VALUES(1,2);'+
    'INSERT INTO foo VALUES("X","Y")');

This makes it useful for bulk loading.


Oncolumn
----
onecolumn() provides no inputs or outputs.  It simply returns the first column
of the first row.  The mode and other options are ignored.

    var maxid = db.oncolumn('SELECT max(id) FROM foo');


Query
----
The  workhorse method is query() which:

- compiles SQL into a compiled code, and caches it.
- binds parameters.
- executes the query.
- returns the results.

Here is an example:

    db.query('INSERT INTO players VALUES(?,?);', {values:["Barry",44]});

### Query Options
Query options can be controlled either of two ways.  Per query, as in:

    db.query('SELECT * FROM test1', {mode:'json'});

or we can change the defaults (for the connection) like so:

    db.conf({queryOpts:{mode:'json'}});
    db.query('SELECT * FROM test1');
    db.query('SELECT * FROM test2');

Here is a list of the available query() options:

| Option        | Type   | Description                                                  | Default |
|---------------|--------|--------------------------------------------------------------|---------|
| callback      | FUNC   | Function to call with each row result.                       |         |
| headers       | BOOL   | First row returned contains column labels.                   |         |
| limit         | INT    | Maximum number of returned values.                           |         |
| mapundef      | BOOL   | In variable binds, map an 'undefined' var to null.           |         |
| maxString     | INT    | If not using prefetch, the maximum string value size (0=8K). |         |
| mode          | CUSTOM | Set output mode of returned data.                            |         |
| nocache       | BOOL   | Query is not to be cached.                                   |         |
| noNamedParams | BOOL   | Disable translating sql to support named params.             |         |
| nullvalue     | STRKEY | Null string output (for non-json mode).                      |         |
| paramVar      | ARRAY  | Array var to use for parameters.                             |         |
| prefetch      | BOOL   | Let client library cache entire results.                     |         |
| separator     | STRKEY | Separator string (for csv and text mode).                    |         |
| table         | STRKEY | Table name for mode=insert.                                  |         |
| typeCheck     | CUSTOM | Type check mode.                                             | error   |
| values        | ARRAY  | Values for ? bind parameters.                                |         |
| varName       | STRBUF | String name of array var for ? bind parameters.              |         |
| width         | CUSTOM | In column mode, set column widths.                           |         |


### Outputs
The returned value from a query is determined by the chosen output mode.

The default mode (rows) just returns an array of objects, which looks like this:

    [ { a:1, b:2 }, { a:"X", b:"Y" } ]

The choices for mode are a superset of those available in the sqlite3 command-line tool, namely:

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


We can change the output mode for a query() using:

    db.query('SELECT * FROM foo', {mode:'list'});
    1|2|X
    Y|3|Z

**Note**:
    Output for some modes is affected by the headers and separator options.


#### JSON
The json modes are useful
when data is destined to be sent to a web browser, eg. via [websockets](Builtins.md#websocket).

    db.exec('DROP TABLE IF EXISTS foo; CREATE TABLE foo(a,b);');
    var n = 0, x = 99;
    while (n++ < 3) {
        db.query('INSERT INTO foo VALUES(@x,@n)');
        x -= 4;
    }
    x=db.query('SELECT * FROM foo',{mode:'json'});
    [ {"a":99, "b":1}, {"a":95, "b":2}, {"a":91, "b":3} ]

Where large amounts of data are involved, the headers option can be used to reduce size:

    db.query('SELECT * FROM foo',{mode:'json', headers:true});
    [ ["a", "b"], [99, 1], [95, 2], [91, 3] ]

The "json2" mode is used to split headers and values out
into separate members:

    db.query('SELECT * FROM foo',{mode:'json2'});
    { "names": [ "a", "b" ], "values": [ [99, 1 ], [95, 2 ], [91, 3 ] ] }


#### Callback Function

Normally, query() will execute an entire query before returning the result.
There are two ways to change this:

- set the callback option, or
- give a function as the second argument.

Either way invokes the callback with
each row result:

    function myfunc(n) { puts("a=" + n.a + ", b=" + n.b); }
    db.query('SELECT * FROM foo',myfunc);

If the callback function returns false, evaluation terminates immediately.

    db.query('SELECT * FROM foo', function (n) {
        puts("a=" + n.a + ", b=" + n.b);
        if (a>1) return false;
      });


### Inputs
Sql inputs can be easily formatted using strings:

    var a=1, b='big';
    db.query('INSERT INTO foo VALUES('+a+*','*+b+')');

However this raises issues of security and predictability.
Fortunately variable binding is easy.


#### Bindings

MySql variable binding uses "?" placeholders to refer to array elements., eg:

    db.query('INSERT INTO foo VALUES(?,?)', {values:[11,12]});
    
    var vals = [9,10];
    db.query('INSERT INTO foo VALUES(?,?)', {values:vals});

This approach, for a small number of parameters, is more than adequate.


#### Named-Binds

Jsi enhances MySql's standard variable binding with named binding.
This is modelled after Sqlite named
binding, and works by extracting
named variables from the query, translating them internally into standard "?" form.

A named-bind begin with one of the characters: :,  @, and $.
For example:

    var x1=24.5, x2="Barry", x3="Box";
    db.query('INSERT INTO test2 VALUES( :x1, @x2, $x3 );');

As in Sqlite, the $ bind may append round-brackets () to refer to compound variables.

This example binds to objects members:

    var y = {a:4, b:"Perry", c:"Pack"};
    db.query('INSERT INTO test2 VALUES( $y(a), $y(b), $y(c) );');

And this one to arrays:

    var Z = 2;
    var y = [9, 'Figgy', 'Fall'];
    db.query('INSERT INTO test2 VALUES( $y(0), $y(1), $y([Z]) );');

Or more usefully:

    var y = [
        {a:4, b:"Perry", c:"Pack"},
        {a:9, b:'Figgy', c:'Fall'}
    ];
    for (var i=0; i < y.length; i++)
        db.query('INSERT INTO test2 VALUES($y([i].a), $y([i].b), $y([i].c);');

The contents of the round-brackets can contain multiple levels of dereference (but not expressions).

Here are a few sample bindings, and their associated variables:

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


#### Types
A  type specifier may also be included in a $X(Y) binding, as in:

    var y = {a:4, b:"Purry", c:"Pax"};
    db.query('INSERT INTO test2 VALUES( $y(a:integer), $y(b:string), $y(c:string) );');

The type is the part after the colon ":", and just before the close round-brace.

By default, a type is used to convert data sent to MySql to the correct type.

Type specifiers are supported for all variants of $X(Y) binding, such as:

    var Z = 0;
    var x = ['Figgy'];
    var y = {c:'Fall'};
    db.query('INSERT INTO test3 VALUES( $x(0:string), $y(c:string), $x([Z]:string) );');

The supported type names are:

| Type      | Description           |
|-----------|-----------------------|
| bool      | A tiny/bit value      |
| double    | A double value        |
| integer   | A 64 bit wide integer |
| string    | A string              |
| blob      | A blob                |
| date      | A date value          |
| datetime  | A date+time value     |
| time      | A time value          |
| timestamp | A unix timestamp      |


We can also change the type-checking behaviour via the typeCheck query option:

For example, we can instead cause an error to be kicked an error with:


    var x = [ 'bad' ];
    db.query('UPDATE test SET n = $x(0:number) );', {typeCheck:'error'});

The valid typeCheck modes are:

| Value   | Description                                      |
|---------|--------------------------------------------------|
| convert | Coerce value to the requested type (the default) |
| warn    | Generate a warning                               |
| error   | Generate an error                                |
| disable | Ignore type specifiers                           |


Miscellaneous
----

### User Functions
SQL functions can be defined in javascript using func():

    db.func('bar',function(n) { return n+'.000'; });
    puts(db.onecolumn('SELECT bar(a) FROM foo WHERE b == 2;'));


### Timestamps
Javascript time functions use the unix epoch to store the number of milliseconds
since Jan 1, 1970 UTC.

MySql stores dates/time in an internal format with one of the following types:

| Value     | Description           |
|-----------|-----------------------|
| TIMESTAMP | A unix date/timestamp |
| DATETIME  | A date and time       |
| DATE      | A date                |
| TIME      | a time                |


### Caching
In the interest of efficiency, compiled queries are cached on a per connection basis.
The size of the cache is controlled by the maxStmts option.

You can also disable caching for individual querys with nocache.


### Differences From Sqlite
Differences include a greater dependance on types, and requiring a user, password and database.


### Building
The MySql driver does not (by default) come builtin to Jsi.

However, once you [download the source](Build.md) you can build it in with:

    make mysql

Alternatively, the shared library can be built (for unix) with:

    make libmysql
