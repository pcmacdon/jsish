/*
    proc Import {_} {
        # Import transactions
        #TODO: delete old or use temp tree to rename (use trace?).
        variable pc
        upvar $_ {}
        set pt $(t:xaction)
        set pa $(t:aclist)

        set fname [tk_getOpenFile -parent $(w,.) -title [mc {Transactions To Import}]]
        if {$fname == {}} return
        set dir [file dirname $fname]
        set afile [file join $dir aclist$pc(ext)]
        if {[file tail $fname] == "xaction$pc(ext)" && [file exists $afile]} {
            if {[tk_messageBox -message [mc {Import Accounts File?}] -type okcancel -parent $(w,.)] == {ok}} {
                tree op restore $(t:aclist) -file $afile
                FillAccount $_
            }
        }
        tree op restore $(t:xaction) -file $fname
        incr (changed)
        $_ Fix-Sums
        return
    }

    proc Import-QIF-Accounts {_} {
        # Import a QIF accounts file
        upvar $_ {}
        variable pc
        set pt $(t:xaction)
        set pa $(t:aclist)
        set fname [tk_getOpenFile -parent $(w,.) -title [mc {QIF Account File To Import}]]
        if {$fname == {}} return
        set fp [open $fname r]
        Cursor $_ busy
        update
        set accts {}
        set cnt 0
        set iscat 0
        set desc {}
        set name {}
        set lf [open [file join $(-dir) import.log] a+]
        puts $lf "Importing accounts from $fname"
        while {[set n [gets $fp str]]>0} {
            set ch [string index $str 0]
            set rest [string trim [string range $str 1 end]]
            if {[string match Type* $str]} continue
            if {[string match !Type:Cat* $str]} {
                set iscat 1
                puts $lf "accounts are catagories"
                continue
            }
            switch -- $ch {
                N { set name $rest }
                D { set desc $rest }
                I { set p(atype) revenue }
                E { set p(atype) expense }
                R { }
                T {
                    if {$iscat} {
                        set p(ataxed) 1
                    } else {
                        set desc $rest
                    }
                }
                ^ {
                    if {$name == {}} {
                        puts $lf "account already exists: $rest"
                    } elseif {[LookupField $_ aclist aname $name] == {}} {
                        set aid [CreateAccount $_ aname $name acatagory $iscat ainstnotes $desc]
                        lappend accts $aid
                        foreach i [array names p] {
                            $pa update $aid $i $p($i)
                        }
                        puts $lf "created account: $rest"
                    }
                    set desc {}
                    set name {}
                    array unset p
                    incr cnt
                }
            }
        }
        puts $lf {}
        close $lf
        close $fp
        set (v,status1) "$cnt [mc {account/catagories imported}]"
        Cursor $_ normal
        incr (changed)
        foreach aid $accts {
            UpdateRunTotal $_ $aid
        }
        FillAccount $_
        Sort-Accounts $_
        SelAct $_
        return
    }

    proc Export-QIF-Accounts {_ {catagories 0}} {
        # Export a QIF accounts file
        upvar $_ {}
        variable pc
        set pt $(t:xaction)
        set pa $(t:aclist)
        set fname [tk_getSaveFile -parent $(w,.) -title [mc {QIF Account File To Export}]]
        if {$fname == {}} return
        set fp [open $fname w+]
        set cnt 0
        set lf [open [file join $(-dir) import.log] a+]
        puts $lf "Exporting accounts from $fname"
        set alst [$pa find -notop]
        foreach i $alst {
            set iscat [$pa get $i acatagory]
            if {$iscat} {
                if {!$catagories} continue
            } else {
                if {$catagories} continue
            }
            set g [$pa get $i]
            array unset p
            array set p $g
            puts $fp "N$p(aname)"
            puts $fp "D$p(ainstnotes)"
            switch -- $p(atype) {
                revenue { puts $fp "I" }
                expense { puts $fp "E" }
            }
            puts $fp ^
        }
        puts $lf {}
        close $lf
        close $fp
        set (v,status1) "$cnt [mc {exported}]"
        return
    }
    
    proc Export-QIF-Catagories {_} {
        # Export a QIF catagories file
        Export-QIF-Accounts $_ 1
    }

    proc QIF-Export {_ {acct {}}} {
        # Export to QIF file.
        upvar $_ {}
        variable pc
        variable pp
        set pt $(t:xaction)
        set pa $(t:aclist)

        if {$acct == {}} {
            set acct [TreeView index $(w,aclist) focus]
        }
        if {$acct == {}} {
            tk_messageBox -message [mc {Sorry, an account must be selected first}] -type ok -parent $(w,.)
            return
        }
        set aname [$pa get $acct aname]
        set fname [tk_getSaveFile -parent $(w,.) -title [mc {QIF File To Export}]]
        if {$fname == {}} return
        set fp [open $fname w+]
        set cnt 0
        set lf [open [file join $(-dir) import.log] a+]
        puts $lf "Exporting $aname $fname"
        puts $fp "!Type:Bank"
        foreach i [$pt find -notop -key tacct -name $acct] {
            set g [$pt get $i]
            array unset p
            array set p $g
            puts $fp "D[clock format $p(tdate) -format %D]"
            puts $fp "T$p(tsum)"
            if {$p(tpayee) != {}} { puts $fp "P$p(tpayee)" }
            if {$p(tnum) != {}}   { puts $fp "N$p(tnum)" }
            if {$p(tmemo) != {}}  { puts $fp "M$p(tmemo)" }
            if {$p(treco) != {}}  { puts $fp "CR" }
            set ids [$pt tag nodes grp$p(tgroup)]
            if {[llength $ids]>=2 && [set n [lsearch $ids $i]]>=0} {
                set ids [lreplace $ids $n $n]
            } else {
                Notify $_ "invalid group: $p(tgroup)" -icon error
                break
            }
            if {[llength $ids]==1} {
                set i2 [lindex $ids 0]
                set acct2 [$pt get $i2 tacct]
                set aname [$pa get $acct2 aname]
                set ch L
            } else {
                set ch S
            }
            foreach i2 $ids {
                set acct2 [$pt get $i2 tacct]
                set aname2 [$pa get $acct2 aname]
                puts $fp "$ch$aname2"
                if {$ch == "S"} {
                    puts $fp "\$[expr {-[$pt get $i2 tsum]}]"
                }
            }
            puts $fp "^"
            incr cnt
        }
        puts $lf "$cnt transactions exported"
        puts $lf {}
        close $lf
        close $fp
        set (v,status1) "$cnt [mc {transactions exported}]"
        return
    }

*/

    function QIF_Import (inStr, targacct) {
        var p, doinit = true; lst=[], skip = 0, inl = inStr.split('\n');
            
        for (n=0; n<inl.length; n++) {
            var ttim, tsums, tdest,
                str = inl[n], ch = str.substr(0,1), rest = str.substr(1).trim();
            if (str.match(/^Type/)) continue;
            if (doinit) {
                doinit = false;
                p = {treco:null, tpayee:'', tnum:'', tmemo:''};
                tsums = []; tdest = [];
            }
            switch (ch) {
                case 'C':  p['treco'] = ''; break;
                case 'P':  p['tpayee'] = rest; break;
                case 'N':  p['tnum'] = rest; break;
                case 'M':  p['tmemo'] = rest; break;
                case 'T': case '$':  tsums.push(parseFloat(rest.replace(/,\$/g, ''))); break;
                case 'L': case 'S':  tdest.push(rest); break;
                case 'D': {
                    ttim = strptime(rest);
                    var dfmts = ['%D', '%d %b %Y', '%d %b %y'];
                    for (var i = 0; i<dfmts.length && ttim == NaN; i++)
                        ttim = strptime(rest, dfmts[i]);
                    if (ttim == NaN) {
                        ttim = strptime('now');
                        dputs("Date parse failed, using today: "+str);
                    }
                    p['tdate'] = strftime(ttim, "%Y-%m-%d");
                    break;
                }
                case '^':  { // Terminator.
                    doinit = true;
                    if (tsums.length<=0 || tsums.length != tdest.length)
                        dputs("invalid item");
                    else {
                        var tsums = [];
                        if (tsums.length == 1)
                            tsums = {aid:tdest[0], tsum:tsums[0], aid:targacct, tsum:-tsums[0]};
                        else {
                            var s = 0;
                            for (var i = 0; i<tsums.length; i++) {
                                tsums.push( {aid:tdest[i], tsum:tsums[i]} );
                                s -= tsums[i];
                            }
                            tsums.push( {aid:targacct, tsum:s} );
                        }
                    }
                    p.tsums = tsums;
                    lst.push(p);
                }
            }
        }
        return lst;
    }

    function QIF_Import_Acct (inStr) {
        var p, doinit = true; lst=[], skip = 0, inl = inStr.split('\n');
            
        for (n=0; n<inl.length; n++) {
            var str = inl[n], ch = str.substr(0,1), rest = str.substr(1).trim();
            if (str.match(/^Type/)) continue;
            if (doinit) {
                doinit = false;
                p = {aname:'', ainstnotes:'', atype:'Expense'};
            }
            switch (ch) {
                case 'N':  p['aname'] = rest; break;
                case 'D':  p['ainstnotes'] = rest; break;
                case 'I':  p['atype'] = 'Revenue'; break;
                case 'E':  p['atype'] = 'Expense'; break;
                case 'T':
                    break;
                case '^':  { // Terminator.
                    lst.push(p);
                    doinit = true;
                }
            }
        }
        return lst;
    }

                T {
                    if {$iscat} {
                        set p(ataxed) 1
                    } else {
                        set desc $rest
                    }
                }
                ^ {
                    if {$name == {}} {
                        puts $lf "account already exists: $rest"
                    } elseif {[LookupField $_ aclist aname $name] == {}} {
                        set aid [CreateAccount $_ aname $name acatagory $iscat ainstnotes $desc]
                        lappend accts $aid
                        foreach i [array names p] {
                            $pa update $aid $i $p($i)
                        }
                        puts $lf "created account: $rest"
                    }
                    set desc {}
                    set name {}
                    array unset p
                    incr cnt
                }
