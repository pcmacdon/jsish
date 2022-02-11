Jsi is a **javascript**-ish interpreter with builtin websocket-server, sqlite and **C**-extensibility. 


[Home](https://github.com/pcmacdon/jsish/ "Github Repository") | [Docs](https://github.com/pcmacdon/jsish/blob/master/lib/www/md/Index.md)

### Start

Get source and build '''make'''.

Or download a binary for (https://github.com/pcmacdon/jsibin/).

    
&#x1f6a9; See [Start](./lib/www/md/Start.md).
    
### Usage

    ./jsish -W -docs /          # Jsi web-docs.
    ./jsish -S mysqlite.db      # Sqlite web-gui.
    
Compile and run a simple C-extension.

    ./jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    ./jsish -e 'require("Sum",0); return Sum.add(9,3);'


