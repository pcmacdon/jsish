// Script to dump markdeep export via websocket.
(function () {

function DeepDumpWs() {
    if (document.URL.indexOf("?export=save")<0) return;
    console.log('DeepDumpWs');
    var url = document.URL.replace(/^http/,"ws").split('#')[0];
    var ws = new WebSocket(url, "ws");
    ws.onopen = function() {
        var data = document.querySelectorAll("body pre");
        if (!data[0]) data = document.querySelectorAll("body code");
        if (!data[0]) return;
        data = data[0].innerText;
        console.log('DATA '+data);
        //clearInterval(DeepDumpWsTO);
        ws.send(JSON.stringify({cmd:"save", url:url, data:data}));
    };
    ws.onmessage = function(msg) {
        if (msg.data !== 'DONE!!!')
            document.location = msg.data+"?export=save";
        else
            document.body.innerHTML = '<style>body{background:#ddd;}</style>DOWNLOAD COMPLETE: PLEASE CLOSE THIS TAB';
    };
};

function DeepDocCmd() {
    document.title=location.pathname.match(/\/([-\w]+)[^\/]*$/)[1];
    if (location.hash !== '') setTimeout(function() { location.href=location.href; console.log(location.href);}, 10);
    DeepDumpWs();
}

if(window.attachEvent) {
    window.attachEvent('onload', DeepDocCmd);
} else {
    if(window.onload) {
        var curronload = window.onload;
        var newonload = function(evt) {
            curronload(evt);
            DeepDocCmd(evt);
        };
        window.onload = newonload;
    } else {
        window.onload = DeepDocCmd;
    }
}
})();
