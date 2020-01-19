Demos
=====
<div id="sectmenu"></div>
Online
----
Jsish can be seen in action in these online demos:

- [Ledger](https://jsish.org/App10/Ledger): a personal accounting app ([Docs](Ledger.md)).
- [Sandbox](https://jsish.org/App11/Sandbox): for testing jsi scripts in a web browser.

**Note**:
    Tested on Chrome, Firefox and Safari.

Builtins
----
If you have downloaded Jsish, it comes with builtin applications and demos.
For example, to use the Jsi debugger with its Web-UI interface use:

    jsish -D tests/while2.jsi

**Screenshot**:
<a href="https://jsish.org/images/jsish_debugui.png" target="_blank"><img height="70px" alt="jsish DebugUI" src="https://jsish.org/images/jsish_debugui.png"></a>


Or try the Sqlite-UI with:

    jsish -S nw2.db

**Screenshot**:
<a href="https://jsish.org/images/jsish_sqliteui.png" target="_blank"><img height="70px" alt="jsish SqliteUI" src="https://jsish.org/images/jsish_sqliteui.png"></a>

Here are some additional examples:

    jsish -w http://jsish.org/index.html; # Download file
    jsish -W index.html; # Serve out an html file.

Note, the implementation scripts come from /zvfs,
so no access to the local filesystem is required.

Jsi-App
----
The online apps (above) are from [jsi-app](https://jsish.org/jsi-app),
collection of applications all of which can be run from fossil, eg:

    fossil clone https://jsish.org/jsi-app jsi-app.fossil
    jsish -a jsi-app.fossil Ledger

See [Deploy](Deploy.md)




