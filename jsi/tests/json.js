/*
=!EXPECTSTART!=
UMASK: 0422
WDIR: /tmp/ready
TOSTR: { count:422, done:null, gonogo:true, umask:"0422", "watch-dir":"/tmp/ready" }
JSON: { "count":422, "done":null, "gonogo":true, "umask":"0422", "watch-dir":"/tmp/ready" }
{ "count":422, "done":null, "gonogo":true, "umask":"0422", "watch-dir":"/tmp/ready" }
=!EXPECTEND!=
*/

var d = '
{
    "count": 422, 
    "umask": "0422", 
    "watch-dir": "/tmp/ready",
    "gonogo" : true,
    "done" : null
}
';
var j = JSON.parse(d);
j.toString();
puts('UMASK: ' + j.umask);
puts('WDIR: ' + j["watch-dir"]);
puts('TOSTR: ' + j.toString());
puts('JSON: ' + JSON.stringify(j));

var x = JSON.stringify(j);
puts(x);

