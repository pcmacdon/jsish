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
| [Builtins](https://jsish.org/doc/Builtins.md)   | The built-in commands          | [Reference](https://jsish.org/doc/Reference.md) | Built-in commands (generated)  |
| [C-API](https://jsish.org/doc/C-API.md)         | The C API                      | [Interp](https://jsish.org/doc/Interp.md)       | The Interp API                 |
| [CData](https://jsish.org/doc/CData.md)         | Using C structs in Jsi         | [Logging](https://jsish.org/doc/Logging.md)     | Logging support                |
| [Coding](https://jsish.org/doc/Coding.md)       | Developing with Jsi            | [Misc](https://jsish.org/doc/Misc.md)           | Various topics                 |
| [DBQuery](https://jsish.org/doc/DBQuery.md)     | Sqlite database queries from C | [MySql](https://jsish.org/doc/MySql.md)         | MySql database API             |
| [Debug](https://jsish.org/doc/Debug.md)         | Debugging in Jsi               | [Web](https://jsish.org/doc/Web.md)             | Web and WebSocket support      |
| [Demos](https://jsish.org/doc/Demos.md)         | Demo applications              | [Sqlite](https://jsish.org/doc/Sqlite.md)       | Sqlite database API            |
| [Deploy](https://jsish.org/doc/Deploy.md)       | Deploying Jsi apps             | [Testing](https://jsish.org/doc/Testing.md)     | Testing facility for scripts   |
| [Download](https://jsish.org/doc/Download.md)   | Downloading and building       | [Types](https://jsish.org/doc/Types.md)         | Function parameter types       |

### Notes

 - The main fossil-repository and docs are here: [https://jsish.org//](https://jsish.org/)
 - A github mirror is here: [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish)
 
