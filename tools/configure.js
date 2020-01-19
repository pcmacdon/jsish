#!/usr/bin/env jsish
"use strict";
// Jsi configuration script.

function configure(inp:array):number {
    var input = inp.slice(0);
    var Conf = { // Configuration definitions.
        args:[
            "help",          "Display this help", 
            "buildin=",      "Build-in one or more support libs: sqlite websocket", 
            "config=",       "Read in predefined config Configs/make_XXX.conf",
            "ext=",          "CSV list of Extensions to build-in; (*) defaults to module", 
            "label=",        "Comment to include at top of make.conf output", 
            "mod=",          "CSV list of Extensions to build-out as loadable module",
            "nozip",         "Do not zip jsi library to end of executable",
            "opt=",          "CSV list of feature options to enable; (*) is off by default",
            "prefix=",       "Target directory for install, defaults to /usr/local", 
            "program=",      "Program name, defaults to jsish", 
            "shared",        "Build libjsi shared library", 
            "static",        "Link static", 
            "symbols",       "Compile with debugging symbols", 
            "target=",       "Build target-type (default gcc)", 
            "xcprefix=",     "Cross compiler prefix"
        ],
        ext:[
            "mysql",          "MySql database driver (mod)", 
            "socket",         "Socket driver for tcp/udp (unimpl on windows)", 
            "sqlite",         "Sqlite3 database driver ",
            "websocket",      "Libwebsocket driver (mod)"
        ],
        opt:[
            "base64",        "Base64 encoding", 
            "cdata",         "C struct data facility", 
            "debug",         "Debugging facility", 
            "encrypt",       "Encryption support (btea)", 
            "event",         "Events like setTimeout/setInterval", 
            "info",          "Commands compile in extra help-info strings", 
            "load",          "The 'load' command, for dynamic extensions",
            "markdown",      "HTML markdown", 
            "math",          "Math functions",
            "md5",           "MD5 hash support", 
            "memdebug",      "Enable memory debugging (*)", 
            "miniz",         "Build-in miniz compression statically",
            "readline",      "Use libreadline", 
            "regex",         "Build-in regex statically (eg. windows)",
            "sanitize",      "Address Sanitizer (*)", 
            "sha1",          "SHA1 hash support", 
            "sha256",        "SHA256 hash support", 
            "signal",        "Signals support (unimpl on windows)", 
            "stubs",         "Stubs support", 
            "threads",       "Thread support", 
            "filesys",       "Filesystem support", 
            "zvfs",          "Zip filesystem support"
        ],
        target:[
            "unix",          "Compile using gcc (default)", 
            "musl",          "Build standalone using libmusl", 
            "win",           "Cross compile for windows using mingw"
        ]
    };
    
    var Defs = {  // Default values.
        prefix  :"/usr/local",
        program :"jsish",
        xcprefix:"",
        shared  :0,
        static  :0,
        symbols :0,
        nozip   :0,
        sanitize:0,
        memdebug:0,
        target  :"unix",
        buildin  :"sqlite,websocket",
        label :'',
        ext     :"socket,sqlite,websocket",
        mod     :"mysql",
        opt     :"base64,cdata,debug,encrypt,event,filesys,info,load,markdown,math,md5,readline,sha1,sha256,signal,stubs,threads,zvfs",
        allbuildin:"sqlite,websocket",
        websocketSrc:'libwebsockets-2.0-stable.zip',
        sqliteSrc:'sqlite-amalgamation-3180000.zip',
        srcUrl:'http://jsish.org/download/'
    };
    Defs.allext = Defs.ext+","+Defs.mod;
    Defs.allopt = Defs.opt+",memdebug,miniz,regex,sanitize";
    
    var Hdrs = {
        mysql:'mysql/mysql.h'
    };

    function Split(str:string, sep:string) {
        if (str === "") return [];
        return str.split(sep);
    }
   
    var List = { // Current values.
        allbuildin:  Split(Defs.allbuildin, ','),
        allext:     Split(Defs.allext, ','),
        allmod:     Split(Defs.allext, ','),
        allopt:     Split(Defs.allopt, ','),
        mod:        Split(Defs.mod, ','),
        ext:        Split(Defs.ext, ','),
        opt:        Split(Defs.opt, ','),
        buildin:     Split(Defs.buildin, ','),
        label:    '',
        target:     Defs.target,
        program:    Defs.program,
        prefix:     Defs.prefix,
        xcprefix:   Defs.xcprefix,
        nozip:      0,
        shared:     0,
        symbols:    0,
        static:     0
    };
    
    function getFile(fn:string, todir:string) {
        var furl = Defs.srcUrl+fn;
        var tofn = todir+"/"+fn;
        var tofd = File.rootname(tofn);
        if (File.isdir(tofd)) return;
        exec("wget -o /dev/null -O "+tofn+' '+furl);
        exec("unzip -d "+todir+" "+tofn);
        File.link(todir+'/src', tofd);
    }
    
    // Check that values are in list.
    function valueInList(alist:array, values:string, descr:string) {
        // Verify that value is in alist.
        if (values === "" ) return;
        var vlist=values.split(',');
        for (var vind in vlist) {
            var vval=vlist[vind];
            if (alist.indexOf(vval)<0)
                throw "Invalid "+descr+": "+vval+" is not one of: "+alist.join(', ');
        }
    }
    
    function hasHeader(fn:string) {
        return (exec('echo \'#include "'+fn+'"\' | cpp >/dev/null 2>/dev/null', {retCode:true} )==0);
    }
 
    function dumpSub(list:array) { // Dump sub-list.
        var s = '';
        for (var i=0; i < list.length; i+=2)
            s += format("       %-12s- %s\n", list[i], list[i+1]);
        return s;
    }
    
    function dumpConf() { // Dump Conf.
        var s = '';
        var OPTS = Conf.args;
        for (var i=0; i < OPTS.length; i+=2) {
            s+= format("--%-14s : %s\n", OPTS[i], OPTS[i+1]);
            if (OPTS[i].match(/^ext=/))
                s+=dumpSub(Conf.ext);
            else if (OPTS[i].match(/^opt=/))
                s+=dumpSub(Conf.opt);
            else if (OPTS[i].match(/^target=/))
                s+=dumpSub(Conf.target);
        }
        return s;
    }
    
    // Parse command line arguments.
    function parseArgs() {
        var ai, farg, icnt = 0, ci, flst, fnv, i, copt, cslist, cpre, carg;
        for (ai=0; ai<input.length; ai++) {
            farg = input[ai];
            if (farg.match(/^--config=/))
                break;
        }
        if (ai>=input.length) {
            input.unshift('--config=.');
        }
        for (ai=0; ai<input.length; ai++) {
            farg = input[ai];
            if (!farg.match(/^--/))
                throw("Options must start with --: "+farg);
            copt=farg.substr(2),
                cslist=copt.split('=');
                cpre=cslist[0];
                carg=cslist[1];
            if ((copt.indexOf('=')>=0 && cslist[1] === undefined) ||
                Conf.args.indexOf(copt+'=')>=0)
                throw("Expected argument at: "+farg);
            if (carg !== undefined && carg !== '') {
                var carg0=carg.substr(0,1);
                if (carg0 === "+" ||  carg0 === "-") {
                    carg=carg.substr(1);
                    var calist = Split(carg, ',');
                    if (!List['all'+cpre])
                        throw("bad +/- for option: "+farg);
                    if (carg0 === "+") {
                        //puts("CCC: "+cpre+calist.toString());
                        for (ci in calist)
                            List[cpre].push(calist[ci]);
                    } else {
                        // Produce new list minus values in args.
                        for (ci in calist) {
                            var cidx=List[cpre].indexOf(calist[ci]);
                            if (cidx>=0)
                                List[cpre].splice(cidx, 1);
                            else if (List['all'+cpre].indexOf(calist[ci])<0)
                                throw("unknown element: "+calist[ci]);
                        }
                    }
                    carg = List[cpre].join(',');
                }
            }
            
            switch (cpre) {
                case 'help':
                    puts( "Usage: configure [options]\n" + "Available options:\n"+ dumpConf()+'\n'+
                        "CSV list options may be specified multiple times and lists can use =(+/-), eg. \n\n"+
                        "   ./configure --mod=sockets,sqlite --ext=-signal,stubs --opt=+memdebug --opt=-encrypt,md5");
                    exit(0);
                    break;
                case 'reconfig': // Regenerate files in Configs dir.
                    flst = File.glob('Configs/make_*.conf');
                    for (var fi of flst) {
                        fnv = File.rootname(File.tail(fi)).split('_')[1];
                        puts("DOING: "+fnv);
                        exec('./configure --config='+fnv);
                        File.copy('make.conf', fi, true);
                    }
                    puts("Finished reconfiguring Configs/ directory");
                    exit(0);
                    break;
                case 'config':
                    if (icnt++>100)
                        throw("loop bug");
                    if (carg === 'default')
                        break;;
                    var fn;
                    if (carg == "." && File.exists('make.conf'))
                        fn = 'make.conf';
                    else
                        fn = 'Configs/make_'+(carg==='.'?'default':carg)+'.conf';
                    if (!File.exists(fn)) {
                        var tlst = []; flst = File.glob('Configs/make_*.conf');
                        for (i in flst) {
                            fnv = File.rootname(File.tail(flst[i])).split('_')[1];
                            tlst.push(fnv);
                        }
                        puts('Bad --config.  Not one of: '+tlst.sort().join(', '));
                        exit(1);
                    }
                    var cdat = File.read(fn);
                    var csub = cdat.match('command: ./configure ([^\n]*)');
                    if (!csub) {
                        puts("failed to find configure");
                        exit(1);
                    }
                    csub = csub[1].map(['  ', ' ', '\t', ' ']);
                    var cspl = csub.split(' ');
                    var ni = ai;
                    input.splice(ai,  1);
                    for (i of cspl) {
                        input.splice(ni++, 0, i);
                    }
                    ai--;
                    break;
                case 'ext':
                    valueInList(List.allext, carg, cpre);
                    List.ext=Split(carg, ',');
                    break;
                case 'mod':
                    valueInList(List.allmod, carg, cpre);
                    List.mod=Split(carg, ',');
                    break;
                case 'nozip':
                    List.nozip=1;
                    break;
                case 'opt':
                    valueInList(List.allopt, carg, cpre);
                    List.opt=Split(carg, ',');
                    break;
                    List.prefix=carg;
                    break;
                case 'buildin':
                    valueInList(List.allbuildin, carg, cpre);
                    List.buildin=Split(carg, ',');
                    break;
                case 'shared':
                case 'static':
                    List[cpre]=1;
                    break;
                case 'program':
                case 'prefix':
                case 'label':
                case 'target':
                case 'xcprefix':
                    List[cpre]=carg;
                    break;
                default:
                    throw("Uknown option: "+farg+'\n'+ dumpConf());
                    break;
            }
        }
    }
    
    function outResult() {
        var i, weup, copt, confStr='', INARGS = '', CCDEFS='', MODDEFS='',
            MKOPTS="JSI_CONFIG_DEFINED=1\n";
        
        for (i in List.allopt) {
            copt = List.allopt[i];
            weup=copt.toUpperCase();
            var hv = 0;
            if (List.opt.indexOf(copt)>=0) {
                MKOPTS+="JSI__"+weup+"=1\n";
                hv = 1;
            }
            confStr+="#define JSI__"+weup+"="+hv+"\n";
            CCDEFS+=" -DJSI__"+weup+"="+hv;
        }
    
        for (i in List.buildin) {
            var val= List.buildin[i];
            if (List.ext.indexOf(val)<0)
                List.ext.push(val);
        }
        for (i in List.ext) {
            copt = List.ext[i];
            weup=copt.toUpperCase();
            if (Hdrs[copt] && !hasHeader(Hdrs[copt])) {
                puts("missing header: ignored "+copt);
                continue;
            }
            confStr+="#define JSI__"+weup+"=1\n";
            CCDEFS+=" -DJSI__"+weup+"=1";
            MKOPTS+="WITH_EXT_"+weup+"=1\n";
            var li = List.mod.indexOf(copt);
            if (li>=0)
                List.mod.splice(li,1);
        }
    
        for (i in List.mod) {
            copt = List.mod[i];
            weup=copt.toUpperCase();
            if (List.ext.indexOf(copt)>=0) continue;
            confStr+="#define JSI__"+weup+"=1\n";
            MODDEFS+=" -DJSI__"+weup+"=1";
            MKOPTS+="WITH_MOD_"+weup+"=1\n";
        }
        for (i in List.buildin) {
            copt = List.buildin[i];
            weup=copt.toUpperCase();
            MKOPTS+="BUILDIN_"+weup+"=1\n";
            //if (Defs[i+Src])
               // getFile(Defs[i+Src], i);
        }
    
        if (List.shared) {
            MKOPTS += "JSI_IS_SHARED=1\n";
        }
        var ziplib = (List.nozip?0:1);
        MKOPTS += "JSI__ZIPLIB="+ziplib+"\n";
        MKOPTS += "PROGRAM="+List.program+"\n";
        MKOPTS += "TARGET="+List.target+"\n";
        MKOPTS += "PREFIX="+List.prefix+"\n";
        MKOPTS += "LINKSTATIC="+List.static+"\n";
        MKOPTS += "XCPREFIX="+List.xcprefix+"\n";
        MKOPTS += "BUILDMODS="+List.mod.join(' ')+"\n";
        var vobj = Info.version(true);
        var verStr = vobj.major+'.'+vobj.minor+'.'+vobj.release;
        var makeStr = "# make.conf : " + List.label + "\n" +
            "DEFCONFIG_VER="+verStr+"\n"+
            MKOPTS +
            //"\nPROGFLAGS=-DJSI_USE_CONFIG=1"+
            "\nPROGFLAGS= "+CCDEFS+
            "\nMODFLAGS= "+MODDEFS+'\n';
        

        var summary = "\n# command:" +
            " ./configure "+ input.join(' ') +
            "\n# prefix : "+List.prefix +
            "\n# program: "+List.program +
            "\n# target : "+List.target +
            "\n# shared : "+(List.shared?1:0) +
            "\n# nozip  : "+List.nozip +
            "\n# opt    : "+List.opt.join(',') +
            "\n# ext    : "+List.ext.join(',') +
            "\n# mod    : "+List.mod.join(',') + 
            "\n";
        
        makeStr += summary;
        File.write("jsiconfig.h", confStr);
        File.write("make.conf", makeStr);
        
        puts(summary);
        puts("Configure complete!   For more options type \"./configure --help\"");
        var make = 'make';
        if (exec('uname -o') === "FreeBSD")
            make = 'gmake';
        puts("\nNow type \""+make+"\".");
    }
    
    var rc = 0;
    debugger;
    parseArgs();
    outResult();
    return rc;
}

if (Info.isMain()) {
    configure(console.args);
}

