Jsi is a **C**-extensible javascript interpreter with integrated Web-server and Database.

### Links

 - Documentation: [Start](./lib/web/md/Start.md) | [Index](./lib/web/md/Index.md).
 - Jsi repository: [https://jsish.org](https://jsish.org). 
 - Github for issues: [https://github.com/pcmacdon/jsish](https://github.com/pcmacdon/jsish).

### Quick-Start

Either download binary or [build it](Builds.md):

    wget http://jsish.org/bin/linux -O jsish  && chmod u+x jsish
    
    wget http://jsish.org/zip -O jsi.zip  && unzip jsi.zip && cd jsi && make

    
### Use

    ./jsish -W -docs /          # Jsi web-docs.
    ./jsish -S mysqlite.db      # Sqlite web-gui.
    
Compile and run a simple C-extension.

    ./jsish -c -jsc "function add(n1:number, n2:number=1):number { n1+=n2; \nRETURN(n1);\n }" Sum 
    ./jsish -e 'require("Sum",0); return Sum.add(9,3);'

