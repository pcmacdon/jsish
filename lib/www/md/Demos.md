Demos
=====
[Index](Index.md "Jsi Documentation Index") /  [Reference](Reference.md "Generated Command Reference")

## Online

Jsish can be seen in action in these online demos:

- [Ledger](https://jsish.org/App10/Ledger): a personal accounting app.
- [Sandbox](https://jsish.org/App11/Sandbox): for testing jsi scripts in a web browser.

**Note**:
    Tested on Chrome, Firefox and Safari.

## Builtins

If you have downloaded Jsish, it comes with builtin applications and demos.
For example, to use the Jsi debugger with its Web-UI interface use:

``` bash
jsish -D tests/while2.jsi
```

+++ Screenshot:

![debug](https://jsish.org/images/jsish_debugui.png)

+++

Or try the Sqlite-UI with:

``` bash
jsish -S nw2.db
```

+++ Screenshot:

![sqlite](https://jsish.org/images/jsish_sqliteui.png)

+++

Here are some additional examples:

``` bash
jsish -w http://jsish.org/index.html; # Download file
jsish -W index.html; # Serve out an html file.
```

Note, the implementation scripts come from /zvfs,
so no access to the local filesystem is required.

## Jsi-App

The online apps (above) are from [jsi-app](https://jsish.org/jsi-app),
collection of applications all of which can be run from fossil, eg:

``` bash
fossil clone https://jsish.org/jsi-app jsi-app.fossil
jsish -a jsi-app.fossil Ledger
```

See [Deploy](Deploy.md)


## Documentation

- https://docs.jsish.org
- https://jsish.org/jsi/www/docs
- https://jsish.org/repo/jsi/lib/www/docs/index.html#/Start.md
- https://jsish.org/fossil/jsi/doc/tip/lib/www/md/Start.md
- https://jsish.org/fossil/jsi/doc/tip/lib/www/docs
- https://github.com/pcmacdon/jsish/blob/master/lib/www/md/Start.md

### App

- jsish -W -docs /
- jsish -W -docs /zvfs/lib/www/md
- jsish -W -app /zvfs/lib/www/docs
- jsish -W -zip jsi.zip -docs lib/www/docs
- jsish -W -zip jsi.fossil -docs lib/www/docs
- jsish -W -zip jsi.fossil -app app-demos/simple



