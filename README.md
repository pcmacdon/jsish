**Jsi** is a small, C-embeddable javascript interpreter with tightly woven Web support.

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
| [Builtins](Builtins.md)   | The built-in commands          | [Reference](Reference.md) | Built-in commands (generated)  |
| [C-API](C-API.md)         | The C API                      | [Interp](Interp.md)       | The Interp API                 |
| [CData](CData.md)         | Using C structs in Jsi         | [Logging](Logging.md)     | Logging support                |
| [Coding](Coding.md)       | Developing with Jsi            | [Misc](Misc.md)           | Various topics                 |
| [DBQuery](DBQuery.md)     | Sqlite database queries from C | [MySql](MySql.md)         | MySql database API             |
| [Debug](Debug.md)         | Debugging in Jsi               | [Web](Web.md)             | Web and WebSocket support      |
| [Demos](Demos.md)         | Demo applications              | [Sqlite](Sqlite.md)       | Sqlite database API            |
| [Deploy](Deploy.md)       | Deploying Jsi apps             | [Testing](Testing.md)     | Testing facility for scripts   |
| [Download](Download.md)   | Downloading and building       | [Types](Types.md)         | Function parameter types       |

### Notes

 - The main fossil-repository and docs are here: [https://jsish.org//](https://jsish.org/)
 - A github mirror is here: [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish)
 
