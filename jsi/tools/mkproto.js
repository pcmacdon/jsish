#!/usr/bin/env jsish
// Generate function prototypes for Jsi builtin cmds.

function jsi_mkproto() {

    function DumpArgs(ci) {
        var argStr = ci.argStr;
        if (argStr === undefined)
            argStr = '';
        else {
            argStr = argStr.replace(/\?/g,'');
            argStr = argStr.replace(/ \| /g,'_');
            argStr = argStr.replace(/\|/g,'_');
        }
        return argStr;
    }

    function DumpCmd(cinf) {
        return "var "+cinf.name+" = function("+DumpArgs(tinf)+") {};\n";
    }

    function DumpObj(tinf) {
        var hasconf, ro = '', rv = '', ci, cnam = tinf.name, cmds = Info.cmds(cnam+'.*');
        rv += "var "+tinf.name+" = function(cmd,args) {};\n";
        if (cmds !== undefined) {
            for (var cmd in cmds) {
                var nam = cmds[cmd].split('.')[1];
                if (nam == cnam)
                    continue;
                ci = Info.cmds(cnam+'.'+nam);
                rv += tinf.name+".prototype."+nam+" = function("+DumpArgs(ci)+") {};\n";
            }
        }
        return rv;
    }
    
    //*************** BEGIN MAIN **************
    var rv = '', tinf, lst = Info.cmds();
    var vv = Info.version(true);
    var ver = vv.major+'.'+vv.minor+'.'+vv.release;

    for (var i in lst) {
        tinf = Info.cmds(lst[i]);
        switch (tinf.type) {
        case 'object':
            rv += DumpObj(tinf);
            break;
        case 'command':
            rv += DumpCmd(tinf);
            break;
        default:
            throw("bad id");
            continue;
        }
    }
    return '//JSI Command Prototypes: version '+ver+'\nthrow("DO NOT EXECUTE: LEAVING THIS FILE OPEN IN GEANY IS USEFUL FOR CMD LINE COMPLETION + GOTO TAG");\n\n' + rv;
}

if (Info.isMain()) {
    puts(jsi_mkproto());
}
