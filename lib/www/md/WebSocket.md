WebSocket
========
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")


Jsi provides server-side web services built upon the [WebSockets](Builtins.md#WebSocket) extension.

## Preprocessors

Pre-processing is determined by file extension:


Extension | Type       | Description
----------|------------|---------------------------------------------------------------------
.htmli    | HTML       | Evaluate javascript between tags **&lt;?** and **?&gt;**.
.cssi     | CSS        | Preprocess **$**define style symbols.
.jsi      | Javascript | Translate **typed-functions** (for use within browsers)

These work as described below.

### HTML Preprocessing
The HTML preprocessor is triggered by the extension **htmli**.

It evaluates javascript contained in `<?` and `?> ` tags.

This code is run in a private subinterp to which two new commands have been added:

``` js
function include(file:array|string, inline=false); // Include and evaluate files.
function output(str:string, markdown=false); // Append string to output.
```

Here is an example:

``` html
<!DOCTYPE html>
<html>
<?
    output('<title>My App</title>');
    var files = ['head.html','body.html', 'dialogs.html'];
    for (var i in files)
        include(files[i]);
?>
</html>
```

All **include**-ed files are also similarly processed recursively, regardless of extension.
The only exception are `.js`, `.jsi` and `.css`,
whose handling is described below.

Files with the .css/.cssi extensions are wrapped in `<style>` tags
and *.js/.jsi* are wrapped in **script** tags.

When the include **debug** flag is *true*, non-html files are not included inline but rather
via link/script tags.  This is used to simplify debugging in the browser, eg.

``` js
<head>
    include(['main.jsi', 'main.cssi', 'common.cssi'], true);
</head>
```

### CSS Preprocessing
The CSS preprocessor is triggered by the `.cssi` extension, and provides 
symbolic substitution via defines prefixed with **$**:

``` js
$mycolor = { blue }    /* File: styles.cssi */
$mysize = { 14pt }
$mydefn = {
    color:$mycolor;
    font-size:$mysize;
}
  
#mybut1 { color:$mycolor; font-size:$mysize}
.eclass { $mydefn; margin:1px }
```

used like so:

``` js
<?
    include('styles.cssi');
    output('#mylist { color: $mycolor; }\n');
?>
```

Note  after **$define** expansion, `<?` and `?>` evaluation is also applied.

### JS Preprocessing

The type pre-processor is triggered by the **.jsi** extension.

It converts [Typed](Functions.md) functions into standard web browser javascript, ie.

``` js
function notifyUser(m:string, n:number=1) {
    alert('NOTICE: '+m+': '+n);
}
```

is converted to:

``` js
function notifyUser(m, n) { m=Jsi.ArgCheck(...); n=Jsi.ArgCheck(...);
    alert('NOTICE: '+m+': '+n);
}
```

This provides runtime type-checking of function calls.

To debug, we set a breakpoint on warnings which are output
to the console.

The Jsi support functions are included from: **/jsi/lib/jsi.js**

## Enabling Preprocessors

Pre-processors **handlers** are enabled in WebSocket via:

``` js
var ws = new WebSocket({callback:ws_input, extHandlers:true});

// or

var ws = new WebSocket({callback:ws_input});
ws.handler('.htmli', 'Htmlpp',  null);
ws.handler('.jsi',   'Jspp', null);
ws.handler('.cssi',  'Csspp',  null);
```

## SSI

The following SSI directives are supported for **.shtml** files:

    <!--#include file="X"-->
    <!--#include virtual="X"-->
    <!--#if expr="${X}"-->
    <!--#elif expr="${X}"-->
    <!--#else-->
    <!--#endif-->
    <!--#echo "Session.id=${#}"-->

Wherein the **expr** is a simple var-lookup with prefix modifiers of the form:

|Example|Description|
|-------|---|
|${X}   | Lookup in udata first, then fallback to query. |
|${~X}  | Reverse order: look in query first, then udata. |
|${?X}  | Lookup in query only. |
|${:X}  | Lookup in udata only. |
|${!X}  | Logical not. |
|${@X}  | Failed lookups issue a warning. |
|${*X}  | Failed lookups kick an error. |
|${@?!X}| Multiple modifiers |

An example:

``` html
<!-- File: index.shtml -->
<!--#include file="header.html"-->
<!--#include file="body.shtml"-->

<!--#if expr="${!NOPLUGIN}"-->
<!--#include file="plugins.shtml"-->

<!--#elif expr="${ALTPLUGIN}"-->
<!--#include virtual="altplugins.shtml"-->
Alt Plugins

<!--#else-->
No Plugins
<!--#endif-->
```

```
jsish -W 'index.shtml?NOPLUGIN=true'
jsish -W 'index.shtml?NOPLUGIN=true&ALTPLUGIN=true'
jsish -W -udata "{NOPLUGIN:true}" index.shtml
```

## Utilities

The following resources are available to include from **/jsi/lib/www/**:

|[vue.min.js](https://vuejs.org/)| Web framework library.
|[vue-router.min.js](https://router.vuejs.org/)| Router for Vue.

## SSL

Example:

     jsish -W -port 4443 -local false -server true -sslDir /etc/letsencrypt/live/example.org/ .

## Demos

The WebSocket extension uses libwebsockets to implement
bidirectional socket communication with web-browsers.

When used in conjunction with [Sqlite](Sqlite.md) and [JSON](Builtins.md#json),
it is easy to implement browser based applications.


The following creates a minimal client and server using WebSockets.
First the server file ws.js:

``` js
function ws_input(data, id) {
    puts("ws_input: "* + id + *": " + data);
};

var ws = new WebSocket({callback:ws_input});
var msg = { str:"whos there?", cnt:0 };
while (true) {
    update(1);
    if ((msg.cnt++ % 10) == 0)
       ws.send(JSON.stringify(msg));
}
```

Next the client file: wsc.js:

``` js
function wsc_input(data) {
    puts("wsc_input: " + data);
};

var ws = new WebSocket({client:true, callback:wsc_input});
var msg = { str:"knock knock", cnt:0 };

while (true) {
    msg.cnt++;
    ws.send(JSON.stringify(msg));
    update(1);
}
```

Which we run with:

``` bash
jsish ws.js &
jsish wsc.js
```

There are several ways to use Web in Jsi, all of which ultimately use
the builtin WebSocket api


