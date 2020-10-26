try {
// Jsi docs viewer
Vue.component('app-view', {template: '<router-view />' } );

var dputs = function() {};
//var dputs = console.log;

var jsiroot = window.jsiroot;
var Vuex = window.Vuex;
Vue.use(Vuex)
var Tpath = (window.Pdq?'/Docs/View/':'/');

var Top = { template:` <router-view /> `, }
var Docs = { template:` <router-view /> `, };


/*============ NAVBAR ================*/

Vue.component('jsi-nav-bar', {
    template:`
<div>
  <b-sidebar id="jsi_sidebar" bg-variant="info" title="Docs Menu">
    <b-nav vertical>
      <b-navbar-nav class="mx-2 my-0 py-0">
        <b-nav-item v-if="s_websock" link-classes="py-0 px-0 mx-0" href="#" @click="menuCmd('exit')" :disabled="false" title="Notify jsish server to exit">- Exit Jsish -</b-nav-item>
      </b-navbar-nav >
      <jsi-left-menu />
    </b-nav">
  </b-sidebar>
  
  <b-navbar  class="p-1">
    <b-navbar-nav >

      <b-nav-form class="d-inline-flex visible">
        <b-nav-item id="pdq_menubut2" :class="menuClass" style="margin-bottom:-3px" v-b-toggle.jsi_sidebar>&#9776;</b-nav-item>
        <b-button size="sm" class="my-2 my-sm-0 pdq-nav-area"><a :href="s_project.href" target="_blank" :title="s_project.title">{{s_project.label}}</a></b-button>
      </b-nav-form>
      <b-nav-item v-for="r in s_navFns" :key="r" href="#" @click="goTo(r+'.md')" :disabled="s_mdFile==r+'.md'" >{{r}}</b-nav-item>
    </b-navbar-nav>

    <b-navbar-nav class="ml-auto">
        <b-nav-form class="d-inline-flex" onsubmit="submit">
            <b-form-input id="searchbox" size="sm" :placeholder="searchPH" v-model="search" 
              list="docs-search-list" @keydown.enter="submit" :disabled="searchDis" @change="chg"  @update="chglst"/>
            <b-form-datalist id="docs-search-list" :options="s_searchList" />
            <b-button size="sm" class="my-2 my-sm-0" @click="submit" :disabled="searchButDis">Search</b-button>
          <b-button size="sm" class="w-1 bd-submenus" title="Expose-all submenus in page toggle" @click="subChange" v-html="subShowLabel"></b-button>
         </b-nav-form>
        <b-nav-form class="d-inline-flex" id="pdq-popover-target-1">
        </b-nav-form>
    </b-navbar-nav>
  </b-navbar>
</div>
    `,
    data:function() {
        return {
            Mmenushowbut:true, search:'',
        }
    },
    methods:  Object.assign(
        Vuex.mapMutations('Docs', ['update_s_showAllSub']), {
        chglst:function(evt) {
            //dputs('chglst', evt);
        },
        submit:function(evt) {
            dputs("GOT SUBMIT");
            evt.preventDefault();
            this.chg(this.search, true);
        },
        chg:function(asstr, issubmit) {
            if (!asstr) return;
            var sstr = asstr, mod = '';
            var sii = sstr.lastIndexOf(' : ');
            if (sii>0) {
                sstr = sstr.substr(0, sii);
                mod = asstr.substr(sii+3).trim();
            }
            var svals = this.s_mdIndex.sections, lsstr = sstr.toLowerCase();
            for (var pass = 1; pass<=3; pass++) {
                for (var f in svals) {
                    if (pass==1 && mod && f != mod)
                        continue;
                    var sv = svals[f];
                    for (var i = 0; i<sv.length; i+=2) {
                        var ss = sv[i+1];
                        ss = ss.substr(ss.indexOf(' ')+1).trim();
                        if (ss == sstr || (pass==2 && ss.indexOf(sstr)>=0)
                            || (pass==3 && ss.toLowerCase().indexOf(sstr)>=0)) {
                            var vr = f+'#'+sv[i];
                            vr = encodeURIComponent(vr);
                            dputs("FOUND: ", f, sv[i], vr);
                            if (this.$router.currentRoute.path !== vr) {
                                this.$router.push(Tpath+vr);
                                this.search = '';
                                return;
                            }
                        }
                    }
                }
                if (!issubmit) break;
            }
        },
        goTo:function(nam) {
            if (this.s_mdFile !== nam)
                this.$router.push(Tpath+encodeURIComponent(nam));
        },
        menuCmd:function(op){
            dputs("OP", op);
            switch (op) {
                case 'exit': this.s_websock.send(JSON.stringify({cmd:'exit'})); break;
            }
        },
        subChange:function() {
            this.update_s_showAllSub(!this.s_showAllSub);
        }
    }),
    computed:  Object.assign(
        Vuex.mapState('Docs', ['s_searchList', 's_mdIndex', 's_showAllSub', 's_mdFile', 's_login', 's_websock',
            's_navFns', 's_project']), {
        showMenu:function() { return this.s_websock }, 
        menuClass:function() { return (this.s_websock ? "" : 'smalldev'); }, 
        searchDis:function() { return (this.s_searchList.length==0); },
        searchButDis:function() { return (this.s_searchList.length==0 || this.search==''); },
        searchPH:function() {
            return (this.searchDis?'Search Unavailable':'Enter Search');
        },
        searchOptions:function() {
            return ['Able', 'Baker', 'Charlie'];
        },
        subShowLabel:function() {
            return this.s_showAllSub?'&minus;':'&plus;';
        },
    }),
});

/*============= LEFT MENU ===============*/

Vue.component('jsi-left-menu', {
    template:`
    <b-nav vertical v-if="s_mdList && s_mdList.length" class="py-0">
        <jsi-left-menu-item v-for="(r,i) in s_mdList" :key="i" :mitem="r" :first="first" :max="max" />
    </b-nav>
    `,
    props: {
        first:  { type: Number, default:1},
        max:    { type: Number, default:6},
    },
    computed:
        Vuex.mapState('Docs', ['s_mdList']),
    methods: {       
    },
});

Vue.component('jsi-left-menu-item', {
    template:`
    <li class :class="'nav-item side-entry side-h'+getLev(-1)+' pl-'+getLev(0)" :data-sideidx="mitem.idx">
        <span  @click.ctrl="selSMC" @click.shift="selSMC" @click="setSM" class="py-0" :class="{active:isActive}">{{mitem.label}}</span>
        <jsi-left-menu :mlist="mitem.children" :first="first" :max="max" v-if="mitem.children" />
    </li>
</li>
    `,
    props: {
        mitem:  { type: Object },
        first:  { type: Number, default:1},
        max:    { type: Number, default:2},
    },
    methods:  Object.assign(
    Vuex.mapMutations('Docs', ['update_s_mdFile']),
    {
        getLev:function(n) {
            return this.mitem.level+n;
        },
        selSMC:function() {  // Select item from left menu (MD document menu)
            dputs('Shift-click', this.mitem.value);
            var nrt = Tpath+encodeURIComponent(this.mitem.value);
            nrt = location.origin+location.pathname+'#'+nrt;
            window.open(nrt, '_blank');
            return false;
        },
        setSM:function() {
            dputs('Click', this.mitem.idx);
            var nrt = Tpath+encodeURIComponent(this.mitem.value);
            if (this.$router.currentRoute.path !== nrt)
                this.$router.push(nrt);
        }
    }),
    computed: Object.assign(
        Vuex.mapState('Docs', ['s_mdFile']), {
        isActive:function() {
            return (this.s_mdFile==this.mitem.value);
        },
    }),
});

/*============ RIGHT MENU ================*/

Vue.component('jsi-right-menu', {
    template:`
    <b-nav vertical v-if="mlist && mlist.length" class="py-0">
        <jsi-right-menu-item v-for="(r,i) in mlist" :key="i" :mitem="r" :first="first" :max="max" />
    </b-nav>
    `,
    props: {
        mlist:  { type: Array },
        first:  { type: Number, default:1},
        max:    { type: Number, default:6},
    },
});

Vue.component('jsi-right-menu-item', {
    template:`
    <li class :class="'nav-item toc-entry toc-h'+(mitem.level-1)+' pl-'+mitem.level" :data-tocidx="mitem.index">
        <span @click="selTM"  data-id="mitem.id" class="py-0 toc-subs nav-link" :class="{active:isActive}">{{mitem.content}}</span>
        <jsi-right-menu :mlist="mitem.children" :first="first" :max="max" v-if="tocItemVisible" />
    </li>
</li>
    `,
    props: {
        mitem:  { type: Object },
        first:  { type: Number, default:1},
        max:    { type: Number, default:2},
    },
    methods:    Object.assign(
        Vuex.mapMutations('Docs', ['update_s_curAnc', 'update_s_pickedAnc']), {
        getLev:function(n) {
            return this.mitem.level+n;
        },
        selTM:function() { // Select item from right menu (TOC)
            var vr = this.mitem.anchor;
            var nrt = Tpath+encodeURIComponent(this.s_mdFile+'#'+this.mitem.anchor);
            if (this.$router.currentRoute.path !== nrt) {
                this.update_s_pickedAnc(this.mitem.idx);
                this.update_s_curAnc(this.mitem.idx);
                dputs('SEL: ', this.mitem.idx);
                this.$router.push(nrt);
            }
            //this.$router.push(this.mitem.id.hash.substr(1));
        },
    }),
    computed:  Object.assign(
        Vuex.mapState('Docs', ['s_curAnc', 's_curAncParent', 's_ancChain', 's_mdFile', 's_showAllSub']), {
        isActive:function() {
            return (this.s_curAnc==this.mitem.idx || this.s_ancChain.indexOf(this.mitem.idx)>=0);
        },
        tocItemVisible:function() {
            var mitem = this.mitem;
            if (!mitem.children)
                return false;
            if (this.s_showAllSub)
                return true;
            var ca = this.s_curAnc, cp = this.s_curAncParent,
                idx = mitem.idx, s_ancChain=this.s_ancChain, mp = mitem.parent
            return (mitem.level==1 || idx==ca || idx == cp || (mp && s_ancChain.indexOf(mp.idx)>0) || s_ancChain.indexOf(idx)>0);
        },
    }),
});

Vue.component('jsi-right', {
    template:`
<div>
   <div id="jsi-toc">
        <jsi-right-menu :mlist="s_mdToc"></jsi-right-menu>
    </div>
</div>
    `,
    computed:
        Vuex.mapState('Docs', ['s_mdToc'])
    ,
});


/*============ MAIN ================*/

var DocsView = {
    template:`
<b-container fluid>
  <b-row rows="1" class="bg-navbar bd-navbar">
        <b-col>
            <jsi-nav-bar />
        </b-col>
    </b-row>
    <b-row rows="11">
        <b-col cols="0" md="1" class="bd-sidebar" style="padding-left:0">
            <jsi-left-menu></jsi-left-menu>
        </b-col>
        <b-col cols="12" md="9" class="p-0">
            <div style="height:100%; overflow-y:auto" id="markitup2">
                <vue-markdown-pdq id="markitup" ref="markitup"
                    class="markdown" style="position:relative"
                    toc :source="src"
                    :sub-opts="subOpts" 
                    @toc-rendered="tocRendered" @rendered="rendered"
                    :anchor-attrs="anchorAttrs"
                    langPrefix="language language-"       
                />
            </div>
            
        </b-col>
        <b-col  cols="0" md="2" class="bd-toc p-0">
            <jsi-right></jsi-right>
        </b-col>
    </b-row>
</b-container>
    `,
    props: {
        path: { path:{type:String, default:''} }
    },
    data:function() {
        return {
            srcIdx:'', fetchStatIdx:500, fetchStatText:'', srcDir:'', fetchStatDir:500,
            src:'', fetchStat:200, ancs:null, hash:null, revLookup:null, curRoute:'', tocArr:null, 
            anchorAttrs: { target:'_blank', rel:'noopener noreferrer nofollow' },
            subOpts:{
                toc: {anchorLinkBefore:false, anchorLinkSpace:false},
            },
        }
    },
    mounted:function() {
        this.loadIndexFile();
        this.doLoadFile(this.$route.params.path, this.$route.hash);
    },
    beforeRouteUpdate:function(to, from, next) {
        dputs("TO", to, from);
        if (!to.params.path && from.params.path)
            return;
        this.doLoadFile(to.params.path, to.hash);
        next();
    }, 
    created:function() { window.addEventListener('scroll', this.handleScroll); },
    destroyed:function() { window.removeEventListener('scroll', this.handleScroll); },
    computed:
        Vuex.mapState('Docs', ['s_dirty', 's_mdFile', 's_mdDir', 's_curAnc', 's_ancChain', 
            's_mdAncs', 's_fileMod', 's_mdList', 's_server', 's_isfossil', 's_pickedAnc'])
    ,
    methods: Object.assign(
        Vuex.mapMutations('Docs', ['update_s_dirty', 'update_s_mdToc', 'update_s_mdAncs', 'update_s_curAnc',
            'update_s_ancChain', 'update_s_curAncParent', 'update_s_mdFile', 'update_s_fileMod',
            'update_s_mdList', 'update_s_mdIndex', 'update_s_searchList', 'update_s_server',
            'update_s_project', 'update_s_navFns', 'update_s_pickedAnc']), {
        doLoadFile:function(path, hash, from) {
            var cr = this.$route;
            this.hash = hash;
            this.curRoute = path;
            dputs('CURROUTE', path);
            var hi, fpath = (path?decodeURIComponent(path):null);
            if (!fpath) {
                if (!this.s_mdList.length) return;
                fpath = this.s_mdList[0].value;
            } else if (path == '' || path == null) {
                 this.$router.push(Tpath+fpath);
                 return;
            } else if ((hi=fpath.lastIndexOf('#'))>0) {
                this.hash = fpath.substr(hi);
                fpath = fpath.substr(0, hi);
            }
            dputs('HASH', this.hash);
            dputs('LL', fpath);
            var dirty = this.s_dirty;
            this.update_s_dirty(false);
            if (fpath === this.s_mdFile && this.fetchStat==200 && !dirty)
                this.loadFinish(false);
            else if (fpath.substr(fpath.length-3)=='.md') {
                this.update_s_mdFile(fpath);
                this.loadMdFile();
            }
        },
        loadMdFile:function() {
            var url = this.s_mdDir+this.s_mdFile, that = this;
            dputs("FETCH", url);
            url += '?mimetype=text/markdown';
            Jsish.ajax({url:url,
                success:function(data, okstr, req) {
                    that.fetchStat = 200;
                    that.tocArr = null;
                    that.src = data;
                    that.loadFinish(true)
                },
                error:function(e) {
                    that.fetchStat = e.status;
                    //that.fetchStatText = e.statusText;
                    that.loadFinish(true);
                },
            });
        },
        loadFinish:function(doHi) {
            puts('loadFinish', doHi);
            if (this.fetchStat != 200 || !this.src || (histmode && this.src.substr(0,1)=='<!')) {
                this.src = "PAGE NOT FOUND";
                return;
            }
            if (this.hash && this.revLookup) {
                var hn = this.hash.substr(1);
                var id = this.revLookup[hn], scrbh = 'auto';
                dputs('scrlen', this.src.length);
                if (this.src.length<11000) scrbh = 'smooth';
                if (!id)
                    id = this.revLookup[hn.toLowerCase()];
                if (id)
                    setTimeout(function() {
                        //id.scrollIntoView(true);
                        var yOffset = -50; 
                        var element = id; // document.getElementById(id);
                        var y = element.getBoundingClientRect().top + window.pageYOffset + yOffset;
                        window.scrollTo({top: y, behavior: scrbh});
                    }, 500);
            } else {
                window.scrollTo(0,0);
            }
            this.setTitle();
        },
        setTitle:function() {
            var t = this.s_mdFile;
            if (t.substr(t.length-3) == '.md')
                t = t.substr(0, t.length-3);
            document.title = t+': Jsi-Docs';
        },
        rendered:function() {
            dputs('@rendered');
            Prism.highlightAll();
            this.tocUpdate();
        },
        tocRendered:function(tocHtml, tocMarkdown, arr) {
            dputs('@toc-rendered');
            if (this.tocArr != null) {
                 return;
            }
            this.tocArr = arr;
        },
        tocUpdate:function() {
            // Pull entries from body.
            var arr = this.tocArr;
            if (!arr || !arr.length) return;
           // var l = document.querySelectorAll('a[class=toc-anchor]')
            var ll = {}
            var l = document.querySelectorAll('#markitup a.toc-anchor');
            this.ancs = l;
            for (var k = 0; k<l.length; k++) {
                ll[l[k].hash.substr(1)] = l[k];
            }
            this.revLookup = ll;
            dputs("toc CALLBACK");
            
            var bb = arr;
            function pushHdrs(cur, parent) {
                var res = [], i;
                while (idx<bb.length && bb[idx].level>=cur) {
                    i = bb[idx];
                    i.idx = idx;
                    var ni = bb[++idx];
                    i.children =  (ni && ni.level>cur)? pushHdrs(cur+1, i) : null;
                    i.parent = parent;
                    i.id = ll[i.anchor];
                    res.push(i);
                }
                return res;
            }
            
            var idx = 0;
            var sub = pushHdrs(1, null),
                nsub = (sub && sub.length==1 && sub[0].children?sub[0].children:sub);
            this.update_s_mdToc(nsub);
            this.update_s_mdAncs(arr);
            this.setupAncChain(1);
            this.HrefToRouterPush();

            dputs('res',sub);
        },
        HrefToRouterPush:function() {
            // Convert hrefs to router.push.
            var l = document.querySelectorAll('#markitup a');
            for (var i=0; i<l.length; i++) {
                var href = l[i].getAttribute('href');
                if (href.indexOf(':')>=0) continue;
                var buri = l[i].baseURI.split('#')[0];
                //var cl = l[i].classList;
                //if (cl && cl.contains('toc-anchor')) continue;
                var that = this;
                var vr = href;
                if (href[0] == '#')
                    vr = this.s_mdFile+href;
                vr = encodeURIComponent(vr);
                l[i].onclick=function() {
                    that.$router.push(Tpath+ vr);
                    return false;
                }
            }
        },
        setupAncChain:function(c) { // Update s_ancChain on scroll.
            function getAnc(t, depth) {
                if (depth>100) throw("bad anchor list");
                for (var i = 0; i<t.length; i++)
                    if (t[i].idx === c) return t[i];
                for (var i = 0; i<t.length; i++) {
                    if (t[i].children && c > t[i].idx && ((i==(t.length-1) || c < t[i+1].idx)))
                        return getAnc(t[i].children, depth+1);
                }
                return null;
            }

            var ac = [], toc = this.s_mdAncs, ta = getAnc(toc, 0);
            this.update_s_curAnc(c);
            this.update_s_curAncParent((ta && ta.parent)?ta.parent.idx:-1);
            while (ta && ta.parent && ta.parent.idx>=0 && ac.length<100) {
                ta = ta.parent;
                ac.unshift(ta.idx);
            }
            this.update_s_ancChain(ac);
            //this.scrollTocView(c)
        },
        handleScroll:function(ev) { // Calcuate current header in scrolled window.
            var current;
            if ((current=this.s_pickedAnc)>=0) {
                dputs('picked');
                this.update_s_pickedAnc(-1);
            } else {
                var sections = this.ancs, sl;
                if (!sections || !(sl=sections.length)) return;
                var secs = Array.from(sections).reverse();
                //if (secs[0].offsetTop;
                dputs('OT', window.scrollY, window.innerHeight, secs[0].offsetTop);
                if ((window.scrollY+window.innerHeight) >= secs[0].offsetTop) {
                    current = sl-1;
                } else {
                    var srvl = secs.findIndex(function(s) { return (window.scrollY >= (s.offsetTop - 1)); } ) 
                    current = sl - srvl - 1;
                    if (current<1) current = 1; //TODO: ignoring H1
                    dputs('hscroll', current, this.s_curAnc, srvl, sl);
                }
                if (current == this.s_curAnc)
                    return;
            }
            this.setupAncChain(current);
        },
        getTocList:function(id, first, last) {
            var ii, i, ip, aa = [], bb = [], cc={}, aids = document.querySelectorAll('#markitup a[class=toc-anchor]'),
                n, min = 0, max = 0, cur, anchorName, hlst = 'h1,h2,h3,h4,h5,h6'.split(','), lt = this.lancText;
            if (!first) first = 1;
            if (!last) last = 6;
            //return;
            for (ii = 0; ii<aids.length; ii++) {
                i = aids[ii];
                ip = i.parentElement;
                if (!ip || !ip.nodeName || hlst.indexOf(ip.nodeName.toLowerCase())<0) continue;
                n = parseInt(ip.nodeName[1]);
                if (n<first || n>last) continue;
                if (!min || n<min) min = n;
                if (!max || n>max) max = n;
                var label = ip.innerText.trim(); //escapeHtml(ip.innerText.trim(), 1);
                if (this.tocAnchorBefore) {
                    if (lt.length && label.substr(0,lt.length) == lt)
                        label = label.substr(lt.length).trim();
                } else {
                    if (lt.length && label.substr(label.length-lt.length,lt.length) == lt)
                        label = label.substr(0, label.length-lt.length).trim();
                }
                aa.push({id:aids[ii], level:n, label:label});
                cc[i.hash] = i;
            }
            this.revLookup = cc;
            cur = min;
            for (ii = 0; ii<aa.length; ii++) {
                i = aa[ii];
                while (i.level>(cur+1)) {
                    i = {id:null, level:i.level+1, label:'#'};
                    bb.push(i);
                    if (bb.length>1000)
                        throw("toc far too big", bb.length);
                }
                cur = aa[ii].level;
                bb.push(aa[ii]);
                if (bb.length>1000)
                    throw("toc way too big", bb.length);
            }
        
            function pushHdrs(cur, parent) {
                var res = [], i;
                while (idx<bb.length && bb[idx].level>=cur) {
                    i = bb[idx];
                    i.idx = idx;
                    var ni = bb[++idx];
                    i.children =  (ni && ni.level>cur)? pushHdrs(cur+1, i) : null;
                    i.parent = parent;
                    res.push(i);
                }
                return res;
            }
            
            var idx = 0;
            var res = pushHdrs(min, null);
            return [res, bb];
        },
        loadIndexDir:function() {
            var url = this.s_mdDir + '?callback=null', that = this;
            Jsish.ajax({url:url,
                success:function(data, okstr, req) {
                    that.fetchStatDir = 200;
                    that.srcDir = data;
                    that.loadFinishDir(true)
                },
                error:function(e) {
                    that.fetchStatDir = e.status;
                    //that.fetchStatText = e.statusText;
                    that.loadFinishDir(true);
                },
            });        
        },
        loadFinishDir:function(done) {
            if (this.fetchStatDir != 200) {
                dputs("Failed to load dir");
                return;
            }
            dputs('DIR', this.srcDir);
            var src = JSON.parse(this.srcDir), mdIn =[];
            for (var i of src)
                if (i.type == 'file' && i.name.substr(i.name.length-3)=='.md')
                    mdIn.push(i.name);
            this.loadRightMenu(mdIn);
        },
        loadIndexFile:function(fn) {
            var url = this.s_mdDir + 'index.json?mimetype=text/markdown';
            dputs("FETCH INDEX");
            var that = this;
            Jsish.ajax({url:url,
                success:function(data, okstr, req) {
                    that.fetchStatIdx = 200;
                    that.srcIdx = data;
                    that.loadFinishIdx(true)
                    if (!that.s_server)
                        that.update_s_server(req.getResponseHeader('SERVER'));
                },
                error:function(e) {
                    that.fetchStatIdx = e.status;
                    that.fetchStatText = e.statusText;
                    that.loadFinishIdx(true);
                },
            });
        },
        loadFinishIdx:function(done) {
            dputs('loadFinishIdx');
            if (this.fetchStatIdx != 200) {
                dputs("Failed to load index.json");
                if (!this.s_isfossil && !this.srcDir)
                    this.loadIndexDir();
                return;
            }
            var src = JSON.parse(this.srcIdx);
            this.loadRightMenu(src.files);
            this.update_s_mdIndex(src);
            var slst = [], svals = src.sections;
            if (src.project)
                this.update_s_project(src.project);
            if (src.navFns)
                this.update_s_navFns(src.navFns);
            for (var f in svals) {
                var sv = svals[f];
                for (var i = 0; i<sv.length; i+=2) {
                    var ss = sv[i+1];
                    ss = ss.substr(ss.indexOf(' ')+1).trim();
                    ss += ' : '+f;
                    slst.push(ss);
                }
            }
            this.update_s_searchList(slst);
        },
        loadRightMenu:function(mdIn) {
            var res = [];
            for (var i in mdIn) {
                var mdi = mdIn[i];
                res.push({value:mdi, label:this.getRoot(mdi), level:1, idx:i, children:null});
            }
            /*
            var res = [], mdIn = this.src.trim().split(' ');
            if (!mdIn.length) return;
            for (var i in mdIn) {
                var mdi = mdIn[i].trim();
                res.push({value:mdi, label:this.getRoot(mdi), level:1, idx:i, children:null});
            }*/
            this.update_s_mdList(res);
            if (this.s_mdFile)
                return;
            this.update_s_mdFile(mdIn[0]);
            this.update_s_dirty(true);
            var nrt = Tpath+encodeURIComponent(mdIn[0]);
            if (this.$router.currentRoute.path !== nrt)
                this.$router.push(nrt);
        },
        getRoot:function(value) {
            var x = value.lastIndexOf('.');
            if (x>0)
                value = value.substr(0,x);
            return value;
        },

    }),
    watch: {
        s_fileMod:function() {
            if (!this.s_fileMod) return;
            dputs('Filemod', this.s_fileMod, this.s_mdFile);
            var fm = this.s_fileMod;
            this.update_s_fileMod(null);
            if (fm.indexOf('/'+this.s_mdFile)<0) return;
            this.update_s_dirty(true);
            this.loadMdFile();
        },
    },
};

/*============= CODE ===============*/

var isfossil = Jsish.isfossil();
var histmode = Jsish.config.histmode;
var mdDir = (histmode ? './md/':'../md/');

// Following not used right now.
var sess = Jsish.getCookie('sessionJsi'), iflags = 0, uflags = 0;
if (sess && !isfossil) {
    var ssp = sess.split('.');
    iflags = parseInt(ssp[1]); // Jsish flags
    uflags = parseInt(ssp[2]); // User flags
}

var store = new Vuex.Store({
    state: {},
    mutation: {},
});

var DocsStore = {
    //strict:true,
    namespaced:true,
    state: {
        s_mdDir:mdDir,
        s_mdFile:null,
        s_mdList:[],
        s_mdToc:null,
        s_mdAncs:null,
        s_mdIndex:null,
        s_pickedAnc:-1,
        s_login:'',
        s_searchList:[],
        s_server:null,
        s_websock:null,
        s_isfossil:isfossil,
        s_isjsi:Jsish.isjsi(),
        s_curAnc:0,
        s_curAncParent:-1,
        s_ancChain:[],
        s_fileMod:null,
        s_dirty:false,
        s_showAllSub:false,
        s_navFns:[],
        s_project:{ label:'JSI', title:"home page jsish.org", href:"https://jsish.org" },
        

    },
    mutations: {
        update_s_pickedAnc:function(state, val)   { state.s_pickedAnc = val; },
        update_s_mdFile:function(state, val)      { state.s_mdFile = val; },
        update_s_mdToc:function(state, val)       { state.s_mdToc = val; },
        update_s_mdAncs:function(state, val)      { state.s_mdAncs = val; },
        update_s_curAnc:function(state, val)      { state.s_curAnc = val; },
        update_s_mdList:function(state, val)      { state.s_mdList = val; },
        update_s_mdIndex:function(state, val)     { state.s_mdIndex = val; },
        update_s_navFns:function(state, val)      { state.s_navFns = val; },
        update_s_project:function(state, val)      { state.s_project = val; },
        update_s_searchList:function(state, val)  { state.s_searchList = val; },
        update_s_server:function(state, val)      { state.s_server = val; },
        update_s_websock:function(state, val)     { state.s_websock = val; },
        update_s_ancChain:function(state, val)    { state.s_ancChain = val; },
        update_s_curAncParent:function(state, val){ state.s_curAncParent = val; },
        update_s_fileMod:function(state, val)     { state.s_fileMod = val; },
        update_s_dirty:function(state, val)       { state.s_dirty = val; },
        update_s_showAllSub:function(state, val)  { state.s_showAllSub = val; },
    },
};


store.registerModule('Docs', DocsStore);


var rconf = {
  routes: [{ path:'/:path?', name:'/',  component:DocsView}, { path:'*', redirect:'/' } ]
};

if (Tpath != '/') {
    var childlst = [ 
        {path: 'Docs', name:'Docs', redirect:'Docs/View', component:Docs,
            children: [
                {path: 'View/:path?', name:'Docs.View', component: DocsView },
            ]
        }
    ];
    rconf = {
      routes: [ { path:'/', name:'/', redirect:'/Docs', component:Top, children:childlst,},
      { path:'*', redirect:'/Docs', component:Top } ]
    };
}

if (histmode)
    rconf.mode = 'history';

var router = new VueRouter( rconf );



/*var originalPush = VueRouter.prototype.push;
VueRouter.prototype.push = function push(location) {
  return originalPush.call(this, location).catch(err => {
    if (err.name !== 'NavigationDuplicated') throw err
  });
}*/

Vue.use(VueMarkdownPdq);
var vm = new Vue({
        store:store,
        router:router,
      }).$mount('#app');

var ws = Jsish.websock({
    onchange:function(fname, extn) {
        // If jsish server, handle notify on file change.
        if (extn == 'md')
            store.commit('Docs/update_s_fileMod', fname);
        else
            location.reload();
    },
    onopen:function(msg) {
        dputs("OPENED", msg);
        ws.send('{"cmd":"start"}');
    },
    onrecv:function(msg) {
        dputs("ONMSG", msg);
        switch (msg.cmd) {
            case 'start':
                dputs('STARTED');
                store.commit('Docs/update_s_websock', ws);
                break;
            case 'exit':
                 dputs("exit done");
                 store.commit('Docs/update_s_websock', null);
                 break;
        }
    },
});

Jsish.css('app.css');

} catch(e) {
    document.getElementById('app').innerText= 'ERROR (redirecting in 8 secs): '+e;
    setTimeout(function() { location.href = 'https://jsish.org/docs/'; }, 8000);
    
}
