#!/usr/bin/env jsish
// Demo for Jsi sockets.

var tst = {
    done:false,
    cnt:0,
    s:null,
    exitEv:null
};

var opts = {
    server:false,
    udp:false,
    debug:0
};

for (var i=0; i<console.args.length; i++) {
    switch (console.args[i]) {
        case '-server': opts.server = true; break;
        case '-udp':    opts.udp = true; break;
        case '-debug':  opts.debug = 1; break;
        default:
            throw(i + " argument not one of: -"+opts.keys().join(' -'));
    }
}


function ClientSend() {
    puts("SEND: "+tst.cnt);
    s.send('Hello from client: '+tst.cnt);
    if (tst.cnt++ > 10) {
        puts("DONE");
        tst.done = true;
        exit(0);
    }
}

function onRecv(str, id) {
    puts("onRecv: "+str);
    puts("CONF: "+s.idconf().toString());
    if (opts.server && !opts.udp)
        s.send('Server recv #'+id+' '+tst.cnt++, id);
}

function onOpen(id) {
    puts("onOpen");
    if (tst.exitEv) {
        puts("CLEARING");
        clearInterval(tst.exitEv);
    }
    tst.exitEv = null;
}

function onClose(id) {
    puts("onClose: "+id);
    tst.exitEv = setTimeout(function () { puts("SET DONE"); tst.done=true;}, 10000);
}

opts.onOpen  = onOpen;
opts.onClose = onClose;
opts.onRecv  = onRecv;

s = new Socket(opts);
if (!opts.server) {
    setInterval(ClientSend, 1000);
}

while (!tst.done && Event.update(1000)) {
 // Process...
}
