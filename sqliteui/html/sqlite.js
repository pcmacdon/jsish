// Web browser user interface into sqliteui

var that = {
    debug:1,
    msg:null,
    tlist:{ opened:[], dblclick:null, clkcnt:0 },
    qlist:{ sel:-1, dblclick:null, clkcnt:0, data:[] },
    flist:{ sel:-1, dblclick:null, clkcnt:0 },
    nullValue:'NULL',
    queryLimit:20,
    tvals:{},
    parms:{},
    showing:'tvals',
    frames: ["frame_main"],
    dlgmaxrows:20, queries:[],
    qpos:0,
    conf:{ wrap:false },
    inputs: ['where', 'columns', 'limit', 'offset', 'orderby', 'query'],
    defaults: {where:'', columns:'*', limit:20, offset:0, orderby:'', query:''},
    escFunc: null
};

var puts = console.log.bind(console);

function dputs(str) {
    if (that.debug)
        puts(str);
}

function $(str, top) { /* Simple wrapper DOM access ala JQuery */
    var rc;
    if (!top)
        top = document; 
    switch (str.substr(0,1)) {
        case '#': rc = top.getElementById(str.substr(1)); break;
        case '.': rc = top.getElementsByClassName(str.substr(1)); break;
        default:  rc = top.getElementsByTagName(str); break;
    }
    return rc;
}

function onload()
{
    puts("ONLOAD");
    var pcol, url, u = document.URL;
    if (u.substring(0, 5) == "https") {
        pcol = "wss://";
        u = u.substr(8);
    } else {
        pcol = "ws://";
        if (u.substring(0, 4) == "http")
            u = u.substr(7);
    }
    u = u.split('/');
    url = pcol + u[0];
    that.ws = new WebSocket(url, "jsi-protocol");
    that.ws.onmessage = WebRecv;
    setTimeout(StartConn, 50);
    var lst = that.inputs;
    var pid = $('#parm_query');
    window.addEventListener("keyup", function(e){ return EscPress(e);}, false);
    pid.addEventListener("keyup", function(e){ return KeyPress(e);}, false);
    var fe = function(e){ DoEnter(e,lst[i]); };
    for (var i in lst) {
        $('#parm_'+lst[i]).onkeypress = fe;
    }
    $('#parm_columns').value = '*';
    $('#parm_limit').value = that.queryLimit;
    /*
    $('#tlist').bind('mousewheel DOMMouseScroll', function(e) {
        var scrollTo = null;
    
        if (e.type == 'mousewheel') {
            scrollTo = (e.originalEvent.wheelDelta * -1);
        }
        else if (e.type == 'DOMMouseScroll') {
            scrollTo = 40 * e.originalEvent.detail;
        }
    
        if (scrollTo) {
            e.preventDefault();
            $(this).scrollTop(scrollTo + $(this).scrollTop());
        }
    });*/
}

/****** SHARED ***/


function ClearSelection() {
    if(document.selection && document.selection.empty) {
        document.selection.empty();
    } else if(window.getSelection) {
        var sel = window.getSelection();
        sel.removeAllRanges();
    }
}

function ClassAdd(id, name, add) { // Add or remove name from class
    var a = id.getAttribute('class');
    if (!a) {
        if (add)
            id.setAttribute('class', name);
        return;
    }
    a = a.split(' ');
    var ii = a.indexOf(name);
    if (add) {
        if (ii>=0) return;
        a.push(name);
    } else {
        if (ii<0) return;
        a.splice(ii);
    }
    id.setAttribute('class', a.join(' '));
}
/**************/

function SelRow(tbl, row, on) {
    var id = $('#'+tbl+'_tr_'+row);
    if (!id) return;
    ClassAdd(id,'rowsel',on);
}

function ClickQList(event)  //Select table.
{
    that.in = 'qlist';
    if (event.ctrlKey) {
        DblClickQList(event);
        return false;
    }
    if (!that.dblDelay)
        return _ClickQList(event);
    if (that.qlist.dbclick) {
        clearTimeout(that.qlist.dbclick);
        that.qlist.dbclick=null;
        DblClickQList(event);
    } else {
        that.qlist.dbclick = setTimeout(function() { _ClickQList(event); } , that.dblDelay);
    }
    ClearSelection();
}

function DblClickQList(event)   //Select table.
{
    var target = event.target || event.srcElement;
    var id, pid = target.parentNode;
    id = pid.id;
    if (id === '') return;
    that.curtbl = 'qlist';
    var rsplit = id.split('_');
    var row = rsplit[2];
    if (rsplit[1] == 'th') { 
        row = 'th_'+rsplit[2];
        var tag = rsplit[2];
        ShowHideRows('qlisttbl', row, tag);       return;
    }
}

function _ClickQList(event)     //Select table.
{
    that.qlist.dbclick=null;
    var target = event.target || event.srcElement;
    var id, pid = target.parentNode;
    id = pid.id;
    if (id === '') return;
    var rsplit = id.split('_');
    var row = rsplit[2];
    if (rsplit[1] == 'th') {
        return DblClickQList(event);
    }
    
   // if (row === that.qlist.sel && that.showing == 'tvals') return row;
    SelRow('qlist', that.qlist.sel, false);
    SelRow('tlist', that.tlist.sel, false);
    SelRow('qlist', row, true);
    that.qlist.sel = row;
    var q = that.qlist.data[row];
    SendQuery(q,true);
    $('#parm_query').value = q;
    return row;
}


function OLD_ClickTList(event) //Select table.
{
    that.in = 'tlist';
    if (event.ctrlKey) {
        DblClickTList(event);
        return false;
    }
    if (!that.dblDelay)
        return _ClickTList(event);
    if (that.tlist.dbclick) {
        clearTimeout(that.tlist.dbclick);
        that.tlist.dbclick=null;
        DblClickTList(event);
    } else {
        that.tlist.dbclick = setTimeout(function() { _ClickTList(event); } , that.dblDelay);
    }
}

function DblClickTList(event)       // Show schema
{
    var target = event.target || event.srcElement;
    var id, pid = target.parentNode;
    id = pid.id;
    if (id === '') return '';
    var rsplit = id.split('_');
    var row = rsplit[2];
    if (rsplit[1] == 'th') row = 'th_'+rsplit[2];
    
    if (row==='') return;
    that.tlist.sel = -1;
    if (row.substr(0,3) == 'th_') {
        var tag = row.substr(3);
        ShowHideRows('tlisttbl', row, tag);
    } else {
        var tbl = that.curTable;
        if (!tbl) return;
        LoadSchema(row);
    }
    ClearSelection();
}

function ClickTList(event)  //Select table.
{
    if (event.ctrlKey) {
        DblClickTList(event);
        return false;
    }
    if (!(that.tlist.clkcnt++))
        $('#tlist').setAttribute('title','');
    var target = event.target || event.srcElement;
    var id, pid = target.parentNode;
    id = pid.id;
    if (id === '')
        return '';
    var rsplit = id.split('_');
    if (rsplit[1] == 'th')
        return DblClickTList(event);
    var row = rsplit[2];
    dputs("ID_: "+id);
    SelRow('qlist', that.qlist.sel, false);
    SelRow('tlist', that.tlist.sel, false);
    SelRow('tlist', row, true);
    // Prevent a dblclick from doing load+schema.
    if (that.tlist.sel == row && that.curtbl == 'tlist')
        return;
    that.curtbl = 'tlist';
    that.tlist.sel = that.tlist.reqload = row;
    var tbl = pid.firstChild.innerHTML;
    if (!tbl)
        return '';
    if (rsplit[1] == 'db') {
        return WebSend('dbLoad', {dbFile:tbl});
    }
    var data = that.tlist.data[row];
    if (!data)
        return; //database file.
    
    switch (data.type) {
        case 'index':
            return LoadSchema(row);
        case 'trigger':
            return LoadSchema(row);
        case 'view':
            break;
        case 'table':
            break;
    }
    defs = {columns:'*', limit:20};
    that.parms[that.curTable] = Parms(defs, true);
    Parms(that.parms[tbl], false);
    that.curTable = tbl;
    ReqLoadTable(tbl);
    //$('#tnav_add_row').disabled=(data.type === 'table')?false:true;
    return row;
}


function ClickFList(event)  //Select file.
{
    that.tlist.dbclick=null;
    var target = event.target || event.srcElement;
    var id, pid = target.parentNode;
    id = pid.id;
    if (id === '') return '';
    var rsplit = id.split('_');
    if (rsplit[1] == 'th')
        return;
    var row = rsplit[2];
    dputs("ID_: "+id);
    SelRow('tblfiles', that.flist.sel, false);
    SelRow('tblfiles', row, true);
    that.flist.sel = row;
}

function ShowHideRows(prefix, row, tag) {
    if (that.tlist.opened[prefix] === undefined)
        that.tlist.opened[prefix] = [];
    if (that.tlist.opened[prefix][row] === undefined)
        that.tlist.opened[prefix][row] = true;
    var i, open = that.tlist.opened[prefix][row];
    var ids = $('.'+prefix+'_'+tag);
    //puts("HHIDS: "+ids);
    for (i=0; i<ids.length; i++)
        ClassAdd(ids[i], prefix+'_hide', open);
    that.tlist.opened[prefix][row] = !open;
}

function ClickTVals(event)      // Select row in current table.
{
    if (!(that.tvals.clkcnt++))
        $('#tvals').setAttribute('title','');
    var target = event.target || event.srcElement;
    var id = target.parentNode.id;
    if (id === '') return;
    that.curtbl = 'tvals';
    var rids = id.split('_'), row = rids[2];
    if (row === that.tvals.sel) return row;
    SelRow('tvals', that.tvals.sel, false);
    SelRow('tvals', row, true);
    that.tvals.sel = row;
    if (row == 0) {
        var tname = target.innerHTML;
        id = $('#parm_orderby');
        if (id.value === tname)
            id.value = tname+' DESC';
        else
            id.value = tname;        
        ReloadTbl();
    }
    return row;
}

function DblClickTVals(event) { // Edit row 
    if (that.showing != 'tvals' || that.in == 'qlist') return;
    var row = ClickTVals(event);
    var id = $('#dlg_edit_tvals');
    id.setAttribute('style', 'display:block');
    DlgEditTVals(row);
    ClearSelection();
}

function Tool(name) {
    var id = DlgOpen('dlg_tool_'+name, true);
}

function DlgOpen(name, open) // Open/close a dialog and shade/unshade frame below.
{
    var id = $('#'+name);
    var fend = that.frames[that.frames.length-1];
    var ft = $('#'+fend);
    if (open) {
        if (fend == name) return;
        id.setAttribute('style','display:block');
        puts("NAME: "+name);
        //document.body.style.overflow = "hidden";
         ClassAdd(ft,'shade',true);
        that.frames.push(name);
        that.escFunc = function () { DlgOpen(name, false); };
        var z = $('input', id);
        if (z && z[0])
            z[0].focus();
    } else {
        if (fend != name) return;
        id.setAttribute('style','display:none');
        that.frames.pop();
        fend = that.frames[that.frames.length-1];
        ft = $('#'+fend);
        ClassAdd(ft,'shade',false);
        if (that.frames.length<=1)
            that.escFunc = null;
        else
            that.escFunc = function () { DlgOpen(fend, false); };
    }
    return id;
}

/**********************/

function TableData(fname, row)
{
    if (row === undefined)
        row = S.accts.sel;
    var data = S.accts.data[row];
    if (row === undefined || data === undefined)
        return Bug("bad row");
    if (fname !== undefined)  {
        var ind = S.accts.alup[fname];
        if (ind === undefined)
            return Bug("bad ind: "+fname);
        return data[ind];
    }
    var rc = {};
    for (var i in S.accts.alup)
        rc[i] = data[S.accts.alup[i]];
    return rc;
}

function DBData(fname, row)
{
    if (row === undefined)
        row = S.trans.sel;
    var data = S.trans.data[row];
    if (row === undefined || data === undefined)
        return Bug("bad row");
    if (fname !== undefined)  {
        var ind = S.trans.alup[fname];
        if (ind === undefined)
            return Bug("bad ind: "+fname);
        return data[ind];
    }
    var rc = {};
    for (var i in S.trans.alup)
        rc[i] = data[S.trans.alup[i]];
    return rc;
}

function TNavUpdateBut()    // Update the active/inactive status of Nav buttons.
{
    var buts = $('button',$('#tvals_parm')),
        //adata = TListData(),
        ofs = parseInt($('#parm_offset').value), 
        maxrows = that.dlgmaxrows,
        max = that.tlist.data[that.tlist.sel].size,
        atend = ((ofs+maxrows)>=max),
        atstart = (ofs<=0);
        
    for (var i in buts) {
        switch (buts[i].id) {
            case 'tnav_row_add': break;
            case 'tnav_row_del':
                buts[i].disabled = (that.tlist.sel<0);
                break;
            case 'tnav_page_down':
            case 'tnav_page_end':
                buts[i].disabled = atend;
                break;
            case 'tnav_page_up':
            case 'tnav_page_start':
                buts[i].disabled = atstart;
                break;
            case undefined: break;
            default:
                puts("Unknown id: "+buts[i].id);
        }
    }
}

function OTNavUpdateBut()
{
    var buts = $('button',$('#tvals_parm'));
    for (var i in buts) {
        switch (buts[i].id) {
            case 'tnav_row_add': break;
            case 'tnav_row_del': break;
            case 'tnav_page_down':
            case 'tnav_page_end': break;
            case 'tnav_page_up':
            case 'tnav_page_start': break;
        }
    }
}

function TNavCmd(cmd)
{
    var id = $('#parm_offset');
    var sz = that.tlist.data[that.tlist.sel].size;
    var val = id.value, ql = that.queryLimit, max = sz-ql-1;
    if (max<0)
        max = 0;
    val = parseInt(val);
    if (isNaN(val))
        val = 0;
    dputs("SZ: "+sz);
    switch (cmd) {
        case "down":
            val += ql;
            break;
        case "up":
            val -= ql;
            break;
        case "start":
            val = 0;
            break;
        case "end":
            val = max;
            break;
    }
    dputs("VAL: "+val);
    if (isNaN(val) || val<0)
        val = 0;
    if (val > max)
        val = max;
    id.value = val;
    ReloadTbl();
}

function TNavAddRow(row)    // Add row 
{
    if (row === undefined)
        row = that.tlist.sel;
    if (row<0)
        return Alert("table not selected");
    var data = that.tlist.data[row];
    if (!data || data.type !== 'table') return;
    var id = $('#dlg_add_tvals');
    that.curTable = data.name;
    id.setAttribute('style', 'display:block');
    DlgAddTVals(row);
    ClearSelection();
};

function Alert(msg) { window.alert(msg); };
function Confirm(msg) { return window.confirm(msg); };

function TNavDelRow(row)    // Delete row.
{
    var trow = that.tlist.sel;
    var tdata = that.tlist.data[trow];
    if (!tdata || tdata.type !== 'table') return;
    if (row === undefined)
        row = that.tvals.sel;
    var data = that.tvals.data[row];
    var flds = that.tvals.data[0];
    if (!data)
        return Alert("row not selected");
    if (!Confirm("Delete row"))
        return;
        puts("DELE");
    var subd = {table:tdata.name, rowid:data[0]};
    WebSend('tvalsDelete', subd);
};

function DlgEditTVals(row)  //Fill the Edit-Transaction dialog.
{
    var id = DlgOpen('dlg_edit_tvals', true);
    var z = $('tbody',id)[0];
    var ttl = that.tvals.data[0];
    var len = ttl.length-1, dat, dval = '', i, j, n, bd = '', ccnt;
    dat = that.tvals.data[row];
    len = dat.length-1;
    ccnt = parseInt((len+that.dlgmaxrows-2)/that.dlgmaxrows);
    for (i=0; i<len; i++) {
        bd += "<tr>";
        for (j=0; j<ccnt && i<len; j++, i++) {
            dval = ConvChar(dat[i+1]);
            bd += "<td style='text-align:right'><b>"+ttl[i+1]+":</b></td><td><input id=edt_"+ttl[i+1];
            bd += " value='"+dval+"'></input></td>";
        }
        i--;
        bd += "</tr>\n";
    }
    z.innerHTML = bd;
    z = $('thead', id)[0];
    bd = '<tr>';
    for (j=0; j<ccnt; j++)
        bd += "<th>Name</th><th>Value</th>";
    bd += "</tr>";
    z.innerHTML = bd;
    that.tvals.curRow = row;
    that.tvals.curEd = {row:row, table:that.curTable};
}

//Fill the Add-Transaction dialog.
function DlgAddTVals(row)
{
    var id = DlgOpen('dlg_add_tvals', true);
    var z = $('tbody',id)[0];
    var tinfo = that.tlist.data[row], info = tinfo.info;
    var len = info.length, dat, dval = '', i, j, n, bd = '', ccnt;
    ccnt = parseInt((len+(that.dlgmaxrows-1))/that.dlgmaxrows);
    for (i=0; i<len; i++) {
        bd += "<tr>";
        for (j=0; j<ccnt && i<len; j++, i++) {
            var fnam = info[i].name;
            dval = ((info[i].dflt_value===null)?'':info[i].dflt_value);
            if (dval === "NULL" && that.nullValue !== null)
                dval = that.nullValue;
            bd += "<td style='text-align:right'><b>"+fnam+":</b></td><td><input id=edt_"+fnam;
            bd += " value='"+dval+"'></input></td>";
        }
        i--;
        bd += "</tr>\n";
    }
    z.innerHTML = bd;
    z = $('thead',id)[0];
    bd = '<tr>';
    for (j=0; j<ccnt; j++)
        bd += "<th>Name</th><th>Value</th>";
    bd += "</tr>";
    z.innerHTML = bd;
    that.tvals.curRow = row;
    that.tvals.curEd = {row:row, table:that.curTable};
}

function DlgFormFields(fname, prefix)   // Return fields from form.
{
    var x = $("#dlg_"+fname);
    var zl = [];
    zl[0] = $('input',x);
    zl[1] = $('select',x);
    var s, id, n, res = {};
    for (var j in zl) {
        var z = zl[j];
        for (var i in z) {
            if (z[i].id === undefined) continue;
            id = z[i].id.substr(6+prefix.length);
            res[id] = z[i].value;
        }
    }
    return res;
}

function DlgAddFields(fname, prefix)    // Return fields from add form.
{
    var x = $("#dlg_"+fname);
    var z = [];
    z = $('input',x);
    var s, id, n, res = {};
    for (var i in z) {
        if (z[i].id === undefined) continue;
        id = z[i].id.substr(prefix.length+1);
        res[id] = z[i].value;
    }
    return res;
};

function DlgAddRow() {
    flds = DlgAddFields("add_tvals", "edt");
    var trow = that.tlist.sel;
    var tdata = that.tlist.data[trow];
    if (!tdata || tdata.type !== 'table') return;
    var subd = {table:tdata.name, res:flds};
    WebSend('dbAdd',subd);
};

function DlgSubmit(fname, op)   //Handle submit.
{
    if (op == "Cancel") {
        that.escFunc();
        return;
    }
    var flds;
    switch (fname) {
        case "tool_import":
            flds = DlgFormFields(fname,"tool");
            WebSend('dbImport',flds);
            break;
        case "tool_export":
            flds = DlgFormFields(fname,"tool");
            WebSend('dbExport',flds);
            break;
        case "tool_backup":
            flds = DlgFormFields(fname, "tool");
            WebSend('dbBackup',flds);
            break;
        case "tool_read":
            flds = DlgFormFields(fname, "tool");
            WebSend('dbRead',flds);
            break;
        case "tool_add":
            return DlgAddRow();
        default:
            puts("Unknown cmd: "+fname);
    }
}

function DlgDone(m)
{
    dputs("MM: "+JSON.stringify(m));
    if (m.success) {
        that.escFunc();
        if (m.did === 'read')
            LoadAll(m);
    } else {
        alert(m.msg);
    }
}

function DlgSubmitTVals(tag)    //Handle submit in table edit.
{
    var i, fnam = that.frames[that.frames.length-1];
    if (tag == "Cancel")
        return DlgOpen(fnam, false);
    var isEdit = (fnam.indexOf('edit')>=0);
    var row = that.tvals.curEd;
    var x = $('#'+fnam);
    var z = $('input',x);
    var dat, ttl;
    if (isEdit) {
        row = that.tvals.curRow;
        dat = that.tvals.data[row];
        ttl = that.tvals.data[0];
    } else {
        dat = [];
        ttl = [];
        row = that.tvals.curRow;
        var info = that.tlist.data[row].info;
        for (i in info)
            ttl.push(info[i].name);
    }
    var n = 0;
    var res = {};
    for (i in z) {
        if (z[i].id === undefined) continue;
        var id = z[i].id.substr(4); /* Strip leading 4 chars. */
        
        n = ttl.indexOf(id);
        if (that.nullValue !== null && z[i].value === that.nullValue)
            res[id] = null;
        else
            res[id] = z[i].value;
    }
    var subd = {};
    subd.res = res;
    subd.id = dat[0]; /* rowid */
    subd.tag = tag;   /* "Update", "Delete", "Duplicate", "Add" */
    subd.table = that.curTable;
    that.tvals.submit = subd;
    WebSend('tvalsSubmit', subd);
}

function tvalsSubmitAck(m)
{
    var fnam = that.frames[that.frames.length-1];
    if (!m.result)
        return alert(m.msg);
    DlgOpen(fnam, false);
    ReloadTbl();
    if (m.tag == "Add" || m.tag == "Delete") {
        var row = that.tlist.sel;
        var tr = $('#tlist_tr_'+row);
        var td = $('td',tr)[1];
        var inc = (m.tag == "Add" ? 1 : -1);
        var n = parseInt(td.innerHTML)+inc;
        td.innerHTML = n;
    }
}

function tvalsDeleteAck(m) {
    ReloadTbl();
}

function tvalsAddAck(m)
{
    if (!m.success)
        return Alert(m.msg);
    DlgOpen('dlg_add_tvals', false);
    var trow = that.tlist.sel;
    that.tlist.data[trow].size++;
    ReloadTbl();
    RedrawTList();
}


function Parms(rv, save)    // Save (or restore) parms.
{
    var lst = that.inputs;
    for (var i in lst) {
        var ll = lst[i], id = $('#parm_'+ll),
            defs = that.defaults[ll];
        if (save)
            rv[ll] = id.value;
        else
            id.value = (rv===undefined?defs:rv[ll]);
    }
    return rv;
}


function ReqLoadTable(tbl) { // Request table load from JSI.
    var req = {table:tbl};
    that.curTable=tbl;
    req = Parms(req, true);
    req.opts = {mode:'arrays', headers:true};
    WebSend('loadTable', req, '');
}

function ReloadTbl() {
    ReqLoadTable(that.curTable);
}

function LoadSchema(row,text)
{
    var cclass, data=that.tlist.data[row], values=data.info, hd = '', bd = '', typ=data.type;
    that.tvals.sel = -1;
    if (typ == 'table' || typ == 'view') {
        hd += '<caption><button onclick="LoadSchema('+row+','+(text?'false':'true')+')">';
        hd += 'Click to show '+(text?'Schema':'Sql')+' for '+typ+': "';
        hd += that.tlist.data[row].name+'"</button></caption>';
    }
    else {
        hd += '<caption>SQL for '+typ+': "'+that.tlist.data[row].name+'"</caption>';
        text = true;
    }
    if (text) {
        bd += '<tr><td><pre style="white-space:normal">'+ ConvChar(that.tlist.data[row].sql) +'</pre></td></tr>';
    } else {
        for (var i in values) {
            cclass = ' id=tvals_tr_'+i;
            if (i==0)
                hd +=  '<tr id=tvals_th>';
            bd +=  '<tr '+cclass+'>';
            row = values[i];
            for (var j in row) {
                if (i==0)
                    hd +=  '<th id=tvals_th>'+j+'</th>';
                bd += '<td>' + ConvChar(row[j]) + '</td>';
            }
            bd += '</tr>\n';
            if (i==0)
                hd +=  '</tr>';
        }
    }
    var tt = $('#tvals');
    TTT = tt.innerHTML = hd+bd;
    SetShowing("schema");
}

function SetShowing(show, from)
{
    var tpv = $('#tvals_parm');
    that.showing = show;
    if (show == 'tvals' && from == 'loadTable')
        tpv.setAttribute('style','display:block');
    else
        tpv.setAttribute('style','display:none');
}

function LoadSaved()
{
    var tt = $('#qlist');
    var data = that.qlist.data;
    if (!data.length) return;
    bd = '<tr id=qlist_th_main><th>Saved Queries</th></tr>';
    for (var i in data) {
        var q = data[i];
        var qsub = (q.length<=30?q:q.substr(0,30)+'...');
        bd += '<tr id=qlist_tr_'+i+' class="qlisttbl_main"><td>'+qsub+'</td></tr>';
    }
    tt.innerHTML = bd;
}

function SaveQuery(unsave)
{
    var qid = $('#parm_query');
    var uqd='block', sqd='none';
    if (unsave) { sqd='block'; uqd='none'; }
    var sid = $('#SaveQuery');
    var uid = $('#UnsaveQuery');
    var row, q = qid.value.trim(' '), qind = that.qlist.data.indexOf(q);
    sid.setAttribute('style','display:'+sqd);
    uid.setAttribute('style','display:'+uqd);
    if (unsave) {
        if (qind<0)
            return;
        that.qlist.data.splice(qind,1);
        row = -1;
    } else {
        if (qind>=0)
            return;
        row = that.qlist.data.length;
        that.qlist.data.push(q);
    }
    LoadSaved();
    SelRow('qlist', that.qlist.sel, false);
    if (row>0)
        SelRow('qlist', row, true);
    that.qlist.sel = row;
    WebSend("saveQuery", { from:'saveQuery', save:!unsave, query:q } );
}

function LoadTable(m)
{
    SetShowing('tvals', m.from);
    that.tlist.reqload = -1;
    switch (m.opts.mode) {
    case 'html':
        var tt = $('#tvals');
        tt.innerHTML = m.data;
        return;

    case 'arrays':
        var cclass, tag, row, values=m.data, bd = '';
        that.tvals.data = m.data;
        that.tvals.sel = -1;
        if (m.from == "runQuery") {
            var sqd='block', uqd='none';
            if (m.replay) { uqd='block'; sqd='none'; }
            bd += '<caption><button id=SaveQuery onclick="SaveQuery(false)" style="display:'+sqd+'">Save Query</button></caption>';
            bd += '<caption><button id=UnsaveQuery onclick="SaveQuery(true)" style="display:'+uqd+'">Unsave Query</button></caption>';
        }
        for (var i in values) {
            if (i==0) {
                tag='th';
            } else {
                tag='td';
                that.tvals.sel = 1;
            }
            cclass = ' id=tvals_tr_'+i;
            bd +=  '<tr '+cclass+'>';
            row = values[i];
            for (var j in row) {
                if (j==0 && m.from != 'runQuery') continue; /* Rowid */
                var vval = '';
                if (row[j] !== undefined)
                    vval = ConvChar(row[j]);
                bd += '<'+tag+'>' + vval + '</'+tag+'>';
            }
            bd += '</tr>\n';
        }
        $('#tvals').innerHTML = bd;
        if (that.tvals.sel != -1)
            SelRow('tvals', that.tvals.sel, true);
        TNavUpdateBut();
        return;
    }
}

function RedrawTList() {
    LoadTList(that.tlist.mlSave);
}

function LoadTList(ml)      // Load the table-list
{
    dputs("MM: "+JSON.stringify(ml));
    var cclass, j, idx = -1, bd = '<tr id=tlist_th_main><th>Table</th><th>Size</th></tr>\n';
    var i, m = ml.lst, tcnt = 0;
    that.tlist.mlSave = ml;
    that.tlist.db_list = ml.db_list;
    that.tlist.data = m;
    that.tlist.sel = 1;
    var indices = [], views = [], trigs = [];
    for (i in m) {
        var typ = m[i].type;
        if (typ === 'index' && !m[i].name.match(/^sqlite_autoindex/))
            indices.push(i);
        if (typ === 'view')
            views.push(i);
        if (typ === 'trigger')
            trigs.push(i);
        if (typ !== 'table') continue;
        /*if (that.curTable === undefined)
            that.curTable = m[i].name;
        if (that.tlist.sel<0)
            that.tlist.sel = i;*/
        if (tcnt++==0)
            that.curTable = m[i].name;
        cclass = ' id=tlist_tr_'+i+' class="tlist_table tlisttbl_main"';
        bd +=  '<tr '+cclass+'><td>' + m[i].name + '</td><td>' + m[i].size + '</td></tr>\n';
    }
    if (views.length) {
        bd += "<tr id=tlist_th_view><th>View</th><th>Size</th></tr>";
        for (i in views) {
            j = views[i];
            cclass = ' id=tlist_tr_'+j+' class="tlist_view  tlisttbl_view"';
            bd +=  '<tr '+cclass+'><td>' + m[j].name + '</td><td>' + m[j].size + '</td></tr>\n';
        }
    }
    if (trigs.length) {
        bd += "<tr id=tlist_th_trigger><th>Trigger</th><th>Table</th></tr>";
        for (i in trigs) {
            j = trigs[i];
            cclass = ' id=tlist_tr_'+j+' class="tlist_trigger tlisttbl_trigger"';
            bd +=  '<tr '+cclass+'><td>' + m[j].name + '</td><td>' + m[j].tbl_name + '</td></tr>\n';
        }
    }
    if (indices.length) {
        bd += "<tr id=tlist_th_index><th>Index</th><th>Table</th></tr>";
        for (i in indices) {
            j = indices[i];
            cclass = ' id=tlist_tr_'+j+' class="tlist_index  tlisttbl_index"';
            bd +=  '<tr '+cclass+'><td>' + m[j].name + '</td><td>' + m[j].tbl_name + '</td></tr>\n';
        }
    }
    if (ml.db_list.length>=1) { //TODO: finish db support.
        var dbs = ml.db_list;
        bd += "<tr id=tlist_th_db><th>Database Files</th><th>Name</th></tr>";
        for (i in dbs) {
            j = dbs[i];
            //if (j.name == 'main') continue; 
            cclass = ' id=tlist_db_'+j.name+' class="tlist_db  tlisttbl_db"';
            bd +=  '<tr '+cclass+'><td>' + j.file + '</td><td>' + j.name + '</td></tr>\n';
        }
    }
    var tt = $('#tlist');
    tt.innerHTML = bd;
    if (tcnt) {
        SelRow('tlist', that.tlist.sel, true);
        ReqLoadTable(that.curTable);
    }
}

function LoadAll(m)      /* Load schemas and current-table. */
{
    var id = $('title')[0];
    id.innerHTML = 'SqliteUI: '+m.dbtail;
    LoadTList(m);
    if (m.savedq) {
        for (var i in m.savedq) 
            that.qlist.data.push(m.savedq[i].query);
        LoadSaved();
    }
    if (m.integrityFail)
        Alert("DB Integrity Check Failed!");
    else if (m.foreignKeysFail)
        Alert("DB Foreight Keys Check Failed");
} 

function SendQuery(value,replay) {
    var req = {query:value};
    req.opts = {mode:'arrays', headers:true, limit:that.queryLimit};
    req.replay = replay;
    WebSend('runQuery',req,'');
}

function DoEnter(e,inp)     //Handle hitting enter.
{
    if (!e) e = window.event;
    var keyCode = e.keyCode || e.which;
    if (keyCode == '13'){
        if (inp === 'query') {
            var val = $('#parm_query').value.trim(' ');
            if (val !== '') {
                that.queries.push(val);
                that.qpos = 0;
                SelRow('qlist', that.qlist.sel, false);
                SelRow('tlist', that.tlist.sel, false);
                return SendQuery(val,false);
            }
        } 
        ReloadTbl();
        return false;
    }
}

function KeyPress(e)    // Key pressed in query input.
{
    var key = e.keyIdentifier;
    switch (key) {
        case 'Up':
        case 'Down':
            var len = that.queries.length;
            var id = $('#parm_query');
            var val = id.value.trim(' ');
            that.qpos += (key=="Up"?1:-1);
            if (that.qpos<0)
                that.qpos = 0;
            else if (that.qpos>=len)
                that.qpos=len-1;
            val = that.queries[len-1-that.qpos];
            break;
        default: return;
    }
    id.value = (val === undefined ? '' : val);
}

function ToggleUI(ui) {
    var id = $('#'+ui);
    if (id.style.display === 'none')
        id.style.display = 'inline';
    else
        id.style.display = 'none';
}

function ToggleOpt(that,opt) {
    var tvid = $('#tvals');
    old = that.conf[opt];
    that.conf[opt] = !old;
    switch (opt) {
        case 'wrap':
            ClassAdd(tvid, 'conf_spacepre', old);
            break;
        default: dputs("unknown opt: "+opt);
    }
}

function EscPress(e) {
    if(e.keyCode == 27 && that.escFunc) that.escFunc();
}

function DoExit() {
    $('#body_main').innerHTML = '<H3>SqliteUI has exited: please close window</H3>';
}

function StartConn() {
    if (that.ws.readyState == 1)
        WebSend("loadAll", {init:true});
    else
        setTimeout(StartConn, 50);
}

function WebSend (op, data, type) {
    data = { data:data, op:op, type:type };
    data = JSON.stringify(data);
    dputs("SENDING: "+data);
    that.ws.send(data);
}

function WebRecv (msg)
{
    dputs("RECV: "+msg.data);
    var ms=JSON.parse(msg.data);
    that.msg = ms;       
    var m = ms.data;
    switch (ms.op) {
        
        case 'chgAcct':     return LoadTran(m);
        case 'loadTable':   return LoadTable(m);
        case 'loadAll':     return LoadAll(m);
        case 'status':      that.status = m; break;
        case 'tvalsSubmitAck': return tvalsSubmitAck(m);
        case 'tvalsDeleteAck':  return tvalsDeleteAck(m);
        case 'tvalsAddAck':  return tvalsAddAck(m);
        case 'DlgDone':     return DlgDone(m);
        case 'fileBrowse':  return fileBrowse(m);
        case 'error':       alert(m); break;
        case 'exit':        DoExit(); break;
        default:            puts('Unknown webmsg: '+ms.op);
    }
}

function ToggleMe(id) {
    id.value = (id.value=="1"?"0":"1");
}

function FileBrowse() {
    flds = { files_dir:'/tmp' };
    WebSend('fileBrowse',flds);
}

function fileBrowse(d)      // File selector.
{
    dputs("fileBrowse: "+JSON.stringify(d));
    var id = $('#dlg_tool_files');
    var tbl = $('#tblfiles');
    var tbody = $('tbody',tbl)[0];
    var hid = $('#tblfiles_th_0');
    var th = $('th',hid)[0];
    that.flist.data = d;
    th.innerHTML = d.dir;
    var files = d.files;
    var dirs  = d.dirs;
    var n = 1;
    var rc = '<tr id=tblfiles_tr_0><td>../</td></tr>';
     for (var i in dirs) {
        rc += '<tr id=tblfiles_tr_'+ n++ +'><td>' + ConvChar(dirs[i]) + '/</td></tr>';
     }
     for (var j in files) {
        rc += '<tr id=tblfiles_tr_'+ n++ +'><td>' + ConvChar(files[j]) + '</td></tr>';
    }
    tbody.innerHTML = rc;
    DlgOpen('dlg_tool_files', true);
}


function ConvChar( str )
{
  if (str === undefined) return '';
  if (str === null) {
      if (that.nullValue !== null)
        return that.nullValue;
      return '';
  }
  c = {'<':'&lt;', '>':'&gt;', '&':'&amp;', '"':'&quot;', "'":'&#039;', '#':'&#035;'};
  return str.toString().replace( /[<&>'"#]/g, function(s) { return c[s]; } );
}

puts("DONE");
