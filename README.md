
    wget http://jsish.org/jsi/zip/jsi -O jsi.zip 
    unzip jsi.zip && cd jsi && make

    jsish -u tests          # Run builtin tests.
    jsish -W index.html     # View html in browser.
    jsish -S mysqlite.db    # Sqlite web GUI.
    
    jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    jsish -e 'require("Sum".0); return Sum.add(9,3);'   # Simple extension.

**Jsi** is a small, C-embeddable javascript interpreter with tightly woven support for Web development.

Documentation and fossil repo is here: [https://jsish.org/](https://jsish.org/).


