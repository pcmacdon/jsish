CData
====
<div id="sectmenu"></div>
The CData interface provides:

- Generation of C-extension modules containing code and data-structs.
- Script-defined C-structs, accessed via the [CData](Reference.md#CData) commands.

**Note**: All examples below that start `"cat XXX <<EOF"` are bash testable with a simple copy+paste. 

C-Code
====

**FYI**, the following presumes you have a current, and active [download/build](Download.md) of Jsi.

Module
----
C modules generate from a **.jsc** file,
leveraging [typed](Types.md) *function-signatures* to provide
argument checking and conversion.

This example creates a simple C module, compiled as the shared library **Tiny.so**:

    cat > Tiny.jsc <<EOF
    extension Tiny = { // A Tiny C Extension
    
        function range(size:number=10, start:number=0, step:number=1):array { // Fill a range.
            Jsi_Value *a = Jsi_ValueMakeArrayObject(interp, ret, NULL);
            Jsi_RC rc = JSI_OK;
            int n=size-1, i=start+n*step;
            for (; n>=0&&rc==JSI_OK; i-=step,n--)
                rc = Jsi_ValueInsertArray(interp, a, n, Jsi_ValueNewNumber(interp, i), 0);
            return rc;
        }
    };
    EOF
    
    jsish -c Tiny.jsc
    cc `jsish -c -cflags true Tiny.so`
    
    jsish -e 'require("Tiny"); return Tiny.range();'
    jsish -e 'require("Tiny"); return Tiny.range(8,0,2);'

==>

    [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ]
    [ 0, 2, 4, 6, 8, 10, 12, 14 ]

Note: module generation takes care of all aspects of package generation and option parsing.

Standalone
----
If you omit the **.so** (or use a **.c**), *Tiny* will build as a standalone:

    cc `jsish -c -cflags true Tiny`
    
    jsish -z zvfs Tiny
    
    ./Tinysh -e 'return Tiny.range();'
    ./Tinysh -e 'return Tiny.range(8,0,2);'

The **jsish -z zvfs** is used to zip-append the */zvfs* filesystem to the end of the executable.

### Help
The help system is extension aware:

    ./Tinysh
    Jsish interactive: see 'help [cmd]'
    # help Tiny
    Tiny.method(...)
    
    Methods: range
    
    # help Tiny.range
    Tiny.range(size:number=10, start:number=0, step:number=1)
    Fill a range..

### Scripts and Autoload
If you like, you can add custom scripts to the standalone, eg:

    mkdir mydir
    cat > mydir/range.jsi <<EOF
    function range(n=20, start=0, step=1) {
        return Array(n).fill(0).map(function(v,i,o) { return start+i*step; });
    }
    function range1(n=20, start=0, step=1) {
       var a = Array(n).fill(0);
       for (var i in a)
          a[i] = start+i*step;
       return a;
    }
    EOF

...

    cat > mydir/autoload.jsi <<EOF
    Jsi_Auto.range    = 'source("'+Info.scriptDir()+'/range.jsi")';
    Jsi_Auto.range1    = 'source("'+Info.scriptDir()+'/range.jsi")';
    EOF

...

    jsish -z -userDir mydir zvfs Tiny
    Tinysh -e "range()"

==>
    
    [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 ]

Options
----
Commands can take a trailing option argument, for parsing into a C-struct:

    cat > Bag.jsc <<EOF
    // Simple range functions using of CAPI.
    
    struct Bag_ranger = { // Struct used by ranger()
        int size = 10;        //
        int start;            // Note: 0 is default in structs.
        int step = 1;
    };
    
    extension Bag = { // A C Extension
        // C code starts after the last comment.
        #include <stdio.h>
        
        function range(size:number=10, start:number=0, step:number=1):array { // Fill a range.
            /* C-code. */
            Jsi_Value *a = Jsi_ValueMakeArrayObject(interp, ret, NULL);
            Jsi_RC rc = JSI_OK;
            int n=size-1, i=start+n*step;
            for (; n>=0&&rc==JSI_OK; i-=step,n--)
                rc = Jsi_ValueInsertArray(interp, a, n, Jsi_ValueNewNumber(interp, i), 0);
            return rc;
        }
    
        function ranger(options:object=void):array { // Fill a range using config
            /* C code. */
            Jsi_Value *a = Jsi_ValueMakeArrayObject(interp, ret, NULL);
            Jsi_RC rc = JSI_OK;
            int n=data.size-1, i=data.start+n*data.step;
            for (; n>=0&&rc==JSI_OK; i-=data.step, n--)
                rc = Jsi_ValueInsertArray(interp, a, n, Jsi_ValueNewNumber(interp, i), 0);
            return rc;
        }
       
    };
    EOF

    jsish -c Bag.jsc
    cc `jsish -c -cflags true Bag.so`
    
    jsish -e 'require("Bag"); return Bag.range();'
    jsish -e 'require("Bag"); return Bag.ranger({size:8,step:2});'

==>

    [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ]
    [ 0, 2, 4, 6, 8, 10, 12, 14 ]

An advantage of using [Structs](#Structs) for option parsing is the great variety of
types available.

Applicability
----
There are 4 different situations where option structs may be applied:

- [Bag.jsi](../c-demos/cdata/Bee.jsc?mimetype=application/javascript): Struct private to a single method (as above).
- [Bee.jsc](../c-demos/cdata/Bee.jsc?mimetype=application/javascript): A global data struct visible from script.
- [Baker.jsc](../c-demos/cdata/Baker.jsc?mimetype=application/javascript): An interp-private data struct.
- [Car.jsc](../c-demos/cdata/Car.jsc?mimetype=application/javascript): A constructor, eg. **new Car()**.

Here is a test script: [ext_test.jsi](../c-demos/cdata/ext_test.jsi?mimetype=application/javascript).


Conventions
----
- a function with the same name as the object will be treated as an Constructor.
- **conf** is the configuration command and must have a trailing **options:object**.
- any other function with trailing **options:object** supports per-command options.

### Parameters

- Parameters are unloaded into function-local vars based on their type (singular).
- [Type Unions](Types.md#Type Unions) are currently unsupported and simply ignored.
- A difference from JS signatures is [implicit types](Types.md#Implicit Types) are not supported.


### Vars
If you define a **"var"** with whose prefix matches name, but ending with `_Data` (eg. `Bee_Data` above)
you can access the state via `CData.get()` etc (non-object commands only).


Returns
----
To return a function values, the `RETURN()` statement is used on a line by itself.
The function return type determines what actually gets returned.  As with parameters, this must not be a
"Type Union".


Enums
----
*Enums* for use by structs are defined as follows:

    enum BeeType = {
        Drone,
        Worker,
        Queen
    };
    
    struct Bee = {
        int max;
        int buzzCnt;
        int stingCnt;
        int pollinateCnt;
        BeeType type;
        STRING32 flower;
    };

Structs
----
Structs are a collection of fields.

### Fields
The type of a struct field is one of:

- a **base type**
- a **struct**
- an array of **base type** or **struct**
- an **enum**
- a *bitfield* of int type or **enum**
- a *bitset* of an **enum** (indicated by name ending in **@**)

See [Field Specification](C-API.md#Field Specification).


### Base Types
The base field types are:

    bool int8_t int16_t int32_t int64_t uint8_t uint16_t
    uint32_t uint64_t float double ldouble Jsi_Strbuf time_w time_d time_t size_t
    intptr_t uintptr_t Jsi_Number int uint long ulong short ushort Jsi_Value*
    Jsi_DString Jsi_Value* Jsi_Value* Jsi_Value* Jsi_Value* Jsi_Value* Jsi_Value* Jsi_Value*
    Jsi_Strkey const char

Plus *char arrays* and **STRING**n for powers-of-2 up to 64k.


### Example
Test output is show from [cdatatest.jsi](../c-demos/cdata/cdatatest.jsi?mimetype=application/javascript).

Here are an excerpt:

    CStruct.conf('Bee') ==> { crc:0, flags:0, help:null, idx:4, name:"Bee", size:6, ssig:0, value:10 }
    CStruct.conf('Bee','name') ==> Bee
    CStruct.names() ==> [ "Bkey", "Pest", "Bee", "Flower" ]
    CStruct.names('Bee') ==> []
    CStruct.fieldconf('Bee', 't') ==> { arrSize:0, bits:0, boffset:7, flags:1, help:null, idx:2, info:null, init:8, name:"t", offset:1, size:4, type:"int" }
    CStruct.fieldconf('Bee', 's', 'bits') ==> 4
    CStruct.get('Bee') ==> { fields:[ { bitoffs:0, bitsize:4, id:"int", isbit:0, label:"", name:"s", offset:0, size:4 }, { bitoffs:4, bitsize:3, id:"int", isbit:0, label:"", name:"r", offset:0, size:4 }, { bitoffs:7, bitsize:0, id:"int", isbit:0, label:"", name:"t", offset:1, size:4 }, { bitoffs:39, bitsize:0, id:"Fruit", isbit:0, label:"", name:"k", offset:5, size:1 } ], name:"Bee", size:6 }

Uninitialized fields will default to zero.

CType
----
CType is simply available to query available types.

Here is another test output from [cdatatest.jsi](#Example):

    CType.names().sort() ==> [ "ARRAY", "BOOL", "CUSTOM", "DOUBLE", "DSTRING", "FLOAT", "FUNC", "INT", "INT16", "INT32", "INT64", "INT8", "INTPTR_T", "LDOUBLE", "LONG", "NUMBER", "OBJ", "REGEXP", "SHORT", "SIZE_T", "SSIZE_T", "STRBUF", "STRING", "STRKEY", "TIME_D", "TIME_T", "TIME_W", "UINT", "UINT16", "UINT32", "UINT64", "UINT8", "UINTPTR_T", "ULONG", "USEROBJ", "USHORT", "VALUE", "VAR" ]
    CType.names(true).sort() ==> [ "Bee", "Bkey", "Flower", "Fruit", "Herd", "Jsi_DString", "Jsi_Number", "Jsi_Strbuf", "Jsi_Value", "Pest", "Vegetable", "bool", "const char", "double", "float", "int", "int16_t", "int32_t", "int64_t", "int8_t", "intptr_t", "ldouble", "long", "short", "size_t", "ssize_t", "time_d", "time_t", "time_w", "uint", "uint16_t", "uint32_t", "uint64_t", "uint8_t", "uintptr_t", "ulong", "ushort" ]
    CType.conf('INT') ==> { cName:"int", flags:0, fmt:"d", help:"Integer", idName:"INT", size:4, user:0, xfmt:"#x" }
    CType.conf('INT','flags') ==> 0

See [Option Types](#Structs).

Variables
----
Many types of variables are available.

Here, an example from [c-demos/cdata/demo0.jsc](../c-demos/cdata/demo0.jsc?mimetype=application/javascript)
defines a number of vars using two structs: **Foo** and **Bar**:

    vars MyVars = {
        Bar     bar;        // Struct Bar.
        Foo     foo;        // Struct Foo.
    
        Foo     foos[10];   // Array of Foo structs.
        Foo     foos2[FooSize];// Array of Foo structs with enum size.
        Bar     barss[10];  // Array of Bar.
    
        Bar     bars{};     // Map-Tree with string-key.
        Bar     BN{0};      // Map-Tree with integer-key.
        Bar     Bs{@Fidx};   // Map-Tree with struct-key.
    
        Bar     Bs2{#};     // Map-Hash with string-key
        Bar     BN2{#0};    // Map-Hash with integer key.
        Bar     Bs2a{#@Fidx};// Map-Hash with struct-key.
    };

Three different types of **struct**-vars can be declared:

- Simple structs
- Fixed size arrays of structs, denoted by square brackets.
- Maps of structs, denoted by curley braces.

For maps, which can store large amounts of data:

- The **map-type** default is *tree*, unless over-ridden to a *hash*, eg: **{#}**.
- The **key-type** default is *string*, but can be over-ridden as *integer* **{0}** or *struct* **{@Name}**.



Signatures
----
Signatures, which are important when sending raw struct data over the network,
are enabled if the first field of a struct is named **sig**.

If unassigned, the *sig* value is
a *crc32* calculated against the struct definition types (ie. after being stripping of names and comments).



Script
====
The Jsi script commands for defining *structs* and *enums* are:
**CStruct**, **CEnum**, **CData**.
These share the same internal API as [C-Code](#C-Code).

Struct Options
----
This example uses a struct to parse options.

    cat > /tmp/bee.jsi <<EOF
    CEnum('Fruit', 'apple,banana,orange,grape');
    CStruct('Bee', 'int class:4; int size:4=7; Fruit fruit;');
    
    function bee(options:object) {
        var d  = new CData('Bee', options);
        return d.get();
    }
    
    bee({size:2});
    bee({size:3, class:1, fruit:'orange'});
    bee({size:19}); // FAILS: not 7 bits.
    EOF
    
    jsish /tmp/bee.jsi

==>

    /tmp/bee.jsi:10: error: for bitfield option "size": invalid value (at or near "19")

The larger range of types in structs expand type-checking capabilities.

CEnum
----
Three forms are used for enum definitions. The
simplest being single-line, split on commas:

    CEnum.define('Fruit', 'apple,banana,orange,grape=9');

The multi-line form is split on newlines, with comments extracted as help:

    CEnum.define('Vegetable', '
        corn=0, // My favorite
        peas=2, // Your favorite
        potato=-1
    ');

Finally, there is an object form:

    CEnum.define({name:'Herd'}, [ {name:'sheep'}, {name:'goat', value:3, help:'set a value'}, {name:'cow'}]);

Here is another test output from [cdatatest.jsi](#Example):

    CEnum.names() ==> [ "Fruit", "Herd", "Vegetable" ]
    CEnum.names('Fruit') ==> [ "apple", "banana", "orange", "grape" ]
    CEnum.conf('Fruit') ==> { flags:0, help:null, idx:4, name:"Fruit" }
    CEnum.conf('Fruit', 'idx') ==> 4
    CEnum.find('Fruit', 1) ==> banana
    CEnum.fieldconf('Fruit', 'banana') ==> { flags:0, help:null, idx:1, name:"banana", value:1 }
    CEnum.fieldconf('Fruit', 'banana', 'value') ==> 1
    CEnum.value('Fruit', 'apple') ==> 0
    CEnum.get('Fruit') ==> { fields:[ { name:"apple", value:0 }, { name:"banana", value:1 }, { name:"orange", value:2 }, { name:"grape", value:9 } ], name:"Fruit" }


CStruct
----
As with CEnum, there are three forms for struct definitions.
The simplest being single-line, split on semicolons:

    CStruct.define('Bee', 'int s:4; int r:3; int t=8; Fruit k;');

The multi-line form is split on newlines, with comments extracted as help:

    CStruct.define('Pest', '
        int x=3;    // int field.
        Bee b;      // A sub-struct
        int y=5;
    ');

And the object form:

    CStruct.define({name:'Bkey', help:'Struct to use for a key'}, [
            {name:'a', type:'int', help:'first key field'},
            {name:'b', type:'int', help:'second key field'}]
    );


CData
----
A basic JS use of CData to create and manipulate a single struct is:

    var alpha  = new CData('Bee');
    alpha.get(null, 't');
    alpha.set(null, 't', 4);

Data is accessed with set/get/incr:

    alpha.get(null) ==> { k:"apple", r:0, s:0, t:8 }
    alpha.get(null, 't') ==> 8
    alpha.set(null, 't', 4) ==> undefined
    alpha.incr(null, 't', 1) ==> 5
    n=alpha.get(null, 't') ==> 5

As above there is also an object form:

    var bees   = new CData({structName:'Bee', arrSize:10, help:'An array of bees'});

The string form is the simplest way to create [maps](C-API.md#c-api/jsi-lite) and
[hashes](C-API.md#c-api/jsi-lite) of structs.

    var alpha, beta, tree, tree2, tree3, hash, hash2, hash3, pest, flower, n;
    beta   = new CData('Bee[10]');      // Array
    tree   = new CData('Bee{}');        // Map with string key
    tree2  = new CData('Bee{0}');       // Map with number key
    tree3  = new CData('Bee{@Bkey}');   // Map with struct key
    hash   = new CData('Bee{#}');       // Hash with string key
    hash2  = new CData('Bee{#0}');      // Hash with number key
    hash3  = new CData('Bee{#@Bkey}');  // Hash with struct key


which are indexed with a non-null key:

    beta.set(0, 't', 2);
    tree.set('X', 't', 2);
    hash.set('X', 't', 2);


Struct keys are also supported:

    bkey={a:1,b:2} ==> { a:1, b:2 }
    tree3.set(bkey, 't', 2) ==> undefined
    tree3.get(bkey, 't') ==> 2


Database
----
Database definitions may be extracted from struct definitions and used with Sqlite.

Here is another test output from [cdatatest.jsi](#Example):

    var db = new Sqlite('/tmp/bees.db') ==> "#Sqlite_1"
    schema = CStruct.schema('Bee') ==>
      s int
     ,r int
     ,t INT
     ,k TEXT
      -- MD5=2c2573332fca5f166b7272366bd888b0
    db.eval('CREATE TABLE IF NOT EXISTS Bee (
    '+schema+'
    );') ==> undefined
    db.query('INSERT INTO Bee %s',{cdata:'beta'}) ==> 10
    db.query('SELECT %s FROM Bee',{cdata:'beta'}) ==> 10


Limitations
----
JS-defined structs differ from C-defined ones in that they:

- are not visible to sub-interps,
- may be incompatible with actual C-structs due to packing differences, and
- not can not be used with C-extensions.

