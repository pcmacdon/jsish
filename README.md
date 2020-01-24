![Jsi](www/site/logojsi.png)
A small javascript interpreter integrating Database and Web in **C**.

    wget http://jsish.org/jsi/zip/jsi -O jsi.zip 
    unzip jsi.zip && cd jsi && make

    jsish -u tests
    jsish -W js-demos/wspage.html   # Websockets
    jsish -S mysqlite.db            # Web GUI for Sqlite.
    
    jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    jsish -e 'require("Sum".0); return Sum.add(9,3);'   # Simple C-extension.


> Quote this
> and this
>    -- by Me

| DOC                            | DESCRIPTION                    | DOC                            | DESCRIPTION                    |
|--------------------------------|--------------------------------|--------------------------------|--------------------------------|
| [Build](./md/Build.md)         | Downloading, building and using| [Reference](./md/Reference.md) | Command reference (generated)  |
| [Builtins](./md/Builtins.md)   | Command descriptions & examples| [Interp](./md/Interp.md)       | Interpreter API                |
| [Coding](./md/Coding.md)       | Developing with Jsi            | [Logging](./md/Logging.md)     | Logging support                |
| [CData](./md/CData.md)         | Generate C extensions & structs| [Misc](./md/Misc.md)           | Various topics                 |
| [C-API](./md/C-API.md)         | The non-script API             | [MySql](./md/MySql.md)         | MySql database API             |
| [DBQuery](./md/DBQuery.md)     | Sqlite database queries from C | [Sqlite](./md/Sqlite.md)       | Sqlite database API            |
| [Debug](./md/Debug.md)         | Debugging in Jsi               | [Testing](./md/Testing.md)     | Testing facility for scripts   |
| [Demos](./md/Demos.md)         | Demo applications              | [Types](./md/Types.md)         | Function parameter types       |
| [Deploy](./md/Deploy.md)       | Deploying Jsi apps             | [Web](./md/Web.md)             | Web and WebSocket support      |

Links
----

 - [https://jsish.org](https://jsish.org):  Fossil repository and enhanced docs. 
 - [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish): Github repository and issues.

