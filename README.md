<div id="sectmenu"></div>

**Jsi** is a small, C-embeddable javascript interpreter with tightly woven Web and DB support.

    wget http://jsish.org/jsi/zip/jsi -O jsi.zip 
    unzip jsi.zip && cd jsi && make

    jsish -u tests
    jsish -W js-demos/wspage.html   # Websockets
    jsish -S mysqlite.db            # Web GUI for Sqlite.
    
    jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    jsish -e 'require("Sum".0); return Sum.add(9,3);'   # Simple C-extension.

### Docs

| NAME                      | DESCRIPTION                    | NAME                      | DESCRIPTION                    |
|---------------------------|--------------------------------|---------------------------|--------------------------------|
| [Builtins](./md/Builtins.md)   | The built-in commands          | [Reference](./md/Reference.md) | Built-in commands (generated)  |
| [C-API](./md/C-API.md)         | The C API                      | [Interp](./md/Interp.md)       | The Interp API                 |
| [CData](./md/CData.md)         | Using C structs in Jsi         | [Logging](./md/Logging.md)     | Logging support                |
| [Coding](./md/Coding.md)       | Developing with Jsi            | [Misc](./md/Misc.md)           | Various topics                 |
| [DBQuery](./md/DBQuery.md)     | Sqlite database queries from C | [MySql](./md/MySql.md)         | MySql database API             |
| [Debug](./md/Debug.md)         | Debugging in Jsi               | [Web](./md/Web.md)             | Web and WebSocket support      |
| [Demos](./md/Demos.md)         | Demo applications              | [Sqlite](./md/Sqlite.md)       | Sqlite database API            |
| [Deploy](./md/Deploy.md)       | Deploying Jsi apps             | [Testing](./md/Testing.md)     | Testing facility for scripts   |
| [Download](./md/Download.md)   | Download, build and use        | [Types](./md/Types.md)         | Function parameter types       |

### Links  

 - [https://jsish.org](https://jsish.org):  The main fossil repo and docs. 
 - [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish): The github mirror.
