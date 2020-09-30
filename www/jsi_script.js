// Script to fossil jsi.
"use strict";
(function () {

function scrollIntoView(id) {
    console.log('SSS');
    setTimeout(function() { id.scrollIntoView();}, 100 );
}

function handleAnchor(name) {
    console.log('handleAnchor '+name);
    if (name == '') return;
    var ii, i, ids;
    ids = document.querySelectorAll('a[name]');
    for (ii = 0; ii<ids.length; ii++) {
        i = ids[ii];
        if (i.name.trim() === name) { scrollIntoView(i); return;}  
    }
    var aids = document.querySelectorAll('h1,h2,h3,h4,h5,h6');
    for (ii = 0; ii<aids.length; ii++) {
        i = aids[ii];
        var hnam = i.innerText.trim();
        if (hnam == name) { console.log('NNN');scrollIntoView(i); return;}
    }
}

function escapeHtml(text, noquote) {
    var noq = (noquote?"":"&quot;");
  return text
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, noq)
      .replace(/'/g, "&#039;");
}

function menuToggle(ev) {
    console.log('EV');
    var id = document.getElementById('sectmenulist');
    if (id.classList.length)
        id.classList = '';
    else
        id.classList = 'hidden';
}
function preprocessJSON(str) {
    return str.replace(/("(\\.|[^"])*"|'(\\.|[^'])*')|(\w+)\s*:/g,
        function(all, string, strDouble, strSingle, jsonLabel) {
            if (jsonLabel) {
                return '"' + jsonLabel + '": ';
            }
            return all;
        });
}

function parseOptions(opts, dopt) {
    var jopts = JSON.parse(preprocessJSON('{'+dopt+'}'));
    var kn = Object.keys(opts);
    for (var k in jopts) {
        if (opts[k] === undefined) {
            throw('option '+k+' not one of: ' + kn.join(','));
        } else if (typeof opts[k] !== typeof jopts[k]) {
            throw('type of option '+k+' not: ' + typeof opts[k]);
        } else
            opts[k] = jopts[k];
    }
    return opts;
}

function makeMenu() {
    var mid = document.getElementById('sectmenu');
    if (!mid) {
        var lpp = location.pathname, lee = lpp.substr(lpp.length-3);
        if (lee != '.md')
            return;
        var mdid =  document.querySelector('.markdown');
        if (!mdid) return;
        var nid = document.createElement('div');
        nid.id = 'sectmenu';
        mdid.insertBefore(nid, mdid.childNodes[0]);
        mid = nid;
    }
    var ln = 'ol', err;
    var opts = {ul:false, closed:false},
        dopt = mid.getAttribute('data-opts');
    if (dopt) {
        try {
            opts = parseOptions(opts, dopt);
        } catch(e) {
            console.log(err=('parse failed for data-opts: '+e));
        }
    }
    if (opts.ul)
        ln = 'ul';
    var ids = document.querySelectorAll('div[id=sectmenu]');
    var mnu = '<span id="sectmenubut">&#9776;&nbsp;&nbsp;Sections';
    if (err)
        mnu += '<b style="color:red" title="JS ERROR: '+err+'">*</b>';
    mnu += '</span>\n<'+ln+' id="sectmenulist"><hr>';
    var ii, i, ni = 0, aids = document.querySelectorAll('h1,h2,h3,h4,h5,h6');
    var min = 0, max = 0, cur;
    for (ii = 0; ii<aids.length; ii++) {
        i = aids[ii];
        if (!i.nodeName) continue;
        var n = parseInt(i.nodeName.substr(1));
        if (!min || n<min) min = n;
        if (!max || n>max) max = n;
    }
    cur = min;
    for (ii = 0; ii<aids.length; ii++) {
        i = aids[ii];
        ni++;
        if (!i.nodeName) continue;
        var n = parseInt(i.nodeName.substr(1));
        if (n<1||n>6) return;
        var anchorName = "";
        while (n>cur) {
            mnu += '<'+ln+'>\n';
            cur++;
        }
        while (n<cur) {
            mnu += '</'+ln+'>\n';
            cur--;
        }
        var hnam = escapeHtml(i.innerText.trim(), 1);
        if (i.id) {
          anchorName = i.id;
        } else {
          anchorName = hnam;
          i.setAttribute("id", anchorName);
        }
        mnu += '<li><a href="#'+anchorName+'">'+hnam+'</a></li>\n';
    }
    while (n<cur) {
        mnu += '<'+ln+'>\n';
        cur--;
    }
    mnu += '</'+ln+'>\n';
    mid.innerHTML = mnu;
    document.getElementById('sectmenubut').addEventListener('click', menuToggle);
    if (opts.closed)
        menuToggle();

}

function TNavCmd(id) {
    console.log('TNAV');
}


function findAnchor() {
    console.log('findAnchor');
    if (window.pageYOffset) return;
    var name = window.location.hash;
    console.log('name: '+name);
    if (!name) return;
    name = name.substr(1).trim();
    handleAnchor(name);
}


function IndentSel(event) // Toggle textarea indent
{
    var id  = document.querySelector('textarea');
    if (!id) return;
    var s = id.selectionStart, f = id.selectionEnd;
    if (s === undefined || f === undefined) return;
    var r = '', v = id.value, i, lst = [], l,
        sel = id.value.substring(s, f),
        spl = sel.split('\n');
    
    if (sel.substr(0,4) == '    ') {
        for (i = 0; i<spl.length; i++) {
            l = spl[i];
            if (l.substr(0,4) == '    ')
                l = l.substr(4);
            lst.push(l);
        }
    } else {
        var len = spl.length;
        if (spl[len-1] === '') {
            spl = spl.slice(0,len-1);
            f--;
        }
        for (i = 0; i<spl.length; i++) {
            l = spl[i];
            l = '    '+l;
            lst.push(l);
        }
    }
    var nv = v.substr(0,s) + lst.join('\n') + v.substr(f);
    id.value = nv;
}

function ShortCut(ev) {
    if (!ev.ctrlKey) return;
    if (ev.key === 'i') IndentSel();
}
function pageLoad() {
    findAnchor();
    makeMenu();
    var id  = document.querySelector('textarea');
    if (id) {
        id.addEventListener("keypress", ShortCut);
    }

    console.log('pageLoad');
}


   if(document.addEventListener) {
    document.addEventListener("DOMContentLoaded", function() {
      pageLoad();
    }, false);
  } else {
    window.attachEvent("onload", function() {
      pageLoad();
    });
  }


  /*
if(window.attachEvent) {
    window.attachEvent('onload', pageLoad);
} else {
    if(window.onload) {
        var curronload = window.onload;
        var newonload = function(evt) {
            curronload(evt);
            pageLoad();
        };
        window.onload = newonload;
    } else {
        window.onload = pageLoad;
    }
}
  */
})();
