#!/usr/bin/env jsish
// Generate stubs file for shared lib loading.
// Unfinished, but can be used to generate stubs for arbitrary libs.
var opts = { prefix:'jsi', inFile:'', outFile:'' };

//Parse command line args.
var args = console.args;
for (var i = 0; i<args.length; i+=2) {
    var a = args[i];
    var v = args[i+1];
    if (opts[a] == undefined)
        throw('option "'+a+'" not one of: '+opts.keys().sort().join(', '));
    if (v==undefined)
        throw('expected even number of arguments');
    opts[a] = v;
}
if (opts.inFile=='')
    opts.inFile = opts.prefix+'.h';
if (opts.outFile=='')
    opts.outFile = opts.prefix+'Stubs.h';

opts.pret = opts.prefix.toTitle();
opts.preu = opts.prefix.toUpperCase();

var stubs = [];
var d = '';
d += 'EXTERN int '+opts.pret+'_Stubs__initialize(Jsi_Interp *interp, double version, const char* name, int flags, const char *md5, int bldFlags, int stubSize, int structSizes, void **ptr); /*STUB = 0*/\n';
d += File.read(opts.inFile);
d = d.split('\n');
var l = d.length;
var im;
for (var i = 0; i<l; i++) {
    im = d[i].match(/^EXTERN (.[^\/]*)\/\*STUB = ([0-9]+)\*\//);
    if (!im || im.length != 3) continue;
    var ipos = parseInt(im[2]);
    if (ipos<0) continue;
    if (stubs[ipos]) {
        console.log("DUPLICATE EXTERN: ipos="+ipos+' = '+im[1]+' OLD='+stubs[ipos]);
        continue;
    }
    if (!im[1]) {
        console.log("EMPTY EXTERN: ipos="+ipos+' = '+im[1]);
        continue;
    }
    stubs[ipos] = im[1];
    
}

var l = stubs.length;
var iret, iopts, ipre, iname, lopts, nopts, topts;
var fstub = 'typedef struct '+opts.pret+'_Stubs {\n    uint sig;\n    const char* name;\n    int size;\n    int bldFlags;\n';
fstub +='    const char* md5;\n    void *hook;\n';
var fdefs = '';
var finit = '    JSI_STUBS_SIG,    "'+opts.prefix+'",    sizeof('+opts.pret+'_Stubs),     '+opts.preu+'_STUBS_BLDFLAGS,    '+opts.preu+'_STUBS_MD5,    NULL,\\\n';
for (var i = 0; i<l; i++) {
    var x = stubs[i];
    if (!x) throw("missing index: "+i);
    im = x.match(/([^\(]+)\(([^\)]*)\)/);
    if (!im || im.length != 3) throw('bad stub: '+im.toString());
    iopts = im[2];
    ipre = im[1];
    var ii = ipre.lastIndexOf(' '+opts.pret+'_');
    if (ii<0) { puts("III: "+ipre+" "+opts.pret); throw('skip: '+im[0]); }
    iret = ipre.substr(0, ii);
    iname = ipre.substr(ii+1);
    lopts = iopts.split(',');
    nopts = [];
    for (var j = 0; j<lopts.length; j++) nopts.push('n'+j);
    topts = nopts.join(',');
    fstub += '    '+iret+'(*_'+iname+')('+iopts+');\n';
    if (i)
        fdefs += '#define '+iname+'('+topts+') JSISTUBCALL('+opts.prefix+'StubsPtr, _'+iname+'('+topts+'))\n';
    finit += '    '+iname+',\\\n';
}
fstub += '    void *endPtr;\n} '+opts.pret+'_Stubs;\n\n';

var rc = '#ifndef __'+opts.preu+'_STUBS_H__\n#define __'+opts.preu+'_STUBS_H__\n#include "'+opts.inFile+'"\n\n';
rc += '#define '+opts.preu+'_STUBS_MD5 "'+System.md5(fstub)+'"\n\n';
rc += '#undef '+opts.preu+'_EXTENSION_INI\n#define '+opts.preu+'_EXTENSION_INI '+opts.pret+'_Stubs *'+opts.prefix+'StubsPtr = NULL;\n\n';
rc += '#ifdef HAVE_MUSL\n#define '+opts.preu+'_STUBS_BLDFLAGS 1\n#else\n#define '+opts.preu+'_STUBS_BLDFLAGS 0\n#endif\n';
rc += '#ifndef '+opts.pret+'_StubsInit\n';
rc += '#define '+opts.pret+'_StubsInit(interp,flags) (jsiStubsPtr && jsiStubsPtr->sig == \\\n';
rc += '  JSI_STUBS_SIG?jsiStubsPtr->_Jsi_Stubs__initialize(interp, flags, "'+opts.prefix+'", \\\n';
rc += '  '+opts.preu+'_VERSION, '+opts.preu+'_STUBS_MD5, '+opts.preu+'_STUBS_BLDFLAGS, sizeof(';
rc += opts.pret+'_Stubs), '+opts.preu+'_STUBS_STRUCTSIZES, (void**)&'+opts.prefix+'StubsPtr):JSI_ERROR)\n';
rc += '#endif\n\n';
rc += fstub;
rc += 'extern '+opts.pret+'_Stubs* '+opts.prefix+'StubsPtr;\n\n';
rc += '#define __'+opts.preu+'_STUBS_INIT__\\\n'+finit+'    NULL\n\n';
rc += '#ifdef JSI_USE_STUBS\n\n'+fdefs+'\n#endif\n\n#endif\n';
File.write(opts.outFile, rc);



