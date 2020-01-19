Web
========
<div id="sectmenu"></div>

Jsi provides server-side web services built upon the [WebSockets](Builtins.md#WebSocket) extension.

Preprocessors
----
Pre-processing is determined by file extension:


Extension | Type       | Description
----------|------------|---------------------------------------------------------------------
.htmli    | HTML       | Evaluate javascript between tags **&lt;?** and **?&gt;**.
.cssi     | CSS        | Preprocess **$**define style symbols.
.jsi      | Javascript | Translate [typed-functions](./functions.wiki) (for use within browsers)

These work as described below.

### HTML Preprocessing
The HTML preprocessor is triggered by the extension <b>.htmli</b>.

It evaluates javascript contained in <b>&lt;?</b> and <b>?&gt;</b> tags.

This code is run in a private subinterp to which two new commands have been added:

    function include(file:array|string, inline=false); // Include and evaluate files.
    function output(str:string, markdown=false); // Append string to output.

Here is an example:

    <!DOCTYPE html>
    <html>
    <?
        output('<title>My App</title>');
        var files = ['head.html','body.html', 'dialogs.html'];
        for (var i in files)
            include(files[i]);
    ?>
    </html>

All <b>include</b>-ed files are also similarly processed recursively, regardless of extension.
The only exception are <b>.js .jsi</b> and <b>.css</b>,
whose handling is described below.

Files with the .css/.cssi extensions are wrapped in <b>&lt;style></b> tags
and *.js/.jsi* are wrapped in **script** tags.

When the include **debug** flag is *true*, non-html files are not included inline but rather
via link/script tags.  This is used to simplify debugging in the browser, eg.

    <head>
        include(['main.jsi', 'main.cssi', 'common.cssi'], true);
    </head>


### CSS Preprocessing
The CSS preprocessor is triggered by the <b>.cssi</b> extension, and provides symbolic substitution via defines prefixed with <b>$</b>:

    $mycolor = { blue }    /* File: styles.cssi */
    $mysize = { 14pt }
    $mydefn = {
        color:$mycolor;
        font-size:$mysize;
    }
      
    #mybut1 { color:$mycolor; font-size:$mysize}
    .eclass { $mydefn; margin:1px }

used like so:

    <?
        include('styles.cssi');
        output('#mylist { color: $mycolor; }\n');
    ?>


Note  after **$define** expansion, <b>&lt;?</b> and <b>?&gt;</b> evaluation is also applied.

### JS Preprocessing

The type pre-processor is triggered by the **.jsi** extension.

It converts [Typed](Types.md) functions into standard web browser javascript, ie.

    function notifyUser(m:string, n:number=1) {
        alert('NOTICE: '+m+': '+n);
    }

is converted to:

    function notifyUser(m, n) { m=Jsi.ArgCheck(...); n=Jsi.ArgCheck(...);
        alert('NOTICE: '+m+': '+n);
    }


This provides runtime type-checking of function calls.

To debug, we set a breakpoint on warnings which are output
to the console.

The Jsi support functions are included from: <b>/jsi/lib/jsi.js</b>

Enabling Preprocessors
----

Pre-processors **handlers** are enabled in WebSocket via:

    var ws = new WebSocket({callback:ws_input, extHandlers:true});
    
or

    var ws = new WebSocket({callback:ws_input});
    ws.handler('.htmli', 'Htmlpp',  null);
    ws.handler('.jsi',   'Jspp', null);
    ws.handler('.cssi',  'Csspp',  null);

SSI
---
The following SSI directives are supported for **.shtml** files:

    <!--#include file="X"-->
    <!--#include virtual="X"-->
    <!--#if expr="${X}"-->
    <!--#elif expr="${X}"-->
    <!--#else-->
    <!--#endif-->

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

==>

    jsish -W 'index.shtml?NOPLUGIN=true'
    jsish -W 'index.shtml?NOPLUGIN=true&ALTPLUGIN=true'
    jsish -W -udata "{NOPLUGIN:true}" index.shtml

Utilities
----

The following resources are available to include from **/jsi/lib/web/**:

|[jsi.js](jsi.js.md)| Support code for type-checking.
|[vue.min.js](https://vuejs.org/)| Web framework library.
|[vue-router.min.js](https://router.vuejs.org/)| Router for Vue.
