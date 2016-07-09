// User interface for Ledger.

var S = {
    reconciling:false,
    recEndData:null,
    debug:2,
    accts: {parms:[], ed:{}},
    trans: {ed:{}, offset:0, maxrows:20, sel:1},
    escFunc: null,
    amtMatch: /^(-)?[0-9]+(\.[0-9][0-9])?$/,
    numMatch: /^[0-9]+$/,
    dateMatch: /^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]?$/,
    
    //TODO: refactor menu system.
    menu: [ "Transaction", "Account", "Admin", "Help"],
    menus:{
        Admin: ["Reports", "Backup", "Setup"],
        Account: ["New", "Edit", "Delete", "Reconcile", "Import", "Export"],
        Transaction: ["New", "New@date", "Edit", "Move", "Delete", "Unreconcile"],
        Help: ["About", "Database"]
    },
    accessKeys: ['New']
    
};

S.accts.fields = {
    aid:    { title: "ID" },
    aname:  { title: "Account" },
    attl:   { title: "Balance", render: Dollar  },
    aobal:  { title: "OpenBal", render: Dollar  },
    acbal:  { title: "CloseBal", render: Dollar  },
    arecobal: { title: "RecoBal", render: Dollar  },
    acat:   { title: "Category" },
    atransnums: { title: "Count" }
};

S.trans.fields = {
    tid:    { title: "ID" },
    treco:  { title: "R", render: Render },
    tnum:   { title: "Num" },
    tdate:  { title: "Date" },
    tpayee: { title: "Payee" },
    alist:  { title: "Account", render: Render },
    tsum:   { title: "Amount", render: Dollar },
    truntot:{ title: "Total", render: Dollar },
    tmemo:  { title: "Memo" },
    aid:    { title: "Trans Account"},
    tsched: { title:  "Unused"}
};
    
var puts = console.log.bind(console);
var dputs = function(str) {};
var dputs2 = function(str) {};

function $(str, top) { /* DOM access helper (ala JQuery) */
    var rc;
    if (!top)
        top = document; 
    switch (str.substr(0,1)) {
        case '#': rc = top.getElementById(str.substr(1)); break;
        case '.': rc = top.getElementsByClassName(str.substr(1)); break;
        default:  rc = top.getElementsByTagName(str); break;
    }
    return rc;
};

function Bug(msg) { if (S.debug) window.alert(msg); else Alert(msg); };
function Alert(msg) { window.alert(msg); };
function Confirm(msg) { return window.confirm(msg); };

function Render(id, data, row)
{
    switch (id) {
        case 'treco': if (data != '') return 'R'; return '';
        case 'alist': return LookupAcctName(data);
    }
};

function Dollar(id, data, row)
{
    if (typeof data === "number")
        return data.toFixed(2);
    return "0.00";
};

function ClearSelection() {
    if(document.selection && document.selection.empty) {
        document.selection.empty();
    } else if(window.getSelection) {
        var sel = window.getSelection();
        sel.removeAllRanges();
    }
};

function isVisible (id) { return (id.offsetParent !== null); }

function validateRegex(id, pat, name) {
    if (!isVisible(id)) return;
    var value = id.value.trim();
    //puts("VAL: "+value);
    if (value === '' || pat.test(value))
        return true;
    Alert('Value is not "'+name+'"');
    id.focus();
    return false;
}

function validateList(id, lst, name) {
    if (!isVisible(id)) return;
    var value = id.value.trim();
    puts("VAL: "+value);
    if (value === '' || lst.indexOf(value)>=0)
        return true;
    Alert('Value is not "'+name+'"');
    id.focus();
    return false;
}

function ValidateSum() {
    return validateRegex(this, S.amtMatch, "dollar");
}

function ValidateNum() {
    return validateRegex(this, S.numMatch, "number");
}

function ValidateDate() {
    return validateRegex(this, S.dateMatch, "date");
}

function ValidateAcct() {
    return validateList(this, S.accts.list, "account");
}

function TransAcctDest(row) { /* Return destination account for row */
    var tdata = S.trans.data[row];
    var data = tdata[S.trans.alup.alist];
    return LookupAcctName(data);
};

function LookupAcctName(data) { /* Given alist string, return dest account name or "[SPLIT]" */
    if (data == null) return '';
    var selrow = S.curAid; // TODO: should save curAcc row.
    try {
        var adata = S.accts.data;
        var tlist = data.split(',');
        if (tlist.length != 2) return "[SPLIT]";
        
        selrow = S.accts.ilup[selrow];
        var aid = adata[selrow][S.accts.alup.aid]; // Current account id.
        var taid = tlist[(aid == tlist[0]) ? 1 : 0]; // Target id.
        var xidx = S.accts.ilup[taid]; // Get data for target
        var rc = adata[ xidx ][S.accts.alup.aname];
        return rc;
    } catch(e) {
        puts("BAD("+data+"): :"+e);
    }
    return "";
};


function TNavUpdateBut() { // Update the active/inactive status of trans Nav buttons.
    var buts = $('button',$('#trans_nav')),
        adata = AcctData(),
        ofs = S.trans.offset, 
        max = S.trans.maxrows,
        atend = ((ofs+max)>=adata.atransnums),
        atstart = (ofs<=0);
        
    for (var i in buts) {
        switch (buts[i].id) {
            case 'tnav_row_add': break;
            case 'tnav_row_del':
                buts[i].disabled = (S.trans.sel<0);
                break;
            case 'tnav_page_down':
            case 'tnav_page_end':
                buts[i].disabled = atend;
                break;
            case 'tnav_page_up':
            case 'tnav_page_start':
                buts[i].disabled = atstart;
                break;
            default:
                if (buts[i].id)
                    puts("Unknown id: "+buts[i].id);
        }
    }
}

function TNavCmd(id) {
    puts("TT: "+id);
    var ofs = S.trans.offset, ql = S.trans.maxrows,
        max = AcctData('atransnums', S.curAcctInd);
    switch (id) {
        case "add": return DlgTransEdit({op:'New', row:null});;
        case "del": return TranRowDelete();
        case "start": ofs = 0; break;
        case "up":  ofs -= ql; break;
        case "down": ofs += ql; break;
        case "end": 
            ofs = (max-ql);
            break;
    }
    if (ofs<0)
        ofs = 0;
    else if (ofs > (max-ql))
        ofs = (max-ql);
    S.trans.offset = ofs;
    ReloadTbl();
}

function SelRow(tbl, row, on) {
    var id = document.getElementById(tbl+'_tr_'+row);
    //dputs("SELROW: "+row+' '+on);
    if (!id) { puts("NO: "+row); return; }
    if (on)
        id.classList.add('rowsel');
    else
        id.classList.remove('rowsel');
    if (tbl == 'trans' && on) {
        $('#tnav_row_del').disabled = false;
    }
};

function ClickAccts(event) { //Select table.
    //puts("CLICK: ");
    var target = event.target || event.srcElement;
    var id = target.parentNode.id;
    if (!id) return;
    var row = id.split('_')[2];
    dputs("ROW: "+row);
    if (row === S.accts.sel) return;
    SelRow('accts', S.accts.sel, false);
    SelRow('accts', row, true);
    S.accts.sel = row;
    var tbl = target.innerHTML;
    dputs("TT: "+tbl);
    if (!tbl) return;
    var tidx = S.accts.data[row][S.accts.alup.aid]
    S.accts.parms[S.curAid] = Parms({},true);
    ReqLoadTran(tidx);
    //Parms(S.accts.parms[tbl],false);
    //ReqLoadTran(tbl);
    
};

function ClickTrans(event) { // Select row in current table.
    //puts("click xact");
    if (S.reconciling) return;
    var target = event.target || event.srcElement;
    var id = target.parentNode.id;
    if (id === '') return;
    var rids = id.split('_'), row = rids[2];
    //dputs("ROW: "+row);
    if (row === S.trans.sel) return row;
    SelRow('trans', S.trans.sel, false);
    SelRow('trans', row, true);
    S.trans.sel = row;
    return row;
};

function DlgOpen(name, open) // Open/close a dialog and shade/unshade frame below.
{
    var id = document.getElementById(name);
    //dputs("ID: "+id+' '+name);
    var fend = S.frames[S.frames.length-1];
    var ft = document.getElementById(fend);
    if (open) {
        if (fend == name) return;
        id.setAttribute('style','display:block');
        ft.classList.add('shade');
        S.frames.push(name);
        S.escFunc = function () { DlgOpen(name, false); };
    } else {
        if (fend != name) return;
        id.setAttribute('style','display:none');
        S.frames.pop();
        fend = S.frames[S.frames.length-1];
        ft = document.getElementById(fend);
        ft.classList.remove('shade');
        if (S.frames.length<=1)
            S.escFunc = null;
        else
            S.escFunc = function () { DlgOpen(fend, false); };
    }
    return id;
};

function DlgClick(name,op) {
    var id, rt, rf;
    if (op != 'Cancel') {
        switch (name) {
            case 'trans':
                return DlgTransSubmit(op);
            case 'split_trans':
                return DlgSplitsSubmit(op);
            case 'accts':
                return DlgAcctsSubmit(op);
            case 'trans_move':
                return DlgTransMoveSubmit(op);
            case 'export':
                id = $('#adm_exp_type');
                rt = id.options[id.selectedIndex].value;
                rf = $('#adm_exp_fname').value;
                return WebSend("adminExport", {aid:S.curAid, type:rt, file:rf});
            case 'import':
                id = $('#adm_imp_type');
                rt = id.options[id.selectedIndex].value;
                rf = $('#adm_imp_fname').value;
                return WebSend("adminImport", {aid:S.curAid, type:rt, file:rf});
        }
    }
    DlgOpen('dlg_edit_'+name, false);
};

function DlgClickName(name,op) {
    var id, rt, rf;
    if (op != 'Cancel') {
        switch (name) {
            case 'admin_export':
                id = $('#adm_exp_type');
                rt = id.options[id.selectedIndex].value;
                rf = $('#adm_exp_fname').value;
                return WebSend("adminExport", {aid:S.curAid, type:rt, file:rf});
            case 'admin_import':
                id = $('#adm_imp_type');
                rt = id.options[id.selectedIndex].value;
                rf = $('#adm_imp_fname').value;
                return WebSend("adminImport", {aid:S.curAid, type:rt, file:rf});
            case 'admin_backup':
                rf = $('#adm_backup_fname').value;
                return WebSend("adminBackup", {file:rf});
            case 'admin_setup':
                rf = $('#adm_setup_numrows').value;
                rt = $('#adm_setup_cats').checked;
                return WebSend("adminSetup", {aid:S.curAid, numrows:rf, cats:rt});
            case 'admin_init':
                rf = $('#adm_init_aname').value;
                rt = $('#adm_init_cats').checked;
                if (rf.trim() === '')
                    return Alert('account name can not be blank');
                return WebSend("adminInit", {aname:rf, cats:rt});

            case 'admin_login':
                rf = $('#adm_login_aname').value;
                rt = $('#adm_login_create').checked;
                var rp = $('#adm_login_apassword').value;
                if (rf.trim() === '')
                    return Alert('database name can not be blank');
                if (!rf.match(/^[0-9a-zA-Z_]+$/))
                    return Alert('database name can contain only 0-9a-zA-Z_');
                return WebSend("adminLogin", {dbname:rf, password:rp, create:rt});

            default:
                return Alert('Unimplemented')
        }
    }
    DlgOpen('dlg_'+name, false);
};

function DlgAcctsEdit(opts)
{
    var row = opts.row;
    //dputs("EDACCT: "+row);
    //var availableTags = L.acctNames;
    var id = DlgOpen("dlg_edit_accts", true), tt = $('h3', id);
    var ed = {row:row};
    if (opts.duplicate)
        ed.row = null;
    tt[0].innerHTML = (row !== null?"Edit":"New")+" Account";
    if (row !== null) {
        $('#dlg_edit_accts_update').setAttribute('style', 'display:block');
        $('#dlg_edit_accts_new').setAttribute('style', 'display:none');
    } else {
        $('#dlg_edit_accts_update').setAttribute('style','display:none');
        $('#dlg_edit_accts_new').setAttribute('style','display:block');
    }

    var z = $('input', id);
    var dat = null;
    if (row !== null)
        dat = S.accts.data[row];
    //dputs("DDAT: "+dat);
    var flds = S.accts.data[0];
    var n = 0, len = z.length;
    var res = [];
    for (var i = 0; i<len; i++) {
        //dputs("II: "+z[i].id);
        if (z[i].id === undefined)
            continue;
        z[i].value = '';
        if (!dat) continue;
        var id = z[i].id.substr(3);
        n = flds.indexOf(id);
        dputs("N="+n+" ID="+id+' dat='+dat[n]);
        if (n<0) continue;
        switch (id) {
            case 'acatagory':
            case 'ataxable':
                if (dat[n] == 1)
                    z[i].checked = true;
                else
                    z[i].checked = false;

            default:
                z[i].value = dat[n];
        }
    }
    S.accts.ed = ed;
};

function FillFromTpayee () {
    var tpayee;
    var eid = $("#dlg_edit_trans");
    var z = $('input', eid);
    // Return unless only tpayee and tdate are non-empty. Get tpayee.
    for (var i = 0; i<z.length; i++) {
        if (z[i].id == undefined)
            continue;
        var id = z[i].id.substr(3);
        n = S.trans.alup[id];
        switch (id) {
            case 'tdate': break;
            case 'tpayee': tpayee = z[i].value; break;
            default:
                if (z[i].value !== '') return;
            
        }
    }
    if (!tpayee || tpayee == '') return;
    var fnd, ti = S.trans.alup.tpayee, data = S.trans.data;
    for (var i = 1; i<S.trans.data.length; i++) {
        if (tpayee === data[i][ti]) {
            fnd = i;
            break;
        }
    }
    if (!fnd)
        return;
    DlgTransEdit({op:'Fill', row:fnd});
};

function DlgTransMove()      //Move Transaction dialog.
{
    var row = S.trans.sel;
    if (row <= 0)
        return Alert("A transaction is not selected");
    var id = DlgOpen("dlg_edit_trans_move", true);
}

function DlgAcctShow(name)
{
    var row = S.accts.sel;
    if (row <= 0)
        return Alert("An account is not selected");
    var id = DlgOpen("dlg_"+name, true);
}

function ToggleVis(name) {
    var id = $('#'+name);
    id.style.display = (id.style.display == 'block' ? 'none' : 'block');
}

function Show(name, hide)
{
    var id = $('#'+name);
    if (!id)
        return Alert('bad id: '+name);
    if (hide !== undefined)
        id.setAttribute('style', 'display:none');
    else
        id.setAttribute('style', 'display:block');
}


function AcctData(fname, row) {
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

function TranData(fname, row) {
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

function AcctRecoToggle(id, row) {
    var on = id.checked,
        tsum = parseFloat(TranData('tsum', row));
    if (on)
        S.recoDiff -= tsum;
    else
        S.recoDiff += tsum;
    $('#reco_diff').innerHTML = S.recoDiff.toFixed(2);
    puts("ON: "+on);
}

function AcctReconcileEnd(finish)  // Finish reconcilation.
{
    S.reconciling = false;
    var cval = parseFloat($('#reco_closebal').value)
    Show('bannav');
    Show('accts_div');
    Show('banreco', false);
    Show('reco_div', false);
    if (!finish)
        return ReloadTbl();
    var lst = [], id = $('#trans'), inputs = $('input', id);
    for (var i = 0; i<inputs.length; i++) {
        if (inputs[i].checked !== true) continue;
        var pid = inputs[i].parentNode.parentNode.id,
            row = pid.split('_')[2],
            tid = TranData('tid', row);
        lst.push(tid);
    }
    if (lst.length<=0)
        return ReloadTbl();
    var rmemo = $('#reco_memo').value;
    var subd = {lst:lst, aid:S.curAid, rdate:S.recEndData, rmemo:rmemo, arval:cval};
    WebSend('recoDone', subd);
        
}

function AcctReconcileBegin()  // Switch to acct reconcilation.
{
    var row = S.trans.sel;
    if (row <= 0)
        return Alert("A transaction is not selected");
    var arow = S.accts.sel;
    if (arow <= 0)
        return Alert("An account is not selected");
    var ind = S.trans.alup.treco;
    // if (!S.trans.data[row][ind]) return;
    S.reconciling = true;
    S.recEndData = TranData('tdate');
    ReloadTbl();
    Show('bannav', false);
    Show('accts_div', false);
    Show('banreco');
    Show('reco_div');
    var aind = S.accts.ilup[S.curAid];
    var adata = S.accts.data[aind];
    var aname = adata[S.accts.alup.aname];
    $('#banreco').innerHTML = 'Reconciling Account:  "'+aname+'"';
    $('#reco_closedate').value = S.recEndData;
    var arbal = AcctData('arbal');
    $('#reco_openbal').innerHTML = arbal;
    S.recoDiff = -parseFloat(arbal);
    $('#reco_diff').innerHTML = S.recoDiff.toFixed(2);
}

function AcctReportsBegin()
{
    S.reporting = true;
    Show('banrep');
    Show('report_div');
    Show('bannav', false);
    Show('main_sub', false);
    $('#banrep').innerHTML = 'Account Reports';

    // Fill the accounts list.
    var ros = '<OPTION></OPTION>\n', roe = '', data = S.accts.data;
    for (var i = 1; i<data.length; i++) {
        var acat = AcctData('acatagory', i),
            rs = '<OPTION value='+AcctData('aid', i)+'>'+AcctData('aname', i)+'</OPTION>\n';
        if (acat)
            roe += rs;
        else
            ros += rs;
    }
    $('#rep_accts').innerHTML = ros+roe;
    AcctReportUpdate();
}

function DlgAcctReports(op)
{
    switch (op) {
        case 'Done':
        {
            S.reporting = false;
            Show('banrep', false);
            Show('report_div', false);
            Show('bannav');
            Show('main_sub');
            break;
        }
        case 'Print':
        {
            //$('#rep_sub').style.display = 'none';
            window.print();
            break;
        }
        case 'Apply':
            AcctReportUpdate();
            break;
        default:
            return Alert("Unknown report op: "+op);
    }
}

function AcctReportUpdate() {       
    puts("APPUPD"); 
    var subd = {};
    var rt = $('#rep_type');
    subd.type = rt.options[rt.selectedIndex].value;
    var rl = [], rlid = $('#rep_accts'), ro = rlid.selectedOptions;
    for (var i=0; i<ro.length; i++) {
        if (ro[i].value)
            rl.push(ro[i].value);
    }
    subd.accts = rl.join(',');
    subd.catagories = $('#rep_cat').checked;
    subd.reconciled = $('#rep_reconciled').checked;
    subd.sortanum = $('#rep_sortanum').checked;
    subd.startdate = $('#rep_startdate').value;
    subd.enddate = $('#rep_enddate').value;
    switch (subd.type) {
        case "ledger":
            break;
        case "payee":
            break;
        case "reconcile":
            break;
        case "trial":
            subd.startdate = '';
            break;
        case "summary":
            subd.startdate = subd.enddate = '';
            subd.sortanum = subd.reconciled = false;
            break;
    }
    WebSend('acctReport', subd);
};


function acctReport(d)
{
    var val = '', o = d.opts;
    puts("DD: "+d.toString());
    switch (o.type) {
        
        case "ledger":
            val = "<H2>GENERAL LEDGER";
            if (o.startdate != '' || o.enddate != '')
                val += ' -- ' + o.startdate+' TO '+o.enddate;
            val += "</H2>"
            var tsfx = "\n</TABLE>\n", lastacct = null;
              tpre = "\n<TABLE CLASS=repbody><TR><TH>TOTAL</TH><TH>DATE</TH><TH>PAYEE</TH><TH>MEMO</TH></TR>\n";
            for (var i = 1; i<d.resp.length; i++) {
                if (lastacct != d.resp[i][3]) {
                    if (i!=1)
                        val += tsfx;
                    lastacct = d.resp[i][3];
                    val += "<hr><h3>ACCOUNT: "+lastacct+"</h3>\n"+tpre;
                }
                val += "<TR><TD>"+parseFloat(d.resp[i][1]).toFixed(2)+"</TD><TD>"+d.resp[i][2]+"</TD><TD>"+d.resp[i][0]+"</TD><TD>"+d.resp[i][4]+"</TD></TR>\n";
            }
            val += "\n</TABLE>\n";
            break;
        case "payee":
            val = "<H2>TOTALS BY PAYEE";
            if (o.startdate != '' || o.enddate != '')
                val += '  - '+ o.startdate+' TO '+o.enddate;
            val += "</H2>\n<TABLE CLASS=repbody><TR><TH>TOTAL</TH><TH>PAYEE</TH></TR>\n";
            for (var i = 1; i<d.resp.length; i++) {
                val += "<TR><TD>"+parseFloat(d.resp[i][1]).toFixed(2)+"</TD><TD>"+d.resp[i][0]+"</TD></TR>\n";
            }
            val += "\n</TABLE>\n";
            break;
            break;
        case "reconcile":
            break;
            
        case "summary":
            val = "<H2>ACCOUNT SUMMARY";
        case "trial":
            if (val == '')
                val = "<H2>TRIAL BALANCE";
            if (o.startdate != '' || o.enddate != '')
                val += ' -- ' + o.startdate+' TO '+o.enddate;
            val += "</H2>\n<TABLE CLASS=repbody><TR><TH>TOTAL</TH><TH>ACCOUNT</TH></TR>\n";
            for (var i = 1; i<d.resp.length; i++) {
                val += "<TR><TD>"+parseFloat(d.resp[i][1]).toFixed(2)+"</TD><TD>"+d.resp[i][0]+"</TD></TR>\n";
            }
            val += "\n</TABLE>\n";
            break;
    }
    $('#rep_body').innerHTML = val;
}

function DlgTransUnreconcile()
{
    var row = S.trans.sel;
    if (row <= 0)
        return Alert("A transaction is not selected");
    var ind = S.trans.alup.treco;
    if (!S.trans.data[row][ind])
        return;
    if (!Confirm("Remove reconciliation from transaction"))
        return;
    WebSend('tranUnreconcile', {tid:S.trans.data[row][0]});
}

function DlgTransEdit(opts)      //Fill the Edit-Transaction dialog.
{
    var row = opts.row, op = opts.op, isEdit = 0, isFill = 0, isNew = 0;
    switch (op) {
        case 'Edit':     isEdit = 1; break;
        case 'New':      isNew = 1;  break;
        case 'New@date': isNew = 2;  break;
        case 'Fill':     isFill = 1; break;
        default:         return puts("Bad op: "+op);
    }
    if ((isEdit || isFill || isNew==2 ) && row <= 0)
        return Alert("A transaction is not selected");
    var ed = {row:row, alist:null, oldalist:null };
    var dat, id = DlgOpen("dlg_edit_trans", true), tt = $('h3', id);
    if (isFill) {
        ed = S.trans.ed;
        ed.row = null;
    } else {
        ed.op = op;
        tt[0].innerHTML = (isEdit ?"Edit":"New")+" Transaction";
        if (isEdit) {
            $('#dlg_edit_trans_update').setAttribute('style', 'display:block');
            $('#dlg_edit_trans_new').setAttribute('style', 'display:none');
        } else {
            $('#dlg_edit_trans_update').setAttribute('style','display:none');
            $('#dlg_edit_trans_new').setAttribute('style','display:block');
        }
    }
    var z = $('input', id);
    if (!isNew) {
        dat = S.trans.data[row];
    } else {
        dat = null;
    }
    var flds = S.trans.data[0];
    var n = 0, len = z.length;
    dputs("TDAT: "+row+' '+dat);

    var res = [];
    for (var i = 0; i<len; i++) {
        if (z[i].id == undefined)
            continue;
        var id = z[i].id.substr(3);
        n = S.trans.alup[id];
        if (isEdit==0 && isFill == 0) {
            if (id == 'tdate') {
                var srow = S.trans.sel;
                if (opts.atDate && srow >= 0) {
                    var sdat = S.trans.data[srow];
                    z[i].value = sdat[flds.indexOf('tdate')];
                } else {
                    var dd = new Date();
                    z[i].value = dd.toISOString().slice(0,10)
                }
                continue;
            }
            if (id == 'tpayee') {
                z[i].focus();
            }
            
            z[i].value = '';
            continue;
        } else if (isFill) {
             if (id == 'tdate')
                continue;
        }
        //dputs("N="+n+" ID="+id+' V='+dat[n]);
        if (id == "aid") {
            ed.oldalist = S.trans.ed.alist = dat[S.trans.alup.alist];
            z[i].value = TransAcctDest(row);
        /* TODO: finish treco
        } else if (id == "treco") {
            z[i].value = (dat[n] === null ? "" : "R"); */
        } else
            z[i].value = dat[n];
    }

    if (dat) {
        var tgroup = dat[flds.indexOf('tgroup')];
        ed.tid = dat[flds.indexOf('tid')]
        ed.oldtsum = dat[flds.indexOf('tsum')]
    } else {
        tgroup = ed.tid = ed.oldtsum = null;
    }
    ed.tsums = null;
    ed.splitEdit = false;
    ed.splitChg = false;
    ed.tgroup = tgroup;
    if (!isFill) {
        S.trans.ed = ed;
        datepickr('#ed_tdate', {dateFormat:'Y-m-d'});
    }
    if (tgroup)
        WebSend('tranEditStart', {tgroup: tgroup}); // Get the list of tsums for splits
    else {
        S.trans.ed.tidInd = 0;
        S.trans.ed.tsums = [{aid:S.curAid, tid:null, tsum:0}, {aid:null, tid:null, tsum:0}];
    }
};

function tranEditStart(m) { // Receive trans list for tgroup and save index of the one we are editing
    S.trans.ed.tsums = m;
    for (var i = 0; i<m.length; i++) {
        if (m[i].tid == S.trans.ed.tid) {
            S.trans.ed.tidInd = i;
            break;
        }
    }
};

function DlgSplitsEdit()
{
  /* if (L.dlgSplitSetup === undefined) {
        for (var i = 0; i<9; i++)
            $("#esb_"+i).autocompleteArray( L.acctNames, L.completeOpts);
        L.dlgSplitSetup = true;
    }*/
    
    var id = DlgOpen("dlg_edit_split_trans", true);
    var t = S.trans.ed.row;
    var z = $('input', id);
   /* if (!S.trans.ed.tsums)
    {
        S.trans.ed.tsums = [[aid:S.curAid, tid:null, tsum:tsum], [aid:aid, tid:null, tsum:tsum]];
    }*/

    var vals = S.trans.ed.tsums;
    var vval, m = 0, n = -1, len = z.length;

    for (var i = 0; i<len; i++) {
        n++;
        if (n%2)
            vval = S.acctLookup[vals[m].aid];
        else
            vval = vals[m].tsum;
        if (vval !== undefined && vval !== null)
            z[i].value = vval;
        if (n%2) m++;
        if (m>=vals.length) break;
    }
};

function DlgSplitsSubmit(tag) { // Complete a split edit.
    var row = S.trans.ed.row;
    var x = $('#dlg_edit_split_trans');
    var z = $('input',x);
    var vals = S.trans.ed.tsums, sumAll = 0.0;
    var i, m = 0, n = -1, len = z.length, chg = false, alist = [], slist = [], aid;

    // Validate pairs.
    var isend = 0;
    for (i = 0; i<len; i+=2) {
        if (z[i].value == '' && z[i+1].value == '') { isend = 1; continue; }
        var isone = (z[i].value != '' );
        var istwo = (z[i+1].value != '');
        if (isend &&  (isone || istwo))
            return Alert("blank split found");
        if (isend == 0 && !(isone && istwo))
            return Alert("splits must be in pairs");
    }
    
    // Collect terms and validate fields.
    for (i = 0; i<len; i++) {
        n++;
        if (z[i].value == '') break;
        if (n%2) {
            if (z[i].value != S.acctLookup[vals[m].aid]) {
                chg = true;
                aid = S.acctLookup.indexOf(z[i].value);
                if (aid<0)
                    return Alert("invalid account: "+z[i].value);
            } else {
                aid = vals[m].aid;
            }
            alist.push(aid);
        } else {
            if (!S.numMatch.test(z[i].value))
                return Alert("Not a number");
            simAll += parseFloat(z[i].value);
            slist.push(z[i].value);
        }
        if (n%2) m++;
    }
    if (parseInt(sumAll*100) != 0)
        return Alert("does not sum to zero");
    if (alist.length != slist.length)
        return Alert("malformed split");

    // Rebuild ed.tsums.
    if (!chg) {
        for (i = 0; i<alist.length; i++) {
            vals[i].tsum = slist[i];
        }
    } else {
        var xl = [];
        for (i = 0; i<alist.length; i++) {
            xl[i] = {aid:alist[i], tsum:slist[i]};
        }
        S.trans.ed.tsums = xl;
        S.trans.ed.splitChg = true;
    }
    S.trans.ed.alist = alist.join(',');
    S.trans.ed.splitEdit = true;
    DlgOpen('dlg_edit_split_trans', false);
};

function GetAcctId(name) {
    var data = S.accts.data, nind = S.accts.alup.aname, iind = S.accts.alup.aid;
    for (var i = 1; i < data.length; i++) {
        if (data[i][nind] === name)
            return data[i][iind]
    }
};

function DlgTransMoveSubmit() {
    //var dat, id = DlgOpen("dlg_edit_trans_move", true);
    var subd = {aid:S.curAid};    
    var row = S.trans.sel;
    var dat = S.trans.data[row];
    subd.id = dat[0];
    var val = $('#edmv_aid').value;
    if ((subd.new_aid=GetAcctId(val))<0)
        return Alert("bad account");
    if (subd.new_aid == subd.aid)
        return Alert("Source==Dest account: nothing to do");
    WebSend('tranMove', subd);
}

function DlgTransSubmit(tag) {
    var subd = S.trans.ed;
    var row = subd.row;
    var x = $('#dlg_edit_trans');
    var z = $('input',x);
    var dat = S.trans.data[row];
    var n = 0, len = z.length;
    var res = {};
    for (var i = 0; i<len; i++) {
        if (z[i].id == undefined) continue;
        var id = z[i].id.substr(3);
        dputs("GOT: "+id);
        switch (z[i].type) {
            case 'checkbox':
                res[id] = (z[i].checked ? 1 : 0);
                break;
            default:
                res[id] = z[i].value;
                break;
        }
        if (res[id] === '')
            res[id] = null;
    }
    // Check for normal non-split edit
    if (subd.splitEdit === false && subd.tsums.length === 2 && res.tsum != subd.oldtsum) {
        var si = subd.tidInd;
        subd.tsums[si].tsum = res.tsum;
        subd.tsums[si==1?0:1].tsum = -res.tsum;
        var naid = GetAcctId(res.aid);
        if (!naid) 
            return Alert("bad acct: "+res.aid);
        subd.tsums[si==1?0:1].aid = naid
    }
    subd.res = res;
    subd.id = (dat ? dat[0] : null);
    subd.tag = tag;
    //subd.oldaid   = S.trans.ed.oldaid;
    dputs("DlgTransSubmit: "+row+' '+id+' '+JSON.stringify(subd));
    WebSend('tranSubmit', subd);
};

function tranSubmit(m) {
    if (m.status == true) {
        DlgOpen('dlg_edit_trans', false);
        //TODO: rather than reloading, just update the trans/accts on-screen if possible.
        return ReloadAll();
    }
    Alert("submit failed: "+m.msg);
};

function tranMove(m) {
    if (m.status == true) {
        DlgOpen('dlg_edit_trans_move', false);
        return ReloadAll();
    }
    Alert("move failed: "+m.msg);
};

function acctSubmit(m) {
    puts("SUBD: "+m+' '+typeof(m));
    if (m.status == true) {
        DlgOpen('dlg_edit_accts', false);
        return ReloadAll();
    }
    Alert("submit failed: "+m.msg);
};

function dlgComplete(m, name) {
    puts("SUBD: "+m.status+' '+m.msg);
    if (m.status == true) {
        return DlgOpen('dlg_'+name, false);
    }
    Alert(name+" failed: "+m.msg);
};

function adminBackup(m) {
    //puts("SUBD: "+m+' '+typeof(m));
    if (m.status == true) {
        return Alert("Backup complete");
    }
    Alert("Backup failed: "+m.msg);
};

function adminRestore(m) {
    //puts("SUBD: "+m+' '+typeof(m));
    if (m.status == true) {
        return Alert("Restore complete");
    }
    Alert("Restore failed: "+m.msg);
};


function adminInit(m) {
    if (m.status == true) {
        DlgOpen("dlg_admin_init", false);
        return ReloadAll()
    }
    Alert("Init failed: "+m.msg);
};

function adminSetup(m) {
    if (m.status == true) {
        return ReloadAll()
    }
    Alert("Setup failed: "+m.msg);
};

function DlgAcctsSubmit(tag) {
    var subd = S.accts.ed;
    var row = subd.row;
    var x = $('#dlg_edit_accts');
    var z = $('input',x);
    var dat = S.accts.data[row];
    var flds = S.accts.data[0]
    var n = 0, len = z.length;
    var res = {aid:dat?dat[0]:null};
    for (var i = 0; i<len; i++) {
        if (z[i].id == undefined) continue;
        var id = z[i].id.substr(3);
        dputs("GOT: "+id);
        n = flds.indexOf(id);
        switch (z[i].type) {
            case 'checkbox':
                res[id] = (z[i].checked ? 1 : 0);
                break;
            default:
                res[id] = z[i].value;
                break;
        }
        if (res[id] === '')
            res[id] = null;
    }
    if (res[flds.indexOf('aname')] == '')
        return Alert('account name can not be empty');
    subd.res = res;
    subd.id = (dat ? dat[0] : null);
    subd.tag = tag;
    //subd.oldaid   = S.accts.ed.oldaid;
    puts("DlgAcctsSubmit: "+row+' '+id+' '+JSON.stringify(subd));
    WebSend('acctSubmit', subd);
};

function DblTransClick(event) { // Edit trans 
    dputs("clickdbl trans");
    if (S.reconciling) return;
    var row = ClickTrans(event);
    DlgTransEdit({op:"Edit",row:row});
    ClearSelection();
};

function DblAcctsClick(event) { // Edit account
    dputs("clickdbl accts");
    var row = ClickTrans(event);
    var id = $('#dlg_edit_accts');
    id.setAttribute('style', 'display:block');
    ClearSelection();
    DlgAcctsEdit({op:"Edit", row:S.accts.sel});
};

function Parms(rv,save) { // Save/restore parms.
    return rv;
    
    var lst = S.inputs;
    for (var i in lst) {
        var ll = lst[i], id = document.getElementById('parm_'+ll);
        if (save)
            rv[ll] = id.value;
        else
            id.value = (rv==undefined?'':rv[ll]);
    }
    return rv;
};

function LoadAccts(m) {
    var cclass, tag, row, values=m.acctList, bd = '<thead>', aidIdx = -1, anameIdx = -1;
    var lup = S.accts.fields;
    S.accts.data = values;
    S.accts.list = [];
    S.accts.alup = {}; // Lookup for aXXX name column index by name.
    S.accts.ilup = {}; // Lookup for row number by account index.
    S.accts.sel = 1;
    S.acctLookup = {};
    row0 = values[0];
    for (var i in values) {
        row = values[i];
        if (i==0) {
            tag='th';
        } else {
            tag='td';
            S.acctLookup[row[0]] = row[S.accts.alup.aname];
            if (S.curAid === undefined) {
                S.curAid = row[0];
                S.curAcctInd = S.accts.ilup[S.curAid];
            }
            if (S.accts.sel<0)
                S.accts.sel = i;
        }
        cclass = ' id=accts_tr_'+i;
        bd +=  '<tr '+cclass+'>'
        for (var j in row) {
            var rv = row[j];
            rv = (rv===null?'':rv);
            if (i==0) {
                S.accts.alup[rv] = j;
                if (rv === "aid")
                    aidIdx = j;
                if (rv === "aname")
                    anameIdx = j;
                if (lup[row0[j]] && lup[row0[j]].title)
                    rv = lup[row0[j]].title;
            } else if (i>0) {
                if (j == aidIdx)
                    S.accts.ilup[rv] = i;
                if (j == anameIdx)
                    S.accts.list.push(rv);
                if (lup[row0[j]] && lup[row0[j]].render)
                    rv = lup[row0[j]].render(row0[j], rv,i);
            }
            bd += '<'+tag+' class=a_'+row0[j]+'>' + rv + '</'+tag+'>';
        }
        bd += '</tr>\n';
        if (i==0)
            bd += '</thead><tbody>';
    }
    bd += '</tbody>';
    bd = '<table id=accts class="acctstbl" onclick="ClickAccts(event)" ondblclick="DblAcctsClick(event)">'+bd+"</table>";
    var tt = $('#accts_div');
    tt.innerHTML = bd;
    SelRow('accts', S.accts.sel, true);
    autoComplete.Set("acctList", S.accts.list); 
    autoComplete.Set("atypeList", m.atypeList);
};

function UpdateTbl (tbl, row, cell, value) { // Change a value in table.
    var tr = tbl.rows[row], td = tr.cells[cell];
    td.innerHTML = value;
}

function LoadTran(m) { // Load transaction data to table.
    puts("\n\n\n\n\TRAN: "+JSON.stringify(m));
    var cclass, tag, row, values=m.data, bd = '<thead>';
    var tlist = [], lup = S.trans.fields;
    var tt = $('#trans_div');
    S.trans.data = values;
    S.payeeList = m.payees;
    S.trans.alup = {}; // Lookup for aXXX name column index by name.
    S.trans.sel = m.aopts.sel;
    puts("SEL="+S.trans.sel);
    if (S.trans.sel === undefined)
        S.trans.sel = 1;
    else if (S.trans.sel >= S.trans.data.length)
        S.trans.sel = S.trans.data.length-1;
    S.trans.aopts = m.aopts;
    S.trans.offset = m.aopts.offset;
    row0 = values[0];
    for (var i in values) {
        if (i==0) {
            tag='th';
        } else {
            tag='td';
        }
        cclass = ' id=trans_tr_'+i;
        bd +=  '<tr '+cclass+'>'
               row = values[i];
        for (var j in row) {
            var rv = row[j];
            rv = (rv===null?'':rv);
            if (i==0) {
                S.trans.alup[rv] = j;
            }
            if (i==0 && lup[row0[j]]) {
                if (lup[row0[j]].title !== undefined)
                    rv = lup[row0[j]].title;
            } else if (i>0 && lup[row0[j]]) {
                if (row0[j] == 'treco' && S.reconciling) {
                    rv = "<input type=checkbox onchange='AcctRecoToggle(this,"+i+");'></input>\n";
                } else {
                    if (lup[row0[j]].render)
                        rv = lup[row0[j]].render(row0[j],rv,j);
                    if (row0[j] == 'tpayee' && rv !== '' && tlist.indexOf(rv)<0)
                        tlist.push(rv);
                }
            }

            bd += '<' + tag + ' class=t_'+row0[j]+'>' + rv + '</' + tag + '>';
        }
        bd += '</tr>\n';
        if (i==0) {
            bd += '</head><tbody>';
        }
    }
    if (0 && i>=250) { 
        bd += "<tr><td colspan="+row0.length+"><button><B>NEXT ROWS</B></button>";
        bd += "<button><B>MORE ROWS</B></button><button><B>ALL ROWS</B></button>"
        bd += "</td></tr>";
    }
    bd += '</tbody>';
    bd = '<table id=trans class="transtbl" onclick="ClickTrans(event)" ondblclick="DblTransClick(event)" >' + bd+'</table>';
    tt.innerHTML = bd;
    autoComplete.Set('payeeList', S.payeeList = tlist.sort());
    TNavUpdateBut();
    puts("TT: "+S.trans.sel);
    SelRow('trans', S.trans.sel, true)
   // puts("CG: "+tt.innerHTML);
};

function ReqLoadTran(acct) { // Request Tran load from JSI.
    dputs("REQ: "+acct);
    var req = {acct:acct, reconciling:S.reconciling, recEndDate:S.recEndData, offset:S.trans.offset,
        sel:S.trans.sel};
    req.maxrows = S.trans.maxrows;
    S.curAid = acct;
    S.curAcctInd = S.accts.ilup[S.curAid];
    req = Parms(req,true);
    dputs("SV: "+JSON.stringify(req));
    req.opts = {mode:'arrays', headers:true};
    WebSend('loadTran',req,'');
};

function ReloadTbl() {
    ReqLoadTran(S.curAid);
};

function LoadAll(m) {
    /* Load schemas and current table windows. */
    mm=m; //TODO: remove set of global mm
    var id = $('title')[0];
    id.innerHTML = 'Ledger.jsi: '+m.dbtail;
    if (m.integrityFail)
        Alert("DB Integrity Check Failed!");
    else if (m.foreignKeysFail)
        Alert("DB Foreight Keys Check Failed");
    if (m.acctList.length === 0) {
        return DlgOpen("dlg_admin_init", true);
    }
    LoadAccts(m);
    ReqLoadTran(S.curAid);
};

function WebSend (op, data, type) {
    data = { data:data, op:op, type:type };
    data = JSON.stringify(data);
    dputs2("SENDING: "+data);
    S.ws.send(data);
};

function WebRecv (msg) {
    dputs2("RECV "+msg.data);
    var ms = JSON.parse(msg.data);
    S.msg = ms;       
    var m = ms.data;
    switch (ms.op) {
        case 'loadTran':   return LoadTran(m);
        case 'loadAll':     return LoadAll(m);
        case 'connectReq':  return ConnectReq(m);
        case 'status':      S.status = m; break;
        case 'tranEditStart':    return tranEditStart(m);
        case 'acctEditStart':    return acctEditStart(m);
        //case 'acctInfo':    return AcctFillInfo(m);
        case 'tranSubmit':  return tranSubmit(m);
        case 'acctReport':  return acctReport(m);
        case 'acctSubmit':  return acctSubmit(m);
        case 'adminBackup': return dlgComplete(m, "admin_backup");
        case 'adminRestore':return adminRestore(m);
        case 'adminExport': return dlgComplete(m, "admin_export");
        case 'adminImport': return dlgComplete(m, "admin_import");
        case 'adminSetup': return adminSetup(m);
        case 'adminInit':   return adminInit(m);
        case 'tranMove':    return tranMove(m);
        case 'recoDone':    return ReloadTbl(m);
        case 'error':       return Alert(m.msg);
        default: puts('Unknown webmsg: '+ms.op);
    }
};

function ReloadAll() {
    WebSend("loadAll", {browser:navigator.userAgent});
}

function ConnectReq(m) { // Server acked.
    if (m.master) {
        return DlgOpen("dlg_admin_login", true);
    }
    WebSend("loadAll", {browser:navigator.userAgent});
}


function GetKeyUp(e) { 
    switch (e.keyCode) {
        case 27:
            if (S.escFunc && autoComplete._dropdown.style.display != "block") S.escFunc();
            break;
        case 9: break;
        default:
            if (e.target === 'input#ed_aid') {
                //puts("KEY: " + String.fromCharCode(e.keyCode));
            }
    }
};

function AddEvent(id, name, func)
{
    var el = document.getElementById(id);
    if (el.addEventListener) {
        el.addEventListener(name, func, false);
    } else {
        el.attachEvent('on'+name, func);
    }  
}


function AcctRowDelete (row) {  // Delete an account.
    if (row === undefined) {
        if (!confirm("Delete account"))
            return;
        row = S.accts.sel;
    }
    var dat = S.accts.data[row];
    var flds = S.accts.data[0]
    var n = flds.indexOf('aid');
    var subd = {tag:'Delete', res:{aid:dat[n]}};
    WebSend('acctSubmit', subd);
};

function TranRowDelete (row) {  // Delete a transaction.
    if (row === undefined) {
        row = S.trans.sel;
    }
    if (row <= 0)
        return Alert("A transaction is not selected");
    if (!confirm("Delete transaction"))
        return;
    var dat = S.trans.data[row];
    var flds = S.trans.data[0]
    var n = flds.indexOf('tgroup');
    var subd = {tag:'Delete', res:{tgroup:dat[n]}};
    WebSend('tranSubmit', subd);
};

function MenuSub (value) {
    switch (S.curMenu) {
        case "Transaction": {
            switch (value) {
                case "New":         return DlgTransEdit({op:value, row:null});
                case "New@date":    return DlgTransEdit({op:value, row:S.trans.sel,atDate:true});
                case "Edit":        return DlgTransEdit({op:value, row:S.trans.sel});
                case "Delete":      return TranRowDelete();
                case "Move":        return DlgTransMove();
                case "Unreconcile": return DlgTransUnreconcile();
                default:            return Alert("Transaction unimplemented: "+value);
            }
        }
        case "Account": {
            switch (value) {
                case "New":         return DlgAcctsEdit({row:null});
                case "Edit":        return DlgAcctsEdit({row:S.accts.sel});
                case "Delete":      return AcctRowDelete();
                case "Reconcile":   return AcctReconcileBegin();
                case "Export":      return DlgAcctShow('admin_export');
                case "Import":      return DlgAcctShow('admin_import');
                default:            return Alert("Account unimplemented: "+value);
            }
        }
        case "Admin": {
            switch (value) {
                case "Reports":     return AcctReportsBegin();
                //case "Restore":     return DlgOpen('dlg_admin_restore', true);
                case "Backup":      return DlgOpen('dlg_admin_backup', true);
                case "Setup":       return DlgOpen("dlg_admin_setup", true);
                default:            return Alert("Admin unimplemented: "+value);
            }
            
        }
        case "Help": {
            switch (value) {
                case "About":       return DlgOpen("dlg_help_about", true);
                case 'Database':    return WebSend('helpDb');
                default:            return Alert("Help unimplemented: "+value);
            }
        }
        default: Alert("unimplemented: "+S.curMenu);
    }
};

function MenuClick (id,value) {
    dputs("MenuClick: "+id+' '+value);
    if (id == '#subnav') {
        return MenuSub(value);
    }
    if (id != '#banin') return;
    S.curMenu = value;
    var Old, New, ids = $('span', $(id));
    for (var i = 0; i<ids.length; i++) {
        if (ids[i].innerHTML === value) {
            if (ids[i].className == 'menusel') {
                ids[i].className = '';
                $('#nav').style.display = 'none';
                dputs("CLOSE: "+value);
                return;
            }
            ids[i].className = 'menusel';
            New = i;
        } else if (ids[i].className == 'menusel') {
            ids[i].className = '';
            Old =i;
        }
    }
    $('#nav').style.display = 'block';
    dputs("NAV: "+New);
    MenuFill('#subnav', S.menus[S.menu[New]]);
    //SwitchImg(Old,New);
};

function MenuFill (id,values) {
    var rv = '', cnt = 0, fill;
    for (var i = 0; i<values.length; i++) {
        fill = '';
        if (S.accessKeys.indexOf(values[i])>=0)
            fill = ' accesskey='+values[i].substr(0,1);
        if (cnt++) {
            rv += "|";
            //fill = '';
        }
        rv += "<span"+fill+" onclick='MenuClick(\""+id+"\",\""+values[i]+"\""+")'>"+values[i] + "</span>"
    }
    $(id).innerHTML = rv;
};


function StartConn() {
    if (S.ws.readyState == 1)
        WebSend("connectReq", {browser:navigator.userAgent});
    else
        setTimeout(StartConn, 50);
};


function onload() {
    if (S.debug)
        dputs = console.log.bind(console);
    if (S.debug>1)
        dputs2 = console.log.bind(console);
    dputs('onload');
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
    S.ws = new WebSocket(url, "jsi-protocol");
    S.ws.onmessage = WebRecv;
    S.frames = ['frame_main'];
    setTimeout(StartConn, 100);
    window.addEventListener("keyup", GetKeyUp, false);
    MenuFill('#banin', S.menu);
    MenuClick('#banin', 'Transaction');
    
    /* Setup field validators. */
    AddEvent('ed_tsum',  'blur', ValidateSum);
    AddEvent('ed_tnum',  'blur', ValidateNum);
    AddEvent('ed_tdate', 'blur', ValidateDate);
    //AddEvent('ed_aid',   'blur', ValidateAcct);
    for (var i = 0; i<10; i++) {
        AddEvent('esa_'+i, 'blur', ValidateSum);
        //AddEvent('esb_'+i, 'blur', ValidateAcct);
    }
    datepickr('.datepickr', {dateFormat:'Y-m-d'});
    var rinps = $('input',$('#rep_sub'));
    for (var i in rinps) {
        rinps[i].onchange = function () { AcctReportUpdate(); };
    }
};

/*************************************************************************************/
// Keep at end as this function confuses the Geany syntax parser.

function ConvChar( str ) { // Convert to &CODE; in string.
  if (str === null) return;
  c = {'<':'&lt;', '>':'&gt;', '&':'&amp;', '"':'&quot;', "'":'&#039;', '#':'&#035;' };
  return str.toString().replace( /[<&>'"#]/g, function(s) { return c[s]; } );
};


