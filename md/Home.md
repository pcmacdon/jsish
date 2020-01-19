Home
=====

**Jsi** is a small, embeddable javascript interpreter.

    # Get & Build
    wget http://jsish.org/jsi3/zip/jsi -O jsi.zip && unzip jsi.zip && cd jsi && make

    jsish -u tests          # Run builtin tests.
    jsish -W index.html     # View html in browser.
    jsish -S mysqlite.db    # Sqlite web GUI.
    
    jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    jsish -e 'require("Sum".0); return Sum.add(9,3);'   # Simple extension.
    

**Index**

| NAME                      | DESCRIPTION                    | NAME                      | DESCRIPTION                    |
|---------------------------|--------------------------------|---------------------------|--------------------------------|
| [Builtins](Builtins.md)   | The built-in commands          | [Home](Home.md)           | Home page                      |
| [C-API](C-API.md)         | The C API                      | [Ledger](Ledger.md)       | Financial app developed in Jsi |
| [CData](CData.md)         | Using C structs in Jsi         | [Logging](Logging.md)     | Logging support                |
| [Coding](Coding.md)       | Developing with Jsi            | [Misc](Misc.md)           | Various topics                 |
| [DBQuery](DBQuery.md)     | Sqlite database queries from C | [MySql](MySql.md)         | MySql database API             |
| [Debug](Debug.md)         | Debugging in Jsi               | [Reference](Reference.md) | Built-in commands (generated)  |
| [Demos](Demos.md)         | Demo applications              | [Sqlite](Sqlite.md)       | Sqlite database API            |
| [Deploy](Deploy.md)       | Deploying Jsi apps             | [Testing](Testing.md)     | Testing facility for scripts   |
| [Download](Download.md)   | Downloading and building       | [Types](Types.md)         | Function parameter types       |
| [Interp](Interp.md)       | The Interp API                 |                           |                                |
