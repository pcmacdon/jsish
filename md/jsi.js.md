jsi.js
====

<div id="sectmenu"></div>

[jsi.js](https://jsish.org/jsi/file/lib/web/jsi.js) provides provdes global commands: **$**, **$jsig**, **puts**.

Typed Functions
====
Most important is support for [type-checked](Types.md) function arguments
via preprocessing:

    // bark.jsi
    function bark(s:string, n:number) {
       console.log("BARK: "+s);
    }

This is loaded via **include()**:

    <script src="/jsi/web/jsi.js"></script>
    <script>
        $jsi.include('bark.jsi');
        $jsi.onload(function() {
            bark('abc',0);
            bark(9,0);
            bark('9','0');
            bark(9);
            bark(9,0,0);
        });
        
    </script>

Mismatches are output to the console:

    BARK: abc
    jsi.js:23 type mismatch for arg 1 "s" expected "string" got "number" 9: calling bark(s:string, n:number)
    BARK: 9
    jsi.js:23 type mismatch for arg 2 "n" expected "number" got "string" 0: calling bark(s:string, n:number)
    BARK: 9
    jsi.js:23 missing arguments: expected 2 got 1: calling bark(s:string, n:number)
    BARK: 9
    jsi.js:23 extra arguments: expected 2 got 3: calling bark(s:string, n:number)
    BARK: 9


Typically we use the **jsish** web server to preprocess **.jsi** files:

    <script src="/jsi/web/jsi.js"></script>
    <script src="bark.js"></script>

The preprocessor outputs:

    <script src="/jsi/web/jsi.js"></script>
    <script>
    function bark(s, n) { $jsig('bark(s:string, n:number)', arguments);
       console.log("BARK: "+s+n);
    }
    
    bark('husky',2);
    </script>


$jsi methods
====
The **$jsi** object contains the following commands:

$(selector:string, top:string=void)
----
Jquery-like querySelectorAll functionality.

$jsig(sig:string, args:array)
----
While type-checking can be added manually,
Jsi provides a preprocessor (*Jsipp*) for converting "typed" functions into web-browser compatible form, eg.

    function foo(s:string='', n:number=1) {

==>

    function foo(s, n) { $jsig( "s:string,n:number", arguments)();

The returned value is a function that gets invoked: Usually 
the empty-function if no error, else either *console.warn* or *console.assert*.
This provides clickable messages in the console.
  
Limitation: Comments within function signatures are not supported,
and either are return-types.

ajax(options:object)
----

Compatiable with a subset of JQuery **ajax**.  Supported opts are:

Option    | Default | Description
----------|---------|-----
success   | none    | Called on success
error     | none    | Called on error
complete  | none    | Called when complete
type      | 'GET'   | |
dataType  | 'text'  | One of json, jsonp, script, text.
data      | {}      | Query data.
headers   | {}      | Headers to pass
async     | true    | Is asyncronous
url       | null    | Url of resources


conf(vals:object)
----
Configure jsi options.

filesave(filename:string, data:string, mime='text/html')
----
Save data as filename in browser.

guid()
----
Return unique UUID.

include(file:string\|array)
----
Dyanmic file include, uses ajax for .jsi files when jsish is not the webserver.

onload(f:function)
----
Function to invoke after page is loaded and all **include** processing is complete.

output(str:string)
----
Insert html into page.

    $jsi.output('<b>Hello World</b>');

puts(str:string,...)
----
A bind alias for console.log.

schema(obj:object, schm:object=null)
----
Check object/json schema, or generate when schm null/ommited.

setopts(obj:object, opts:object)
----
Set opts in obj and return.

websock(prot='ws')
----
Open new websocket.  Primarily used to handle auto-reloading pages on modify such
as when using **jsish -W index.html**, or for any websock app using:

    <script src="/jsi/web/jsi.js?websock=true"></script>

