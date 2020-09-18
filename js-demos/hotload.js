document.title = 'Hotload';
(function () {
    var ws = new WebSocket(document.URL.replace(/^http/,'ws'), 'ws');
    ws.onmessage = function(msg) {
        var s = JSON.parse(msg.data);
        if (s.mod == '!' && s.cmd == 'reload')
           location.reload();
    };
})();

