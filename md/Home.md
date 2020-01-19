Home
=====
<div id="sectmenu"></div>
<style>
.content .markdown table.mytbl td { border:0; padding:0; solid white; }
.content .markdown table.mytbl { width:auto; }

</style>

<table class="mytbl"><tr>

<td>
<!--
    *********************************
    * .---------.                   *
    * | Browser |                   *
    * '----+----'                   *
    *      ^         .------.       *
    *      |   .--- >|Database|      *
    *      v  |      '------'       *
    *  .------+.    .----+----.     *
    * |  Jsish  +   | Script  |     *
    * +---------+<--+  File   |     *
    * |  ZVFS   |   '---------'     *
    *  '-------'                    *
    *                               *
    ********************************* -->

<svg class="diagram" xmlns="http://www.w3.org/2000/svg" version="1.1" height="208" width="256" style="margin:0 auto 0 auto;"><g transform="translate(8,16 )">
<path d="M 8,0 L 8,32 " style="fill:none;"></path>
<path d="M 8,112 L 8,144 " style="fill:none;"></path>
<path d="M 48,40 L 48,88 " style="fill:none;"></path>
<path d="M 72,80 L 72,96 " style="fill:none;"></path>
<path d="M 88,0 L 88,32 " style="fill:none;"></path>
<path d="M 88,112 L 88,144 " style="fill:none;"></path>
<path d="M 120,96 L 120,144 " style="fill:none;"></path>
<path d="M 200,96 L 200,144 " style="fill:none;"></path>
<path d="M 8,0 L 88,0 " style="fill:none;"></path>
<path d="M 8,32 L 88,32 " style="fill:none;"></path>
<path d="M 136,48 L 176,48 " style="fill:none;"></path>
<path d="M 88,64 L 112,64 " style="fill:none;"></path>
<path d="M 136,80 L 176,80 " style="fill:none;"></path>
<path d="M 24,96 L 72,96 " style="fill:none;"></path>
<path d="M 120,96 L 200,96 " style="fill:none;"></path>
<path d="M 8,128 L 120,128 " style="fill:none;"></path>
<path d="M 120,144 L 200,144 " style="fill:none;"></path>
<path d="M 24,160 L 72,160 " style="fill:none;"></path>
<path d="M 136,48 C 119.2,48 120,64 120,64 " style="fill:none;"></path>
<path d="M 176,48 C 192.8,48 192,64 192,64 " style="fill:none;"></path>
<path d="M 88,64 C 71.2,64 72,80 72,80 " style="fill:none;"></path>
<path d="M 136,80 C 119.2,80 120,64 120,64 " style="fill:none;"></path>
<path d="M 176,80 C 192.8,80 192,64 192,64 " style="fill:none;"></path>
<path d="M 24,96 C 7.199999999999999,96 8,112 8,112 " style="fill:none;"></path>
<path d="M 72,96 C 88.8,96 88,112 88,112 " style="fill:none;"></path>
<path d="M 24,160 C 7.199999999999999,160 8,144 8,144 " style="fill:none;"></path>
<path d="M 72,160 C 88.8,160 88,144 88,144 " style="fill:none;"></path>
<polygon points="120,64 108,58.4 108,69.6 " style="stroke:none" transform="rotate(0,112,64 )"></polygon>
<polygon points="104,128 92,122.4 92,133.6 " style="stroke:none" transform="rotate(180,96,128 )"></polygon>
<polygon points="56,88 44,82.4 44,93.6 " style="stroke:none" transform="rotate(90,48,88 )"></polygon>
<polygon points="56,40 44,34.4 44,45.6 " style="stroke:none" transform="rotate(270,48,40 )"></polygon>
<g transform="translate(0,0)"><text text-anchor="middle" x="24" y="20">B</text><text text-anchor="middle" x="32" y="20">r</text><text text-anchor="middle" x="40" y="20">o</text><text text-anchor="middle" x="48" y="20">w</text><text text-anchor="middle" x="56" y="20">s</text><text text-anchor="middle" x="64" y="20">e</text><text text-anchor="middle" x="72" y="20">r</text><text text-anchor="middle" x="128" y="68">D</text><text text-anchor="middle" x="136" y="68">a</text><text text-anchor="middle" x="144" y="68">t</text><text text-anchor="middle" x="152" y="68">a</text><text text-anchor="middle" x="160" y="68">b</text><text text-anchor="middle" x="168" y="68">a</text><text text-anchor="middle" x="176" y="68">s</text><text text-anchor="middle" x="184" y="68">e</text><text text-anchor="middle" x="32" y="116">J</text><text text-anchor="middle" x="40" y="116">s</text><text text-anchor="middle" x="48" y="116">i</text><text text-anchor="middle" x="56" y="116">s</text><text text-anchor="middle" x="64" y="116">h</text><text text-anchor="middle" x="136" y="116">S</text><text text-anchor="middle" x="144" y="116">c</text><text text-anchor="middle" x="152" y="116">r</text><text text-anchor="middle" x="160" y="116">i</text><text text-anchor="middle" x="168" y="116">p</text><text text-anchor="middle" x="176" y="116">t</text><text text-anchor="middle" x="144" y="132">F</text><text text-anchor="middle" x="152" y="132">i</text><text text-anchor="middle" x="160" y="132">l</text><text text-anchor="middle" x="168" y="132">e</text><text text-anchor="middle" x="32" y="148">Z</text><text text-anchor="middle" x="40" y="148">V</text><text text-anchor="middle" x="48" y="148">F</text><text text-anchor="middle" x="56" y="148">S</text></g></g></svg>

</td>

<td>
<a href="https://jsish.org">
<img src="../www/site/logojsi.png" style="vertical-align:middle" class="spinimg"></a>
<span style="border-bottom:1px solid"> Javascript interpreter + embedded Web-server</span>
<ul>
<li><a href="Types.md">Typed function-parameters</a>.</li>
<li><a href="Builtins.md#Zvfs">Zvfs integrated scripts</a> for extended functionality.</li>
<li><a href="Download.md#Embedding">Embed into <b>C</b> programs</a> with a single <b>#include</b>.</li>
<li>WebSockets, Sqlite and other common prerequisites built-in.</li>
</ul>

<b>New Users:</b> Click here for <a href="https://jsish.org/fossil/jsi/alerts">Alerts</a>

</td>
</tr></table>

Index
----

- **Start**: [Download](Download.md), [Building](Download.md#Building), [Using](Download.md#Using), [Embedding](Download.md#Embedding)
- **Docs**: [Builtins](Builtins.md), [Reference](Reference.md), [FAQ](../../../wiki/FAQ), [Index](Docs.md), [License](Misc.md#License), [Language](Misc.md#Language Comparisons), [ECMA](Misc.md#ECMA Compatibilty)
- **Development**: [Types](Types.md), [Strict Mode](Types.md#Strict Mode), [Type Checking](Types.md#Checking), [Debugging](Debug.md), [Errors](Testing.md#Errors), [Logging](Logging.md)
- **Core**: [System](Builtins.md#System), [Info](Builtins.md#Info), [Interp](Interp.md), [Format](Logging.md#format), [File-System](Builtins.md#File), [Events](Builtins.md#Event)
- **Integration**: [Modules](Coding.md#Modules), [Packages](Coding.md#Packages), [Auto-Load](Coding.md#Auto-Load)
- **Web**: [Server](Web.md), [Preprocessing](Web.md), [WebSocket](Builtins.md#WebSocket), [Markup](Reference.md#Util), [JSON](Builtins.md#JSON)
- **Miscellaneous**: [CData](CData.md), [Threads](Interp.md#Thread-Interps), [Signal](Builtins.md#Signal), [Sqlite](Sqlite.md), [MySQL](MySql.md), [Zvfs](Builtins.md#Zvfs), [Socket](Builtins.md#Socket), [WebSocket](Builtins.md#WebSocket)
- **Tools**: [Testing](Testing.md), [Tracing](Coding.md#Execution Trace), [Profiling](Coding.md#Code Profile), [Code-Coverage](Coding.md#Code Coverage)
- **C/C++**: [Jsi-Lite](C-API.md#Jsi-Lite), [C Extension](CData.md), [DString](C-API.md#DString), [CData](CData.md), [Options](C-API.md#Options), [Sqlite-C](DBQuery.md), [JSON-C](C-API.md#JSON)
- **Applications**: [Ledger](Ledger.md), [SqliteUI](Download.md#Apps), [Web Server](Coding.md#Server)

Purpose
----

Jsi seeks to fill the void in current development environments:

- Provide a feature-dense scripting environment based on Javascript.
- Has Database (Sqlite) and GUI capabilities (WebSocket) builtin.
- Is easily embedded in C and extended.


Packaging
---------
<table class="mytbl"><tr>

<td>
A <a href="Deploy.md">Deploy</a> is a zip/sqlar archive or fossil
repository containing one or more applications, which Jsi can mount and execute.

For example, this <a href="https://jsish.org/App10/Ledger">Ledger</a> demo is
served out as follows
using <a href="https://jsish.org/jsi-app">jsi-app</a> fossil
(via an Nginx reverse proxy).


But deploying any application inevitably entails dealing with version dependencies.
Jsi handles this in a novel way by mounting **tagged** releases from a fossil repository.
If an application restart is set to automatically update the repository,
it ensures the latest supported release always gets run.
New code may be committed by developers at any time,
but only tagged releases will be used.

</td>
<td>
<!--
        *************************************
        *  .-----------------------------.  *
        *  |     Server running Nginx    |  *
        *  | .---------.      .-----.    |  *
        *  | |  Jsish  |<----+ Fossil|   |  *
        *  | '----+----'      '--+--'    |  *
        *  |      ^              ^       |  *
        *  '------+--------------+-------'  *
        *         |              |          *
        *         v       :      |          *
        *    .---------.  :   .--+--.       *
        *    | Browser |  :  | Fossil|      *
        *    '----+----'  :   '--+--'       *
        *       User      :  Developer      *
        *                 :                 *
        ************************************* -->

<svg class="diagram" xmlns="http://www.w3.org/2000/svg" version="1.1" height="240" width="288" style="margin:0 auto 0 auto;"><g transform="translate(8,16 )">
<path d="M 16,0 L 16,96 " style="fill:none;"/>
<path d="M 32,32 L 32,64 " style="fill:none;"/>
<path d="M 32,144 L 32,176 " style="fill:none;"/>
<path d="M 72,72 L 72,136 " style="fill:none;"/>
<path d="M 112,32 L 112,64 " style="fill:none;"/>
<path d="M 112,144 L 112,176 " style="fill:none;"/>
<path d="M 192,72 L 192,144 " style="fill:none;"/>
<path d="M 256,0 L 256,96 " style="fill:none;"/>
<path d="M 16,0 L 256,0 " style="fill:none;"/>
<path d="M 32,32 L 112,32 " style="fill:none;"/>
<path d="M 176,32 L 208,32 " style="fill:none;"/>
<path d="M 120,48 L 160,48 " style="fill:none;"/>
<path d="M 32,64 L 112,64 " style="fill:none;"/>
<path d="M 176,64 L 208,64 " style="fill:none;"/>
<path d="M 16,96 L 256,96 " style="fill:none;"/>
<path d="M 32,144 L 112,144 " style="fill:none;"/>
<path d="M 176,144 L 208,144 " style="fill:none;"/>
<path d="M 32,176 L 112,176 " style="fill:none;"/>
<path d="M 176,176 L 208,176 " style="fill:none;"/>
<path d="M 176,32 C 159.2,32 160,48 160,48 " style="fill:none;"/>
<path d="M 208,32 C 224.8,32 224,48 224,48 " style="fill:none;"/>
<path d="M 176,64 C 159.2,64 160,48 160,48 " style="fill:none;"/>
<path d="M 208,64 C 224.8,64 224,48 224,48 " style="fill:none;"/>
<path d="M 176,144 C 159.2,144 160,160 160,160 " style="fill:none;"/>
<path d="M 208,144 C 224.8,144 224,160 224,160 " style="fill:none;"/>
<path d="M 176,176 C 159.2,176 160,160 160,160 " style="fill:none;"/>
<path d="M 208,176 C 224.8,176 224,160 224,160 " style="fill:none;"/>
<polygon points="200,72 188,66.4 188,77.6 "  style="stroke:none" transform="rotate(270,192,72 )"/>
<polygon points="128,48 116,42.4 116,53.6 "  style="stroke:none" transform="rotate(180,120,48 )"/>
<polygon points="80,136 68,130.4 68,141.6 "  style="stroke:none" transform="rotate(90,72,136 )"/>
<polygon points="80,72 68,66.4 68,77.6 "  style="stroke:none" transform="rotate(270,72,72 )"/>
<g transform="translate(0,0)"><text text-anchor="middle" x="64" y="20">S</text><text text-anchor="middle" x="72" y="20">e</text><text text-anchor="middle" x="80" y="20">r</text><text text-anchor="middle" x="88" y="20">v</text><text text-anchor="middle" x="96" y="20">e</text><text text-anchor="middle" x="104" y="20">r</text><text text-anchor="middle" x="120" y="20">r</text><text text-anchor="middle" x="128" y="20">u</text><text text-anchor="middle" x="136" y="20">n</text><text text-anchor="middle" x="144" y="20">n</text><text text-anchor="middle" x="152" y="20">i</text><text text-anchor="middle" x="160" y="20">n</text><text text-anchor="middle" x="168" y="20">g</text><text text-anchor="middle" x="184" y="20">N</text><text text-anchor="middle" x="192" y="20">g</text><text text-anchor="middle" x="200" y="20">i</text><text text-anchor="middle" x="208" y="20">n</text><text text-anchor="middle" x="216" y="20">x</text><text text-anchor="middle" x="56" y="52">J</text><text text-anchor="middle" x="64" y="52">s</text><text text-anchor="middle" x="72" y="52">i</text><text text-anchor="middle" x="80" y="52">s</text><text text-anchor="middle" x="88" y="52">h</text><text text-anchor="middle" x="176" y="52">F</text><text text-anchor="middle" x="184" y="52">o</text><text text-anchor="middle" x="192" y="52">s</text><text text-anchor="middle" x="200" y="52">s</text><text text-anchor="middle" x="208" y="52">i</text><text text-anchor="middle" x="216" y="52">l</text><text text-anchor="middle" x="136" y="132">:</text><text text-anchor="middle" x="136" y="148">:</text><text text-anchor="middle" x="48" y="164">B</text><text text-anchor="middle" x="56" y="164">r</text><text text-anchor="middle" x="64" y="164">o</text><text text-anchor="middle" x="72" y="164">w</text><text text-anchor="middle" x="80" y="164">s</text><text text-anchor="middle" x="88" y="164">e</text><text text-anchor="middle" x="96" y="164">r</text><text text-anchor="middle" x="136" y="164">:</text><text text-anchor="middle" x="176" y="164">F</text><text text-anchor="middle" x="184" y="164">o</text><text text-anchor="middle" x="192" y="164">s</text><text text-anchor="middle" x="200" y="164">s</text><text text-anchor="middle" x="208" y="164">i</text><text text-anchor="middle" x="216" y="164">l</text><text text-anchor="middle" x="136" y="180">:</text><text text-anchor="middle" x="56" y="196">U</text><text text-anchor="middle" x="64" y="196">s</text><text text-anchor="middle" x="72" y="196">e</text><text text-anchor="middle" x="80" y="196">r</text><text text-anchor="middle" x="136" y="196">:</text><text text-anchor="middle" x="160" y="196">D</text><text text-anchor="middle" x="168" y="196">e</text><text text-anchor="middle" x="176" y="196">v</text><text text-anchor="middle" x="184" y="196">e</text><text text-anchor="middle" x="192" y="196">l</text><text text-anchor="middle" x="200" y="196">o</text><text text-anchor="middle" x="208" y="196">p</text><text text-anchor="middle" x="216" y="196">e</text><text text-anchor="middle" x="224" y="196">r</text><text text-anchor="middle" x="136" y="212">:</text></g></g></svg>

</td>
</tr>
</table>

Security
----
As Jsi is self contained, running it standalone in a chroot-jail needs only
a few files from /dev and /etc.   For example, this [Ledger](https://jsish.org/App01/Ledger)
demo is run in an unprivileged chroot containing a single executable (*jsish*),
with non-data files made immutable with **chattr**.
And if this is not secure enough, Jsi offers [Safe-mode](Interp.md#safe-mode).

In addition, serving content from a zip or fossil repository adds
another abstraction layer, making it that much harder to corrupt or hijack content.

An advantage of the chroot approach as compared with something like containers, is
that far less disk space and system memory is required.
Jsi and fossil together total around 10Meg of disk. Using hard links you
could create hundreds of chrooted apps with little-to-no additional disk space.
But more important is the 
reduction in [Attack Surface](https://en.wikipedia.org/wiki/Attack_surface).
There is far less code involved and far less complexity.


C-API
----
Jsi is written in C. For embedded developers this provides a [C-API](C-API.md) that simplifies
connecting low level (**C**) with high level (**GUI**) code.
The use of C-Structs is intrinsically integrated into Jsi at all levels,
and is the main mechanism used for module state, option parsing and
C extensions.
This direct
interfacing with C-structs can potential be used to process very
large amounts of data with little overhead.
This includes the Sqlite interface which also supports mass data transfers to/from structs,
which is of particular importance for embedded applications.

The C coding aspect of Jsi however is purely optional. [Ledger](Ledger.md) and the
other demo applications neither use nor require it.



