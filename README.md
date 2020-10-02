Jsi: a self-serving, **C**-extensible javascript interpreter with integrated Web-server and Database.

### Links

 - Documentation: [Using](./md/Using.md) | [Index](./md/Index.md).
 - Jsi repository: [https://jsish.org](https://jsish.org). 
 - Github for issues: [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish).

### Quick-Start

    wget http://jsish.org/jsi/zip/jsi -O jsi.zip  && unzip jsi.zip && cd jsi && make
    
Once built, try it out:

    ./jsish -W js-demos/wspage.html   # Serve minimal websocket page.
    ./jsish -S mysqlite.db            # Web GUI for Sqlite.
    ./jsish -t tests                  # Run tests

    # A simple C-extension.
    ./jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    ./jsish -e 'require("Sum",0); return Sum.add(9,3);'

