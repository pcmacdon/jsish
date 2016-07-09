#!/usr/bin/env jsish
// Generate documentation for Jsi builtin cmds in fossil wiki format.

function jsi_mkref() {

    function DumpOpts(opts, nam, isconf) {
        //if (cnam.indexOf('.')<0) cnam=cnam+'.conf';
        var rv = '';//, opts = Info.cmds(cnam).options;
        rv += '\n\n<a name="'+nam+'Options"></a>\n';
        rv += '<h4>Options for "'+nam+'"</h4>\n';
        rv += "\nThe following options are available for \""+nam+"\"\n\n";
        rv += "<table border='1' class=optstbl>\n";
        rv += "<tr><th>Option</th> <th>Type</th> <th>Description</th> <th>Default</th></tr>\n";
        //puts(opts);
        for (var o in opts) {
            ci = opts[o];
            if (isconf && ci.initOnly) continue;
            var help = (ci.help?ci.help+'.':'');
            if (ci.readOnly) {
                if (!isconf)
                    continue;
                help += " (readonly)";
            }
            if (ci.customArg && ci.customArg.data)
                help += ' '+ ci.customArg.data;
            rv += "<tr><td>"+ci.name+"</td><td>"+ci.type+"</td><td>"+ help +"</td><td>"+ (ci.init?ci.init:'') +' </td><tr>\n';
        }
        rv += "</table>\n";
        return rv;
    }

    function DumpCmd(cinf) {
        var rv = '', ret = '';
        if (cinf.retType != "any")
            ret = ':'+cinf.retType;
        var cnam = format("%s(%s)%s", cinf.name, cinf.args, ret);
        index += "<a href='#"+cinf.name+"'>"+cinf.name+"</a>\n";
        rv += '<a name="'+cinf.name+'"></a>\n';
        rv += '\n<hr>\n';
        rv += '\n\n<h2>'+cinf.name+'</h2>\n\n';
        rv += format("<font color=red>Synopsis: %s(%s)%s</font><p>\n\n", cinf.name, cinf.args, ret);
        if (cinf.help)
            rv += cinf.help+".\n\n";
        if (cinf.info)
            rv += cinf.info+'\n\n';
        if (cinf.options != NULL) {
            rv += DumpOpts(cinf.options, cinf.name, false);
        }
        rv += '<a name="'+cinf.name+'end"></a>\n';
        rv += '\n<p><a href="#TOC">Return to top</a>\n';
       return rv+'\n';
    }
    function LinkOpts(astr,name) {
        var ii;
        if ((ii=astr.indexOf('options'))<0)
            return astr;
        var ss = '';
        if (ii>0)
            ss += astr.slice(0, ii);
        ss += "<a href='#"+name+"Options'>options</a>";
        ss += astr.slice(ii+7);
        return ss;
    }

    function DumpObj(tinf) {
        var hasconf, ro = '', rv = '', ci, cnam = tinf.name, cmds = Info.cmds(cnam+'.*'), ret = '';
        index += "<a href='#"+tinf.name+"'>"+cnam+"</a>\n";
        rv += '<a name="'+tinf.name+'"></a>\n';
        rv += '\n<hr>\n';
        rv += '\n\n<h2>'+cnam+'</h2>\n\n';
        rv += "<font color=red>Synopsis:";
        if (tinf.constructor) {
            if (tinf.retType != "any")
                ret = ':'+tinf.retType;
            rv += 'new '+cnam+"("+tinf.args+")"+ret+"\n\n";
        } else {
            rv += cnam+".method(...)\n\n";
        }
        rv += "</font><p>";
        if (tinf.help)
            rv += tinf.help+".\n\n";
        if (tinf.info)
            rv += tinf.info+'\n\n';
        rv += '\n<h4>Methods</h4>\n';
        rv += "\nThe following methods are available in \""+cnam+"\":\n\n";
        rv += "<table border='1' class=cmdstbl>\n";
        rv += '<tr><th>Method</th><th>Prototype</th><th>Description</th></tr>\n';
        if (tinf.constructor) {
            ci = Info.cmds(cnam+'.'+cnam,true);
            var conhelp = (ci.help?ci.help+'.':''), aastr, tret = '';
            if (ci.retType != "any")
                tret = ':'+ci.retType;
            if (ci.info) conhelp += ci.info;
            aastr = tinf.args;
            if (tinf.options) {
                aastr = LinkOpts(aastr, "new "+cnam);
            }
            rv += "<tr><td>"+cnam+"</td><td>new "+cnam+"("+aastr+")"+tret+" </td><td>"+conhelp+'</td></tr>\n';
        }
        if (cmds !== undefined) {
            for (var cmd in cmds) {
                var nam = cmds[cmd].split('.')[1], tret = '';
                if (nam == cnam)
                    continue;
                ci = Info.cmds(cnam+'.'+nam);
                if (ci.retType != "any")
                    tret = ':'+ci.retType;
                var conhelp = (ci.help?ci.help+'.':'');
                if (ci.info) conhelp += ' '+ci.info;
                if (ci.options) {
                    ro += DumpOpts(ci.options, cnam+'.'+nam, (nam === 'conf'));
                    aastr = LinkOpts(ci.args, cnam+'.'+nam);
                } else {
                    aastr = ci.args;
                }
                //if (nam == 'conf') hasconf = ci.options;
                rv += "<tr><td>"+nam+"</td><td>"+nam+"("+aastr+")"+tret+" </td><td>"+conhelp+'</td></tr>\n';
            }
        }
        rv += "</table>\n";
        if (tinf.options)
            rv += DumpOpts(tinf.options, 'new '+cnam, (nam === 'conf'));
        rv += ro;
        rv += '<a name="'+tinf.name+'end"></a>\n';
        rv += '<p><a href="#TOC">Return to top</a>\n';
        return rv;
    }
    
    
    //*************** BEGIN MAIN **************
    var rv = '', tinf, lst = Info.cmds();
    var vv = Info.version(true);
    var ver = vv.major+'.'+vv.minor+'.'+vv.release;

    puts("<title>Reference</title>\n<i>This page is auto-generated.</i><p>");
    var index = "";

    for (var i in lst) {
        tinf = Info.cmds(lst[i]);
        switch (tinf.type) {
        case 'object':
            rv += DumpObj(tinf);
            break;
        case 'command':
            //rv += DumpCmd(tinf);
            break;
        default:
            throw("bad id");
            continue;
        }
    }
    rv += "</nowiki>";
    index + "\n";
    return 'Builtin Jsi commands. See [./types.wiki|Types] for the syntax used herein.\n\n<a name="TOC"></a>\n<nowiki>\n' + index + rv;
}

if (Info.isMain()) {
    puts(jsi_mkref());
}
