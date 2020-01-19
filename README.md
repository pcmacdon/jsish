# Readme

## http://jsish.org

Jsi is a javascript interpreter with:

+ Tight C integration with extensive C-API.
+ Extensions for Filesystem, OS, WebSocket, Sqlite, MySql, etc.
+ Integrated debugging (command-line or optional GUI-web).
+ Web framework, with example applications.
+ Sub-interpreters and introspection (modelled after Tcl).
+ Easy embedding within C applications.

Jsi implements an extended Ecmascript with functions supporting types and default values:

> __function foo (a:number, b:string=''):number {}__

These functions can also be preprocessed to javascript, ie. for use in Web Browsers.


## Building Jsi

To build:

    ./configure
    make
