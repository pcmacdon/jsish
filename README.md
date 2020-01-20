![Jsi](www/site/logojsi.png)
a small javascript interpreter with tightly woven Web and DB support, embeddable in C.

    wget http://jsish.org/jsi/zip/jsi -O jsi.zip 
    unzip jsi.zip && cd jsi && make

    jsish -u tests
    jsish -W js-demos/wspage.html   # Websockets
    jsish -S mysqlite.db            # Web GUI for Sqlite.
    
    jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    jsish -e 'require("Sum".0); return Sum.add(9,3);'   # Simple C-extension.


| NAME                           | DESCRIPTION                    | NAME                           | DESCRIPTION                    |
|--------------------------------|--------------------------------|--------------------------------|--------------------------------|
| [Build](./md/Build.md)         | Downloading, building and using| [Reference](./md/Reference.md) | Command reference (generated)  |
| [Builtins](./md/Builtins.md)   | The built-in commands          | [Interp](./md/Interp.md)       | The Interp API                 |
| [C-API](./md/C-API.md)         | The C API                      | [Logging](./md/Logging.md)     | Logging support                |
| [CData](./md/CData.md)         | Using C structs in Jsi         | [Misc](./md/Misc.md)           | Various topics                 |
| [Coding](./md/Coding.md)       | Developing with Jsi            | [MySql](./md/MySql.md)         | MySql database API             |
| [DBQuery](./md/DBQuery.md)     | Sqlite database queries from C | [Sqlite](./md/Sqlite.md)       | Sqlite database API            |
| [Debug](./md/Debug.md)         | Debugging in Jsi               | [Testing](./md/Testing.md)     | Testing facility for scripts   |
| [Demos](./md/Demos.md)         | Demo applications              | [Types](./md/Types.md)         | Function parameter types       |
| [Deploy](./md/Deploy.md)       | Deploying Jsi apps             | [Web](./md/Web.md)             | Web and WebSocket support      |

Links
----

 - [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish): Github repository and issues.
 - [https://jsish.org](https://jsish.org):  Fossil repository. 
