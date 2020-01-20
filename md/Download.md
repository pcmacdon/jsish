Download
====
<div id="sectmenu"></div>
Following is an overview of getting, installing, building and using Jsi.

Download
----
**Note**: you can download a static binaries [here](https://jsish.org/jsi-bin).


### Source
To download the zipped source:

    wget http://jsish.org/jsi/zip/jsi -O jsi.zip 
    unzip jsi.zip && cd jsi && make

See [build](#Building) for more directions.


### Fossil

A better way uses [fossil](http://www.fossil-scm.org),
to simplify keeping up to date:

    mkdir jsi
    cd jsi
    fossil clone https://jsish.org/jsi jsi.fossil
    fossil open jsi.fossil

If you don't have fossil:

- On Debian try: *apt-get install fossil*
- Get a binary from [https://fossil-scm.org](https://www.fossil-scm.org/index.html/uv/download.html)


Building
----
Jsi requires a few packages to build, eg. on **Debian**:

    sudo apt-get install build-essential unzip bison fossil libmysqlclient-dev


### Linux
To build the Linux target there are are two steps:

    ./configure

Do not be surprised to see comiler output from [./configure](../tools/configure.js):
it compiles the stripped down shell "jsimin".

Next, run "make" to build the actual "jsish" executable.

    make

**Note**: during the build [sqlite](https://sqlite.org/download.html) and
[libwebsockets](https://libwebsockets.org/) will be downloaded.

The last (optional) step is to run the test suite:

    make test

If you want to see other available options try:

    ./configure --help

**Note**:
    The directory **Configs/**, which contains a number of predefined configurations
    which can be copied to **make.conf**


### Debian
If you are on a debian system, you can build then install as a package:

    cd tools
    ./makedep.sh
    sudo dpkg -i jsish-*


### FreeBSD
On FreeBSD you will need to use **gmake** instead of **make**:

    pkg install fetch gmake bison


### Windows
Jsi can be cross compiled from Linux to Windows using the Mingw32 package:

    sudo apt-get install gcc-mingw-w64

Then configure using:

    ./configure --config=win

**Warning**: Features such as signals are disabled in the Windows build.
As you would expect, there are also obvious differences in the file-system.


### Standalone
The **standalone** build produces a static binary that contains no external library references.
This is useful when you need a standalone executable with no external dependancies.

To create a static image download/unpack [Musl](https://www.musl-libc.org) then do:

     ./configure --prefix=$HOME/usr
     make install
     export PATH=$PATH:$HOME/usr/bin

Next, a few files need to be fixed up:

    echo '#define __P(x) x' > ~/usr/include/sys/cdefs.h
    echo '#include <miniz/zlib.h>' >  ~/usr/include/zlib.h
    cp -pr miniz ~/usr/include/

Finally **cd** back to the **jsi** dir and:

    ./configure --config=musl
    make


### Embedding
Amalgamated source with [jsi.c](../src/jsi.c) is the easiest way to incorporate Jsi into an existing application, eg:

    #include "jsi.c"
    
    int main(int argc, char *argv[])
    {
        Jsi_Interp *interp = Jsi_InterpNew(NULL);
        Jsi_EvalString(interp, "for (var i=1; i<=3; i++)  puts('TEST:',i);", 0);
        if (argc>1)
            Jsi_EvalFile(interp, Jsi_ValueNewStringDup(interp, argv[1]), 0);
    }

Then compile with:

    gcc  myfile.c -o myfile -lm -lz -ldl -lpthread
    

More extensive examples are in [c-demos](/file/c-demos).

In particular [minimal.c](/file/c-demos/minimal.c), used to create **minimalsh** that
handles Jsi arguments then returns control to application:

    $ make -C c-demos minimalsh
    $ c-demos/minimalsh -v

==>

    2.8.12 2.081 6b18cee9a7a458d892df9f9b05b7558e23539948 2019-03-31 20:50:48 UTC


**Note**: Jsi is written in C, and 
can be compiled as either native C, or native C++: it does not use **extern C**.

Using
----
The following describes a few of the ways to use jsish.

The Jsish package comprises the scripts zipped to the end of the jsish
executable that implement command-line option/utilities such as the debugger
and parseOpts.


### Interactive
Interactive mode is the easiest way to try out code snippets, eg:

      ./jsish

==>

    Jsish interactive: see 'help [cmd]'.  \ cancels > input.  ctrl-c aborts running script.
    # var a = [1,2,3];
    # for (var i in a) { puts(a[i]); }
    1
    2
    3
    # help require
    require(name:string=void, version:number|string=1):number|array|object
    Load/query packages.
    With no arguments, returns the list of all loaded packages.
    With one argument, loads the package (if necessary) and returns its version.
    With two arguments, returns object containing: version, loadFile, func.
    An error is thrown if requested version is greater than actual version.
    #
    # for (i=
    > \
    abandoned input# 
    # 9+12;
    21
    # ^C

### Script
The script file to be executed is the first argument:

    jsish prog.js arg1 arg2

Under unix, the first line of executable scripts can be #!:

    #!/usr/bin/env jsish
    for (var i in console.args)
       puts(console.args[i]);

The above allows jsish to be found in the path.

### Inline
Javascript can also be evaluated from the command-line with -e, eg:

    jsish -e 'var i = 0; while (i++<10) puts(i);'

To see the final value, add a **return**:

    jsish -e 'var i = 0; i++; return i;'

Help
----
To see the supported switches in jsish use -h

    jsish -h
    USAGE:
      jsish [PREFIX-OPTS] [COMMAND-OPTS|FILE] ...
    
    PREFIX-OPTS:
      --C FILE	    Option file of config options.
      --F           Trace all function calls/returns.
      --I OPT:VAL   Interp option: equivalent to Interp.conf({OPT:VAL}).
      --L PATH	    Set safeMode to "lockdown" using PATH for safe(Read/Write)Dirs.
      --T OPT       Typecheck option: equivalent to "use OPT".
      --U           Display unittest output, minus pass/fail compare.
      --V           Same as --U, but adds file and line number to output.
    
    COMMAND-OPTS:
      -a            Archive: mount an archive (zip, sqlar or fossil repo) and run module.
      -c            CData: generate .c or JSON output from a .jsc description.
      -d            Debug: console script debugger.
      -e STRING     Eval: run javascript in STRING and exit.
      -g            Gendeep: generate html output from markdeep source.
      -h            Help: show this help.
      -m MOD        Module: invoke runModule, after source if file.
      -s            Safe: runs script in safe sub-interp.
      -u            UnitTest: test script file(s) or directories .js/.jsi files.
      -w            Wget: web client to download file from url.
      -v            Version: show version info.
      -z            Zip: append/manage zip files at end of executable.
      -D            DebugUI: web-gui script debugger.
      -S            SqliteUI: web-gui for sqlite database file.
      -W            Websrv: web server to serve out content.
    
You can also get help for commands from the command-line:

    jsish -help require

==>

    require(name:string=void, version:number|string=1):number|array|object
    Load/query packages.
    With no arguments, returns the list of all loaded packages.
    With one argument, loads the package (if necessary) and returns its version.
    With two arguments, returns object containing: version, loadFile, func.
    An error is thrown if requested version is greater than actual version.



### Modules
Help for module commands can similarly be displayed, eg:

    jsish -d -h
    /zvfs/lib/Debug.jsi:34: help: ...
    A command-line Debugger for Jsi scripts..  Options are:
        -echoCmd    true        // Echo user cmds.
        -safe       false       // Debug program in a safe interp (untested)
    
    Accepted by all .jsi modules: -Debug, -Trace, -Test

and use as in:

    jsish -d -echoCmd true tests/while.js



Apps
----

Jsi is distributed with several demonstration web applications:

- [DebugUI](app/debugui): a Debugger user interface for Jsi scripts.
- [SqliteUI](app/sqliteui): a web user interface to Sqlite.
- [LedgerJS](app/ledgerjs): an accounting program.

These can all be run as [standalone](Builtins.md#zvfs) applications.

Shell
----
You can use jsish
as an enhanced replacement for [#!/usr/bin/env](https://en.wikipedia.org/wiki/Shebang_(Unix)).
This lets you run scripts from the command line with default arguments:

    #!/usr/local/bin/jsish --T Debug %s -Trace true myinput1.txt
    puts(console.args.join(' '));

(there must be a %s and at least one argument)

From geany you can now run the script with F9, and step through
warnings and errors.

This also works for [logging](Logging.md) messages: [mytest2](../js-demos/log/mytest2.jsi)


Editors
----


### Geany
[Geany](http://www.geany.org) is a convenient editor to use with Jsi.
To enable Jsi file completion with Geany:

- Copy tools/geany/filetypes.javascript to ~/.config/geany/filedefs/.
- Open geany and navigate to Tools->Configuration Files->filetypes_extensions.conf, then:
- Add ".jsi;.jsc" to Javascript, ".md.html;.htmli;" to HTML and "*.cssi;" to CSS
- Keep the file tools/protos.jsi open in the editor so Geany knows how to complete Jsi functions.

Geany can also navigate
through Jsi's gcc style scripting errors:

- Start by editing a .jsi file.
- From the Geany Build menu, select Set Build Commands.
- Click on the first blank label in Javascript and enter Jsish.
- In the command section enter the pathspec to jsish, eg. $HOME/bin/jsish %f
- Click Ok

Now hit F8 to run the script. Upon errors, you should be able to navigate to the
highlighted lines, and see warnings in the bottom pane.

Alternatively, you can just create a Makefile to run jsish.

Also, if using the [logging](Logging.md) feature in Jsi, you can also navigate through
debug messages when using [F9 or shell exec](#Shell).

To run scripts that honor the [shebang](#Shell), repeat the above but set the command section
to jsish -# %f.

Caveat: one limitation of Geany is
that a function with a return type will likely not show up in the symbols list.


### Vim
Here is how to setup/use vim with Jsi:

    :set filetype javascript
    :set makeprg=jsish\ %
    :copen

Then to run scripts just use:

    :make

And much of what was said about navigation in Geany also applies to Vim.

To enable syntax highlighting, run **"sudo vi $(locate filetype.vim)"**,
search for **javascript**, and add **,*.jsi**. 
