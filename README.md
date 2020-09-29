Jsi is **C**-extensible javascript interpreter with self-contained Web-server and Database.

### Links

 - Documentation for [Using Jsi](./md/Using.md) and/or [Builds](./md/Builds.md).
 - [https://jsish.org](https://jsish.org):  Jsi site for fossil repository and docs. 
 - [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish): Github alt-repository and issues.

### Quick-Start

    wget http://jsish.org/jsi/zip/jsi -O jsi.zip  && unzip jsi.zip && cd jsi && make
    
Once built, try it out:

    ./jsish -W js-demos/wspage.html   # Serve minimal websocket page.
    ./jsish -S mysqlite.db            # Web GUI for Sqlite.
    ./jsish -t tests                  # Run tests

    # A simple C-extension.
    ./jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    ./jsish -e 'require("Sum",0); return Sum.add(9,3);'

