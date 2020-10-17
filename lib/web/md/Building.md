Building
====
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

Get source by [clicking here](http://jsish.org/jsi/zip/jsi), from [jsish.org](https://jsish.org/fossil/jsi3/vinfo?name=tip), or use command-line:
 
```
wget http://jsish.org/jsi/zip/jsi -O jsi.zip 
unzip jsi.zip
cd jsi
```

Better still [fossil](https://www.fossil-scm.org/index.html/uv/download.html) clone the repository:

```
sudo apt install fossil
fossil clone https://jsish.org/jsi jsi.fossil
fossil open jsi.fossil
```


Before building on Debian Linux, you may need:

```
sudo apt-get install bison build-essential fossil unzip libmysqlclient-dev
```

To build the Linux target use:

```
make
```
During the build it will download:

- Sqlite: https://www.sqlite.org/2019/sqlite-amalgamation-3300100.zip
- WebSocket: http://jsish.org/fossil/lws/zip/lws?r=lws-2.0202
- SSL (optional): http://jsish.org/download/openssl-OpenSSL_1_1_1-stable.zip

## Configs

Predefined configurations (from **Configs/**) can be built using:
```
make remake CONF=static
```

Available values for `CONF` are: **default devel memdebug minimal musl muslssl noext release static win winssl**

### Windows
Jsi can be cross compiled from Linux to Windows using the Mingw32 package:

``` bash
sudo apt-get install gcc-mingw-w64
make remake CONF=win
```

**Warning**: Features such as signals are disabled in the Windows build.
As you would expect, there are also obvious differences in the file-system.


### Static
The [musl](https://www.musl-libc.org/) build produces a static linux binary
containing no external library references.
This is useful when you need an executable with no external dependancies.

``` bash
sudo apt-get install musl-tools
make remake CONF=musl
```


### FreeBSD
On FreeBSD you will need to use **gmake** instead of **make**:

``` bash
pkg install fetch gmake bison
gmake
```

### SSL

To build with SSL support use the provided configs **muslssl** / **winssl**, or:

``` bash
make remake WITH_SSL=1
```


### Debian Package
If you are on a Debian system, you can build then install as a package:

``` bash
cd tools
./makedep.sh
sudo dpkg -i jsish-*
```


### Embedding
Amalgamated source with [jsi.c](https://jsish.org/jsi/file/src/jsi.c) is 
the easiest way to incorporate Jsi into an existing application.

+++ Example

``` clike
    #include "jsi.c"
    
    int main(int argc, char *argv[])
    {
        Jsi_Interp *interp = Jsi_InterpNew(NULL);
        Jsi_EvalString(interp, "for (var i=1; i<=3; i++)  puts('TEST:',i);", 0);
        if (argc>1)
            Jsi_EvalFile(interp, Jsi_ValueNewStringDup(interp, argv[1]), 0);
    }
```

Then compile with **gcc  myfile.c -o myfile -lm -lz -ldl -lpthread**.


More extensive examples are in [c-demos](https://jsish.org/jsi/file/c-demos).

[minimal.c](https://jsish.org/jsi/file/c-demos/minimal.c), used to create **minimalsh** that
handles Jsi arguments then returns control to application.

+++

+++ Example 2

    # make -C c-demos minimalsh
    # c-demos/minimalsh -v
    2.8.12 2.081 6b18cee9a7a458d892df9f9b05b7558e23539948 2019-03-31 20:50:48 UTC

+++



## Use

The following describes a few of the ways to use jsish.

### Interactive
Interactive mode is the easiest way to try out code snippets, eg:

+++ `jsish`

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

+++

### Script
Scripts process args in the usual fashion: 

``` bash
cat > prog.js <<EOF
for (var i in console.args)
   puts(console.args[i]);
EOF
jsish prog.js arg1 arg2
arg1
arg2
```

+++ Inline eval is also supported.

Javascript is evaluated from the command-line with **-e**:

```
jsish -e 'var i = 0; while (i++<10) puts(i);'
```

To see the final value, add a **return**:

```
jsish -e 'var i = 0; i++; return i;'
```

+++

## Help

+++ Jsi option list: `jsish -h`

``` bash
USAGE:
  jsish [PREFIX-OPTS] [COMMAND-OPTS|FILE] ...

PREFIX-OPTS:
  --E CODE	Javascript evaluated before program starts
  --I OPT=VAL	Interp option bits: equivalent to Interp.conf({OPT:VAL}); VAL defaults to true.

COMMAND-OPTS:
  -a		Archive: mount an archive (zip, sqlar or fossil repo) and run module.
  -c		CData: generate .c or JSON output from a .jsc description.
  -d		Debug: console script debugger.
  -e CODE	Evaluate javascript and exit.
  -h ?CMD?	Help: show help for jsish or its commands.
  -m		Module: utility create/manage/invoke a Module.
  -s		Safe: runs script in safe sub-interp.
  -t		testMode: test script file(s) or directories .js/.jsi files.
  -w		Wget: web client to download file from url.
  -v		Version: show version detail: add an arg to show only X.Y.Z
  -z		Zip: append/manage zip files at end of executable.
  -D		DebugUI: web-gui script debugger.
  -J		JSpp: preprocess javascript for web.
  -S		SqliteUI: web-gui for sqlite database file.
  -W		Websrv: web server to serve out content.
```
+++


+++ Help on commands: `jsish -help require`


``` bash
require(name:string=void, version:number|string=1):number|array|object
Load/query packages.
With no arguments, returns the list of all loaded packages.
With one argument, loads the package (if necessary) and returns its version.
With two arguments, returns object containing: version, loadFile, func.
An error is thrown if requested version is greater than actual version.
```

+++

+++ Builtin modules: `jsish -d -h`

Scripts zipped to the end of the jsish
executable provide builtin modules.

``` bash
/zvfs/lib/Debug.jsi:34: help: ...
A command-line Debugger for Jsi scripts..  Options are:
    -echoCmd    true        // Echo user cmds.
    -safe       false       // Debug program in a safe interp (untested)

Accepted by all .jsi modules: -Debug, -Trace, -Test
```

+++

+++ User modules: `jsish module.jsi -h`

``` bash
/tmp/module.jsi:11: help: ...
Module description..  Options are:
	-rootdir	""		// Root directory.

Accepted by all .jsi modules: -Debug, -Trace, -Test.

```

+++


## Apps

Jsi is distributed with several demonstration web applications:

- **DebugUI**: a Debugger user interface for Jsi scripts.
- **SqliteUI**: a web user interface to Sqlite.
- **LedgerJS**: an accounting program.

These can all be run as [standalone](Builtins.md#zvfs) applications.

## Shell

You can use jsish
as an enhanced replacement for [#!/usr/bin/env](https://en.wikipedia.org/wiki/Shebang_(Unix)).
This lets you run scripts from the command line with default arguments:

``` bash
#!/usr/local/bin/jsish --I log=Debug %s -Trace true myinput1.txt
puts(console.args.join(' '));
```

(there must be a %s and at least one argument)

From geany you can now run the script with F9, and step through
warnings and errors.

This also works for [logging](Logging.md) messages: [mytest2](https://jsish.org/jsi/file/js-demos/log/mytest2.jsi)


## Editors


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
debug messages when using [F9 or shell exec](#shell).

To run scripts that honor the [shebang](#shell), repeat the above but set the command section
to jsish -# %f.

Caveat: one limitation of Geany is
that a function with a return type will likely not show up in the symbols list.


### Vim
Here is how to setup/use vim with Jsi:

``` bash
:set filetype javascript
:set makeprg=jsish\ %
:copen
:make
```


And much of what was said about navigation in Geany also applies to Vim.

To enable syntax highlighting, run **"sudo vi $(locate filetype.vim)"**,
search for **javascript**, and add **,*.jsi**. 
