Home
=====

    wget http://jsish.org/jsi/zip/jsi -O jsi.zip 
    unzip jsi.zip && cd jsi && make


    jsish -u tests          # Run builtin tests.
    jsish -W index.html     # View html in browser.
    jsish -S mysqlite.db    # Sqlite web GUI.
    
    jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    jsish -e 'require("Sum".0); return Sum.add(9,3);'   # Simple extension.

**Jsi** is a small, C-embeddable javascript interpreter with tightly woven support for Web development.


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

**NOTES:**  

 - The github mirror is here: [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish)
 - The repository for Jsi-2 was moved to: [https://jsish.org/jsi2/](https://jsish.org/jsi2/)
 
 
