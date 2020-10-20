Extensions
===
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

While the best way to write an extension is modifying an existing one by hand,
that is non-trivial.

Alternatively you can use the simplified generation of C-extensions from **.jsc** files.
It leverages [typed](Functions.md) function providing
argument checking and conversion.

## Example

Here's a simple C extension embedded a JS wrapper:

``` c
extension Tiny = { // A Tiny C Extension
function range(size:number=10, start:number=0, step:number=1):array { // Fill a range.
        // BEGIN C-code
        Jsi_Value *a = Jsi_ValueMakeArrayObject(interp, ret, NULL);
        Jsi_RC rc = JSI_OK;
        int n=size-1, i=start+n*step;
        for (; n>=0&&rc==JSI_OK; i-=step,n--)
            rc = Jsi_ValueInsertArray(interp, a, n, Jsi_ValueNewNumber(interp, i), 0);
        return rc;
        // END C-code
}
};
```

Generate and compile as shared library **Tiny.so**:
```
jsish -c -compile true Tiny.jsc
```
and tested with:

```
jsish -e 'require("Tiny"); return Tiny.range();'
jsish -e 'require("Tiny"); return Tiny.range(8,0,2);'
[ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ]
[ 0, 2, 4, 6, 8, 10, 12, 14 ]
```

Note: module generation takes care of all aspects of package generation and option parsing.

+++ Help

The help system is extension aware:

```
./Tinysh -h Tiny
Tiny.method(...)
A Tiny C Extension
Methods: range
```

and

```
./Tinysh -h Tiny.range
Tiny.range(size:number=10, start:number=0, step:number=1)
Fill a range..
```

+++

+++ Standalone

To build as a standalone, omit the **.so** (or use a **.c**): 

```
cc `jsish -c -cflags true Tiny`
jsish -z zvfs Tiny
./Tinysh -e 'return Tiny.range();'
[ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ]
./Tinysh -e 'return Tiny.range(8,0,2);'
[ 0, 2, 4, 6, 8, 10, 12, 14 ]
```

Note: The **jsish -z zvfs** is used to zip-append the */zvfs* filesystem to the end of the executable.

+++

## Scripts
If you like, you can add custom scripts to the standalone, eg:

``` bash
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

cat > mydir/autoload.jsi <<EOF
Jsi_Auto.range    = 'source("'+Info.scriptDir()+'/range.jsi")';
Jsi_Auto.range1    = 'source("'+Info.scriptDir()+'/range.jsi")';
EOF
```

Generate and test with:

```
jsish -z -userDir mydir zvfs Tiny
./Tinysh -e "range()"
[ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 ]
```

## Options

Commands with option arguments can be parsed into a C-struct:

``` bash
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
```

An advantage of using [Structs](#structs) for option parsing is the great variety of
types available.

## Models

There are 4 different models where option structs may be applied:


|Model  |Example| Description |
|-------|-------|-------------|
|single | [Bag.jsc](https://jsish.org/fossil/jsi/file/c-demos/cextn/Bag.jsc) | Struct private to a single method (as above).|
|global | [Bee.jsc](https://jsish.org/fossil/jsi/file/c-demos/cextn/Bee.jsc) | An global data struct.|
|private| [Baker.jsc](https://jsish.org/fossil/jsi/file/c-demos/cextn/Baker.jsc)| An interp-private data struct.|
|new    | [Car.jsc](https://jsish.org/fossil/jsi/file/c-demos/cextn/Car.jsc) | Constructed, eg. **new Car()**.|

Here is a test script: [ext_test.jsi](https://jsish.org/fossil/jsi/file/c-demos/cextn/ext_test.jsi).


## Conventions

- a function with the same name as the object will be treated as an Constructor.
- **conf** is the configuration command and must have a trailing **options:object**.
- any other function with trailing **options:object** supports per-command options.

### Parameters

- Parameters are unloaded into function-local vars based on their type (singular).
- [Type Unions](Functions.md#unions) are currently unsupported and simply ignored.
- A difference from JS signatures is [implicit types](Functions.md#implicit-types) are not supported.



## Returns

To return a function values, the `RETURN()` statement is used on a line by itself.
The function return type determines what actually gets returned.  As with parameters, this must not be a
"Type Union".


## Enums

*Enums* for use by structs are defined as follows:

``` js
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
```

## Structs

Structs are a collection of fields.

### Fields
The type of a struct field is one of:

- a **base type**
- a **struct**
- an array of **base type** or **struct**
- an **enum**
- a *bitfield* of int type or **enum**
- a *bitset* of an **enum** (indicated by name ending in **@**)

See [Field Specification](Misc.md#field-specification).


### Base Types
The base field types are:

``` js
bool int8_t int16_t int32_t int64_t uint8_t uint16_t
uint32_t uint64_t float double ldouble Jsi_Strbuf time_w time_d time_t size_t
intptr_t uintptr_t Jsi_Number int uint long ulong short ushort Jsi_Value*
Jsi_DString Jsi_Value* Jsi_Value* Jsi_Value* Jsi_Value* Jsi_Value* Jsi_Value* Jsi_Value*
Jsi_Strkey const char
```

Plus *char arrays* and **STRING**n for powers-of-2 up to 64k.


## Signatures

Signatures, which are important when sending raw struct data over the network,
are enabled if the first field of a struct is named **sig**.

If unassigned, the *sig* value is
a *crc32* calculated against the struct definition types (ie. after being stripping of names and comments).


