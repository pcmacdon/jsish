Jsi is a **javascript**-ish interpreter with builtin websocket-server, sqlite and **C**-extensibility. 


- Documentation: [Start](./lib/web/md/Start.md) | [Index](./lib/web/md/Index.md) | [Issues](https://github.com/pcmacdon/jsish/issues)
- Sites: [jsish.org](https://jsish.org) / [github.com](https://github.com/pcmacdon/jsish)

### Quick-Start

To [build](Building.md) **jsish**
get source by [clicking here](http://jsish.org/jsi/zip/jsi),
or use command-line:
 
    wget http://jsish.org/zip -O jsi.zip  && unzip jsi.zip && cd jsi && make

or download a binary for linux/win:

    wget http://jsish.org/bin/jsish   && chmod u+x jsish
    wget http://jsish.org/bin/jsish.exe

    
### Usage

    ./jsish -W -docs /          # Jsi web-docs.
    ./jsish -S mysqlite.db      # Sqlite web-gui.
    
Compile and run a simple C-extension.

    ./jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    ./jsish -e 'require("Sum",0); return Sum.add(9,3);'
