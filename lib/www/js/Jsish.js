// jsi.js:  Typed function arguments. https://jsish.org/jsig
// Implements  builtin typechecking (sans return types) in Jsi.

var LogDebug = function(){},
  LogTrace = function(){},
  LogTest = function(){},
  LogInfo = console.warn.bind(console.window,'INFO: '),
  LogWarn = console.warn.bind(console.window,'WARN: '),
  LogError = console.error.bind(console.window,'ERROR: ');

var Log = {
  debug: function(){},
  trace : function(){},
  test : function(){},
  info : console.log.bind(console.window,'INFO: '),
  warn : console.warn.bind(console.window,'WARN: '),
  error : console.error.bind(console.window,'ERROR: '),
  conf: function(typ, on) {
    // Enable/disable logging where typ is one of: debug, trace, test.
    if (!typ) typ = 'debug';
    switch (typ) {
      case 'debug': LogDebug = Log.debug = (on?console.log.bind(console.window,'DEBUG: '):function(){}); break;
      case 'trace': LogTrace = Log.trace = (on?console.log.bind(console.window,'TRACE: '):function(){}); break;
      case 'test':  LogTest  = Log.test  = (on?console.log.bind(console.window,'TEST: ') :function(){}); break;
      default: puts('unknown log type, not one of: debug,trace,test');
    }
  }
};

(function () {
"use strict";

var config = {
  mode:'error', 
  disabled:false,
  histmode:false, // Use Vue-router history mode.
  insert:false,
  inline:false,
  interp:{},    // Interp state info from jsish.
  onload:null,
  subopts:{},
  approot:'./',
  jsiroot:'./',
  baseroot:'',
  websock:true,
};
var self = { // Internal self information.
  typeNameStr:'number,string,boolean,array,function,object,regexp,any,userobj,void,null,undefined',
  typeNames:null,
  sigs:{},
  uuid:0,
  srcList:[],
  srcCnt:0,
  errCnt:0,
  errCur:null,
  funcLst:[],
  log:{},
  wsstarted:false
};

function gettype(m) {
  if (m===null) return 'null';
  var typ = typeof m;
  if (typ==='object' && Array === m.constructor)
    return 'array';
  return typ;
}

function parseError(msg) {
  errorCmd('PARSERR: '+msg+' ==> '+jsi.self.curSig);
  return [];
}

function errorCmd(msg) { // Handle error condition.
  jsi.self.errCnt++;
  switch (jsi.config.mode) {
    case 'error': if (console.error) console.error(msg); return;
    case 'throw': console.log(msg); throw(msg);
    case 'log': break;
    case 'alert': alert(msg); break;
    default: console.log('unknown mode: '+jsi.config.mode);
  }
  return console.log(msg);
}

function typeValidate(typ) {
  if (typ === '') return null;
  var tlst = typ.split('|');
  var i = -1;
  if (!jsi.self.typeNames)
    jsi.self.typeNames = jsi.self.typeNameStr.split(',');
  for (i = 0; i<tlst.length; i++)
    if (jsi.self.typeNames.indexOf(tlst[i])<0) {
      parseError("type unknown '"+tlst[i]+'" not one of: '+jsi.self.typeNameStr);
      return null;
    }
  if (tlst.length===1 && tlst[0] === 'any')
    return null;
  return tlst;
}

function getType(val) {
  switch (val) {
    case 'true': case 'false': return 'boolean';
    case 'null': return 'null';
    case 'undefined': return 'undefined';
    default:
      if (val[0] === "'" || val[0] === '"')
        return 'string';
      if (parseFloat(val))
        return 'number';
  }
}

function SigParse(sig) { // Parse string signature and return info.
  jsi.self.curSig = sig;
   // var reg = /^([^)]*)|)(\s*)$/;
  //var vals = reg.exec(sig);
  var sargs = '';
  //if (!vals)
     // throw "invalid method: "+sig;
  //LogDebug(vals);
  var someDef = sig.indexOf('=')>=0;
  if (sig.indexOf(':')<0 && !someDef) // Skip functions with no types or defaults
    return [];
  var res = {};

  var astr = sig.trim();
  var acall = [];
  if (astr !== '') {
    var alst = astr.split(',');
    var minargs = alst.length, maxargs = minargs;
    var last = alst.length-1;
    for (var i = 0; i<=last; i++) {
      var aval = alst[i].trim();
      if (aval === '...') {
        if (i != last)
          return parseError("expected ... to be at end");
        maxargs = -1;
        if (minargs>0)
          minargs--;
        break;
      }
      var rega = /^([a-zA-Z0-9_]+)(:[|a-z]+|)(=.+|)$/;
      var avals = rega.exec(aval);
      if (!avals)
        return parseError("invalid argument: "+aval);
      //LogDebug(avals);
      if (i)
        sargs += ', ';
      var afnam = avals[1];
      if (afnam === '' || avals.length<3)
        afnam = aval;
      sargs += afnam;
      var defval=undefined, atyp = '';
      if (avals.length>2)
        atyp = avals[2].substr(1);
      var tlst = typeValidate(atyp);
      var hasDef = (avals[3] && avals[3] !== '');
      if (hasDef) { // Default value
        if (avals[3] !== '=void') {
          defval = avals[3].substr(1);
          var defType = getType(defval);
          if (defType)
            if (tlst === null)
              tlst = [defType];
            else if (tlst.indexOf(defType)<0)
              tlst.push(defType);
        }
        if (minargs===alst.length)
          minargs = i;
      }
      else if (minargs!==alst.length)
        return parseError("non-default value follows default: "+aval+' in: '+str);
      acall.push({name:afnam, typ:tlst, def:defval});
    }
  }
  res.astr = astr;
  res.min = minargs;
  res.max = maxargs;
  res.args = acall;
  res.sargs = sargs;
  res.ssig = sig;
  return res;
}

function SigConvert(code) { // Convert typed functions to work in browser, adding $jsig and default-value set.
  
  function reMethod(str) {
    var reg = /^function\s*([a-zA-Z0-9_]*)\s*\(([^)]*)\)(:[\|a-z]+|)(\s*)\{$/;
    var vals = reg.exec(str);
    if (!vals) {
      LogWarn("invalid method: "+str);
      return str;
    }
    //LogDebug(vals);
    var fnam = vals[1];
    if (fnam !== '' && fnam[0] !== '_')
      jsi.self.funcLst.push(fnam);
    var someDflt = str.indexOf('=')>=0;
    if (str.indexOf(':')<0 && !someDflt) // Skip functions with no types or defaults
      return str;
    var res = 'function '+fnam+'(';
  
    var astr = vals[2].trim();
    if (astr === '')
      res += str + ') {';
    else {
      var sobj = SigParse(vals[2]);
      //LogDebug('SOBJ',JSON.stringify(sobj));
      res += sobj.sargs+')' + '{ $jsig("'+sobj.ssig+'", arguments); ';
    }

    return res;
  }
  var reg = /function\s*[a-zA-Z0-9_]*\s*\([^)]*\)(:[\|a-z]+|)\s*\{/g;
  return code.replace(reg, reMethod) + '\n<!--JSIG GEN-->';
}

function convertTest(s) {
  var rs = '', lst = s.split('\n');
  for (var i in lst) {
    var u, l = lst[i], len = l.length;
    if (l[0] == ';' && l[len-1]==';') {
      if (l[1]=='/' && l[2]=='/') {
        u = l.substr(3,len-4);
        rs += "puts(\""+u+"\" ==> \");";
          + "try { puts(JSON.stringify("+u+")); puts('\\nFAIL!\\n'); } "
          + "catch(err) { puts('\\nPASS!: err =',err); }\n";
      } else {
        u = l.substr(1,len-2);
        rs += "puts(\""+u+" ==> \"+JSON.stringify("+u+"));\n";
      }
    }
    rs += '\n';
  }
  return rs;
}

function fileext(fn) {
  var i = fn.lastIndexOf('.');
  if (i<0) return '';
  return fn.substr(i+1);
}

function addScript(fn) { // Add script into page.
  var f=document.createElement('script');
  f.setAttribute("type","text/javascript");
  if (fn.indexOf('\n')>=0)
    f.innerHTML = fn;
  else {
    f.setAttribute("src", fn);
    jsi.self.srcCnt++;
    f.setAttribute("onload", "$jsi.self.srcCnt--;");
  }
  var h = document.querySelector("head");
  h.appendChild(f);
}

function errHandler(msg, url, lineNumber) {  
  //save error and send to server for example.
  puts('MMM',msg);
  $jsi.output('<p style="color:red">'+msg+'</p>\n');
  return true;
}


var jsi = {
  unittest:0,
  config:config,
  self:self,

  $: function(sel, top) {
    if (typeof sel !== 'string') throw('expected string, got '+typeof sel);
    var rc;
    if (!top)
      top = document; 
    return top.querySelectorAll(sel);
  },
  
  $jsig: function(sig, args) { // Check function arguments
    if (jsi.config.disabled)
      return function() {};
    try {
      jsi.jsigImpl(sig, args);
    } catch(e) {
      if (config.interp.asserts && console.assert)
        return console.assert.bind(console.window, false, 'ASSERT: '+e);
      else if (console.warn)
        return console.warn.bind(console.window, 'WARN: '+e);
      else
        return console.log.bind(console.window, 'LOG: '+e);
    }
    return (function() {});
  },

  gettype:gettype,
  matchType: function(v1, v2) {
    return gettype(v1) === getype(v2);
  },
  jsigImpl: function(sig, args) { // Check function arguments
    function ArgCheckType(o, aind, val) {
      var af = o.args[aind];
      var tlst = (af?af.typ:null);
      if (!tlst) return;
      var nam = af.name;
      var vtyp = gettype(val);
      for (var i = 0; i<tlst.length; i++) {
        switch (tlst[i]) {
          case "number":  if (vtyp === 'number') return; break;
          case "string":  if (vtyp === 'string') return; break;
          case "boolean": if (vtyp === 'boolean') return; break;
          case "function":if (vtyp === 'function') return; break;
          case "array":   if (vtyp === 'array') return; break;
          case "regexp":  if (vtyp === 'object' && val && val.constructor === RegExp) return; break;
          case "object":  if (vtyp === 'object' && val && val.constructor !== Array) return; break;
          case "any":   return; break;
          case "userobj": if (vtyp === 'object') return; break;
          case "undefined": case "void": if (val === undefined) return; break;
          case "null": if (val === null) return; break;
          default: throw("type '"+tlst[i]+'" is unknown: not one of: '+jsi.self.typeNameStr);
        }
      }
      throw 'type mismatch for arg '+(aind+1)+' "'+nam+'" expected "'+tlst.join('|')+'" got "'+vtyp+'" '+val;
    }

    var o = sig;
    if (typeof sig === 'string') {
      o = jsi.self.sigs[sig];
      if (!o)
        o = jsi.self.sigs[sig] = SigParse(sig);    
    }
    if (typeof o !== 'object')
      throw('$jsig arg 1: bad sig:'+sig);
    if (typeof args !== 'object')
      throw('$jsig arg 2: expected arguments:'+args);
    
    var len = args.length, msg;
    var pre = '';
    if (o.max>=0 && len>o.max)
      msg = "extra arguments: expected "+o.max+" got "+len;
    else if (len<o.min)
      msg = "missing arguments: expected "+o.min+" got "+len;
    for (var aind = 0; aind<args.length && !msg; aind++)
      msg = ArgCheckType(o, aind, args[aind]);
    if (msg)
      throw msg;
  },
  getUrl:function(url, success, error) {
    return jsi.ajax({url:url, success:success, error:error});
  },
  ajax:function(opts) { // Ajax
    function none(){};
      
    opts = jsi.setopts(opts, {
      success:  null,
      error:    null,
      complete:   null,
      type:     'GET',
      dataType:   'text', // One of: json, jsonp, script, text.
      data:     {},   // Query data.
      headers:  {},
      async:    true,
      url:    null
    });
    if (!opts.url) throw('url is required');
  
    var req, key, i, qd = null, dt = opts.dataType;
    if (!opts.success) opts.success = none;
    if (!opts.error) opts.error = none;
    if (!opts.complete) opts.complete = none;
    switch (dt) {
    case 'jsonp': case 'script': {
      req = document.createElement('script');  
  
      window[opts.data.callback = jsi.guid()] = function(data) {
        opts.success.call(req, data, null, req);
      };
  
      req.onload = req.onerror = function(e) {
        if (e && e.type === "error")
          opts.error.call(jsi, req);
        opts.complete.call(jsi, req);
        req.remove();
      };
  
      qd = '?';
      for (key in opts.data)
        qd += encodeURIComponent(key) + '=' + encodeURIComponent(opts.data[key]) +'&';
  
      req.src = opts.url + qd;
      document.head.appendChild(req);
      break;
    }
    case 'text':
    case 'json': {
      req = new XMLHttpRequest();
      req.open(opts.type, opts.url, opts.async);
      opts.headers["X-Requested-With"] = "XMLHttpRequest";
      for (key in opts.headers)
        req.setRequestHeader(key, opts.headers[key]);
  
      req.onload = req.onerror = function onload() {
        var text = req.statusText;
        if ( req.status < 200 || req.status >= 400 )
          opts.error.call(jsi, req, text);
        else {
          var resp = req.responseText;
          if (dt === 'json')
            resp = JSON.parse(resp);
          opts.success.call(jsi, resp, text, req);
        }
        opts.complete.call(jsi, req, text);
      };
  
      if (opts.data)
        qd = (typeof opts.data === 'string' ? opts.data : JSON.stringify(opts.data));
      req.send(qd);
      break;
    }
    default:
      throw('dataType not one of: json, jsonp, script, text');
    }
    return req;
  },
  
  conf: function(vals) { $jsig('vals:object', arguments)();
    // Configure jsi options.
    var i;
    if (!vals)
      return jsi.config;
    for (i in vals) {
      var ti, tt = gettype(jsi.config[i]), vv = vals[i];
      if (tt == 'undefined')
        errorCmd('Option "'+i+'": not one of: '+Object.keys(jsi.config).join(', '));
      else {
        switch (i) {
          case 'mode':
            var modes = ['error', 'log', 'throw', 'alert'];
            if (modes.indexOf(vals[i])<0)
              errorCmd('invalid mode "'+vals[i]+'": not one of: '+modes.join(','));
            break;
          case 'insert':
            window.onerror = (vals[i]?errHandler:undefined);
            break;
          case 'interp':
            if (vv.log.indexOf('debug')>=0) Log.conf('debug', true);
            if (vv.log.indexOf('trace')>=0) Log.conf('trace', true);
            if (vv.log.indexOf('test')>=0)  Log.conf('test', true);
            break;
        }
        if (jsi.config[i] !== null && tt !== (ti=gettype(vals[i])))
          errorCmd('type mismatch in conf of "'+i+'": '+tt+'!='+ti);
        jsi.config[i] = vals[i];
      }
    }
  },
  parseOpts: function(target, opts, vals) { $jsig('target:object,opts:object,vals:object=void', arguments)();
    // Configure options from vals to target.
    var i;
    for (i in opts)
      if (typeof(target[i]) === 'undefined')
        target[i] = opts[i];
    for (i in vals) {
      var ti, tt = gettype(opts[i]);
      if (tt == 'undefined')
        LogWarn("option "+i+" not one of: "+Object.keys(opts).join(', '));
      else {
        if (opts[i] !== null && tt !== (ti=gettype(vals[i])))
          LogWarn('type mismatch of '+i+': '+tt+'!='+ti);
        target[i] = vals[i];
      }
    }
  },

   getOpts: function(opts, vals, target) {
    // Configure options from vals to target.
    var i;
    if (typeof(target) == 'undefined')
      target = Object.assign(opts);
    else
      for (i in opts)
        if (typeof(target[i]) === 'undefined')
          target[i] = opts[i];
    for (i in vals) {
      var ti, tt = gettype(opts[i]);
      if (tt == 'undefined')
        LogWarn("option "+i+" not one of: "+Object.keys(opts).join(', '));
      else {
        if (opts[i] !== null && tt !== (ti=gettype(vals[i])))
          LogWarn('type mismatch of '+i+': '+tt+'!='+ti);
        target[i] = vals[i];
      }
    }
    return target;
  },   
  filesave: function(filename, data, mime) {  $jsig("filename:string, data:string, mime='text/html'", arguments)();
    // Save data as filename in browser.

    var blob = new Blob([data], {type: (mime?mime:'text/html')});
    if(window.navigator.msSaveOrOpenBlob) {
      window.navigator.msSaveBlob(blob, filename);
    } else {
      var elem = window.document.createElement('a');
      elem.href = window.URL.createObjectURL(blob);
      elem.download = filename;
      document.body.appendChild(elem);
      elem.click();
      document.body.removeChild(elem);
    }
  },
  
  guid: function() { // Return unique UUID.
    if (!jsi.self.uuid)
      jsi.self.uuid = Date.now();
    return '_uuid'+(jsi.self.uuid++).toString(16);
  },
  
  output: function(str) { $jsig('str:string', arguments)();
    // Insert html into page.
    var f=document.createElement('div');
    f.innerHTML = str;
    var h = document.querySelector("body");
    h.appendChild(f);
  },
  htmladd: function(str) { output(str); },
  
  inc: function(fn, onload, onerror) {
    var f=document.createElement('script');
    f.setAttribute("type","text/javascript");
    f.setAttribute("src", fn);
    jsi.self.srcCnt++;
    if (onload)
      f.onload = onload;
    if (onerror)
      f.onerror = onerror;
    var h = document.querySelector("head");
    h.appendChild(f);
  },
  include: function(fns) { $jsig('fns:string|array', arguments)();
    // Dyanmic file include, uses ajax for .jsi files when jsish is not the webserver.
    if (typeof fns === 'string')
      fns = [fns];
    for (var i in fns) {
      var fn = fns[i];
      if (window.jsiWebSocket)
        return addScript(fn);
      jsi.self.srcList.push(fn);
      jsi.self.srcCnt++;
      //jsi.ajax({url:fn+'?id='+jsi.guid() ...});
      jsi.ajax({url:fn,
        success: function(str, txtcode, req) {
          jsi.self.srcCnt--;
          //LogDebug('Src Success: '+str);
          var hdrs = req.getAllResponseHeaders();
          if (!hdrs || hdrs.indexOf('jsiWebSocket')<0) {
            str = SigConvert(str);
            //LogDebug('Src xlate: '+str);
          }
          if (str.indexOf('\n')>=0)
            addScript(str);
          else
            console.warn('no newline', str);
          //eval.call({}, str);
        },
        error:function(str) {
          jsi.self.srcCnt--;
          LogWarn('Src Error: '+str);
        }
      });
    }
  },
  loadCSS: function(css) {
    return jsi.css(css);
  },
  // Insert css link or style tag.
  css: function(css) {
    if (jsi.gettype(css) == 'array') {
      for (var i in css)
        jsi.css(css[i]);
      return;
    }
    var id = document.head || document.getElementsByTagName('head')[0];
    if (css.indexOf('{')>=0) {
      var style = document.createElement('style');
      id.appendChild(style);
      style.type = 'text/css';
      if (style.styleSheet)
        style.styleSheet.cssText = css;
      else
        style.appendChild(document.createTextNode(css));
    } else {
      if (css[0] != '.' && css[0] != '/')
        css = config.approot+css;
      var link = document.createElement('link');
      link.rel="stylesheet";
      link.type="text/css";
      link.href=css;
      id.appendChild(link);
    }
  },
  onload: function(f) { // Set user function to invoke when page/files loaded.
    if (typeof f !== 'function') {
      throw('onload expects a function');
    }
    jsi.config.onload = f;
  },
  
  schema: function(obj, schm) { // Check object/json schema, or generate when schm null/ommited.
            
    function gen(obj) {
      function sub(m,name) {
        var p, i, typ = gettype(m), rc = {type:typ};
        switch (typ) {
          case 'number':
          case 'boolean':
          case 'string':
          case 'null':
            break;
          case 'array':
            if (m[0]===undefined) throw('array must be non-empty '+name);
            rc.items = sub(m[0], name);
            break;
          case 'object':
            rc.properties = {};
            var req = [];
            for (i in m) {
              rc.properties[i] = sub(m[i], i);
              req.push(i);
            }
            if (req.length)
              rc.required = req;
            break;
            
          default:
            console.log('ignoring unsupported type:', typ);
        }
        return rc;
      }
      return sub(obj,'');
    }
  
    function ref(ref, sch, s) {
      if (ref.substr(0,2) === '#/') {
        s = sch;
        ref = ref.substr(2);
      }
      var rlst = ref.split('/');
      for (var i in rlst)
        s = s[rlst[i]];
      return s;
    }

    function check(m, s, name) {
      if (m === undefined) throw('missing value: '+name);
      if (s === undefined) throw('missing schema for: '+name);
      var p, i, typ = gettype(m);
      switch (s.type) {
        case 'number':
        case 'string':
        case 'boolean':
        case 'null':
          if (typ !== s.type) throw('type mismatch: '+typ+'!='+s.type+' at '+name);
          break;
        case 'array':
          if (typ !== s.type) throw('type mismatch: '+typ+'!='+s.type+' at '+name);
          var nn = name+'[]';
          for (i=0; i<m.length; i++)
            check(m[i], s.items, name+'['+i+']');
          break;
        case 'object':
          var req = s.required;
          if (req && req.length) {
            for (p in req) {
              i = req[p];
              if (m[i] === undefined) throw('missing required value : "'+i+'" at '+name);
            }
          }
          var keys = Object.keys(m);
          if (keys.length===1 && keys[0] === '$ref')
            m = ref(m[keys[0]], sch, s);
          for (i in m) {
            if (!m.hasOwnProperty(i)) continue;
            var spi = s.properties[i];
            if (!spi) throw('object property not in schema: "'+i+'" at '+name);
            sub(m[i], spi, name+'.'+i);
          }
          break;
          
        default:
          throw('unsupported schema type '+s.type);
      }
    }
    if (schm === null || arguments.length==1)
      return gen(obj);
    check(obj, schm, '#');
  },

  setopts: function(obj, opts) { // Set opts in obj and return.
    var i;
    for (i in obj)
      if (opts[i] === undefined)
        throw("unknown option: "+i+' is not one of: '+Object.keys(opts).join(', '));
    for (i in opts)
      if (obj[i] === undefined)
        obj[i] = opts[i];
    return obj;
  },
  isfossil:function() {
    return (location.pathname.indexOf('/doc/ckout/')>=0 || location.pathname.indexOf('/doc/tip/')>=0);
  },
  isjsi:function() {
    return (jsi.getCookie('sessionJsi')?true:false);
  },
  websock:function(opts) { // Create websocket connection if jsish is server.
    if (!config.websock || !jsi.getCookie('sessionJsi'))
      return;
    var ws;
    var w = {
      prot:'ws',
      debug:false,
      noreloadexts:'',
      onrecv:null,
      onopen:null,
      onchange:function(fname) {
        location.reload();
      },
      onmessage:function(obj) {
        //puts("MSG: "+obj.data);
        var msg=JSON.parse(obj.data);
        if (msg.mod === '!') {
          switch (msg.cmd) {
            case 'reload':
              var fname = msg.data.fname, fext, fia;
              if (fname)
                fia = fname.lastIndexOf('.');
              if (fia && fia>0)
                fext = fname.substr(fia+1);
              if (w.noreloadexts != '' && fname) {
                if (typeof(w.noreloadexts)==='string')
                  w.noreloadexts = w.noreloadexts.split(',');
                if (fext && w.noreloadexts.indexOf(fext)>=0) {
                  if (w.debug)
                    puts('ignoring changed file: '+fname);
                  return;
                }
              }
              if (w.onchange)
                w.onchange(fname, fext);
            return;
            default: puts('unknown * cmd');
          }
          return;
        }
        if (msg.mod === '*') {
          puts('TODO: broadcast');
          return;
        }
        if (w.onrecv)
          w.onrecv(msg);
      },
    };
    if (self.wsstarted) {
      console.log('websock already started');
      return;
    }
    self.wsstarted = 1;
    if (opts) {
      if (typeof(opts) === 'string') {
        var nops = {}, flst = opts.split('|');
        for (var fli in flst) {
          var fnn = flst[fli].split(':');
          nops[fnn[0]] = fnn[1];
        }
        opts = nops;
      }
      for (var i in opts) {
        if (w[i]===undefined)
          puts(i,'unknown, expected: '+Object.keys(w));
        w[i] = opts[i];
      }
    }
    var url = document.URL.replace(/^http/,'ws').split('#')[0];
    self.ws = ws = new WebSocket(url, w.prot);
    ws.onmessage = w.onmessage;
    if (w.onopen)
      ws.onopen = w.onopen;
    puts('Websock Started');
    return ws;
  },

  getCookie:function(cname) {
    var name = cname + "=";
    var dc = decodeURIComponent(document.cookie);
    var ca = dc.split(';');
    for(var i = 0; i <ca.length; i++) {
      var c = ca[i];
      while (c.charAt(0) == ' ')
        c = c.substring(1);
      if (c.indexOf(name) == 0)
        return c.substring(name.length, c.length);
    }
    return "";
  },
  
  matchObj:function(msg, match, partial, noerror, pfx) { //msg:object,match:string=void,partial=false,noerror=false
    function validate(msg, match, partial) { //msg:object,match:string=void,partial=false
      function typeGet(msg) {
        var tt = gettype(msg);
        if (tt !== 'object')
          throw('expected object');
        var pat = '{', pre = '';
        var keys = Object.keys(msg).sort();
        for (var i=0; i<keys.length; i++) {
          var nni = keys[i];
          pat += pre + nni + ':' + gettype(msg[nni]);
          pre = ',';
        }
        pat += '}';
        return pat;
      }
      
      var len = 1, i, pat = typeGet(msg);
      if (!match)
        return pat;
      if (typeof(match) !== 'string')
        throw('arg 2: expected string');
      
      match = match.replace(/\s/g,'');
      if (pat === match)
        return;
      if (!partial)
        throw('matchOjb failed: expected "'+match+'", not "'+pat+'"');
      var ss = match.substr(1,match.length-2).split(','), sl = {};
      for (i=0; i<ss.length; i++) {
        var st = ss[i].split(':');
        sl[st[0]] = st[1];
      }
      for (i in msg) {
        if (sl[i] && gettype(msg[i]) === sl[i]) continue;
        throw('matchOjb failed: expected "'+match+'", not "'+pat+'"');
      }
    }
    if (!pfx) pfx = '';
    if (!match)
      return validate(msg, match, partial);
    try {
      validate(msg, match, partial);
    } catch(e) {
      if (config.interp.asserts && console.assert && !noerror)
        return console.assert.bind(console.window, false, 'ASSERT:'+pfx+e);
      else if (console.warn)
        return console.warn.bind(console.window, 'WARN: '+pfx+e);
      else
        return console.log.bind(console.window, 'LOG: '+pfx+e);
    }
    return (function() {});
  },
  assert: function(exp, msg) {
    if (!exp) {
      var e = '!'+exp.toString()+' '+msg;
      if (config.interp.asserts && console.assert)
        return console.assert.bind(console.window, false, 'ASSERT:'+e);
      else if (console.warn)
        return console.warn.bind(console.window, 'WARN: '+e);
      else
        return console.log.bind(console.window, 'LOG: '+e);
    }
  },
  
  _main: function(f) { // Startup.
    if (jsi.self.srcCnt>0) {
      Log.debug("waiting for src");
      setTimeout("$jsi._main()", 1000);
      return;
    }
    var mode = location.search.match(/^\?jsi.mode=(.+)$/);
    if (mode && mode[1])
      jsi.conf({mode:mode[1]});
  
    if (jsi.config.onload)
      jsi.config.onload();
    var scr = $('script[src*="/jsi.js?websock="]')[0];
    if (scr) {
      var ss = scr.src.indexOf('=');
      var ssv;
      if (ss>0) {
        ssv = scr.src.substr(ss+1);
        if (ssv==='false') return;
        if (ssv==='true') ssv = undefined;
      }
      jsi.websock(ssv);
    }
  }
};

window['Jsish'] = jsi;
window['$jsi'] = jsi;
window['$jsig'] = jsi.$jsig;
window['$matchObj'] = jsi.matchObj;
window['$assert'] = jsi.assert;

if (typeof window['puts'] === 'undefined')
  window['puts'] = console.log.bind(console.window);  
if (typeof window['$'] === 'undefined')
  window['$'] = $jsi.$;

if (document.readyState !== 'loading')
  jsi._main();
else
  document.addEventListener("DOMContentLoaded", function () {jsi._main();}, false); 

}());
