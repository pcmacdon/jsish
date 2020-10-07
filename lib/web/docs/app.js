// Jsi docs viewer
import Vuex from '../js/vuex.js';
Vue.use(Vuex)

let Top = {
    template:`
    <div>
      <b-sidebar id="jsi_sidebar" bg-variant="info" title="Menu">
        <b>Hello</b>
      </b-sidebar>
      <router-view />
    </div>
    `,
}

let Docs = {
    template:`
    <router-view />
    `,
    data() {
        return {src:'', fetchStat:500, srcDir:'', fetchStatDir:500}
    },
    mounted() {
        this.loadIndexFile();
        let that = this;
        //setTimeout(function() { that.loadIndexDir(); }, 1000);
    },
    computed: {
    ...Vuex.mapState('Docs', ['s_mdDir', 's_mdFile', 's_server', 's_fossil']),
    },
    methods: {
        ...Vuex.mapMutations('Docs', ['update_s_dirty', 'update_s_mdList', 'update_s_mdFile', 
            'update_s_mdIndex', 'update_s_searchList', 'update_s_server']),
        loadIndexDir() {
            let url = this.s_mdDir + '?callback=null';
            fetch(url).
                then(response => { this.fetchStatDir = response.status; return response.text() }).
                then(result => { this.srcDir = result; }).
                then(p => this.loadFinishDir(true));
        },
        loadFinishDir(done) {
            if (this.fetchStatDir != 200) {
                puts("Failed to load dir");
                return;
            }
            puts('DIR', this.srcDir);
            let src = JSON.parse(this.srcDir), mdIn =[];
            for (var i of src)
                if (i.type == 'file' && i.name.substr(i.name.length-3)=='.md')
                    mdIn.push(i.name);
            this.loadRightMenu(mdIn);
        },
        loadIndexFile(fn) {
            let url = this.s_mdDir + 'index.json?mimetype=text/markdown';
            puts("FETCH INDEX");
            fetch(url).
                then(response => { 
                    this.fetchStat = response.status;
                    if (!this.s_server)
                        this.update_s_server(response.headers.get('SERVER'));
                    return response.text() }).
                then(result => { this.src = result; }).
                then(p => this.loadFinish(true));
        },
        loadFinish(done) {
            if (this.fetchStat != 200) {
                puts("Failed to load index.json");
                if (!this.s_fossil && !this.srcDir)
                    this.loadIndexDir();
                return;
            }
            let src = JSON.parse(this.src);
            this.loadRightMenu(src.files);
            this.update_s_mdIndex(src);
            let slst = [], svals = src.sections;
            for (var f in svals) {
                let sv = svals[f];
                for (var i = 0; i<sv.length; i+=2) {
                    let ss = sv[i+1];
                    ss = ss.substr(ss.indexOf(' ')+1).trim();
                    ss += ' : '+f;
                    slst.push(ss);
                }
            }
            this.update_s_searchList(slst);
        },
        loadRightMenu(mdIn) {
            let res = [];
            for (let i in mdIn) {
                let mdi = mdIn[i];
                res.push({value:mdi, label:this.getRoot(mdi), level:1, idx:i, children:null});
            }
            /*
            let res = [], mdIn = this.src.trim().split(' ');
            if (!mdIn.length) return;
            for (let i in mdIn) {
                let mdi = mdIn[i].trim();
                res.push({value:mdi, label:this.getRoot(mdi), level:1, idx:i, children:null});
            }*/
            this.update_s_mdList(res);
            if (this.s_mdFile)
                return;
            this.update_s_mdFile(mdIn[0]);
            this.update_s_dirty(true);
            let nrt = '/Docs/View/'+encodeURIComponent(mdIn[0]);
            if (this.$router.currentRoute.path !== nrt)
                this.$router.push(nrt);
        },
        getRoot(value) {
            let x = value.lastIndexOf('.');
            if (x>0)
                value = value.substr(0,x);
            return value;
        },

    },
};



/*============================*/

Vue.component('jsi-nav-bar', {
    template:`
<div>
  <b-navbar  class="p-1">
    <b-navbar-nav >
      <b-nav-form class="d-inline-flex">
        <b-nav-item v-if="Mmenushowbut" id="pdq_menubut2" class="" style="margin-bottom:-3px" v-b-toggle.jsi_sidebar>&#9776;</b-nav-item>
        <b-button size="sm" class="my-2 my-sm-0">JSI</b-button>
        <!-- <b-nav-item href="#/Docs/View/Start.md" class="text-dark">Start</b-nav-item>
        <b-nav-item href="#/Docs/View/Reference.md" class="text-dark">Reference</b-nav-item>-->
      </b-nav-form>
    </b-navbar-nav>

    <b-navbar-nav class="ml-auto">
        <b-nav-form class="d-inline-flex">
          <b-form onsubmit="submit">
            <b-form-input id="searchbox" size="sm" :placeholder="searchPH" v-model="search" 
              list="docs-search-list" @keydown.enter="submit" :disabled="searchDis" @change="chg"  @update="chglst"/>
            <b-form-datalist id="docs-search-list" :options="s_searchList" />
            <b-button size="sm" class="my-2 my-sm-0" @click="submit" :disabled="searchButDis">Search</b-button>
          </b-submit>
          <b-button size="sm" class="w-1" title="Toggle showing of all or only active submenus" @click="subChange" v-html="subShowLabel"></b-button>
         </b-nav-form>
        <b-nav-form class="d-inline-flex" id="pdq-popover-target-1">
        </b-nav-form>
    </b-navbar-nav>
  </b-navbar>
</div>
    `,
    data() {
        return {
            Mmenushowbut:true, search:'',
        }
    },
    methods: {
        chglst(evt) {
            puts('chglst', evt);
        },
        submit(evt) {
            puts("GOT SUBMIT");
            evt.preventDefault();
            this.chg(this.search, true);
        },
        chg(sstr, issubmit) {
            if (!sstr) return;
            let sii = sstr.lastIndexOf(' : ');
            if (sii>0)
                sstr = sstr.substr(0, sii);
            let svals = this.s_mdIndex.sections, lsstr = sstr.toLowerCase();
            for (var pass = 1; pass<=3; pass++) {
                for (var f in svals) {
                    let sv = svals[f];
                    for (var i = 0; i<sv.length; i+=2) {
                        let ss = sv[i+1];
                        ss = ss.substr(ss.indexOf(' ')+1).trim();
                        if (ss == sstr || (pass==2 && ss.indexOf(sstr)>=0)
                            || (pass==3 && ss.toLowerCase().indexOf(sstr)>=0)) {
                            let vr = f+'#'+sv[i];
                            vr = encodeURIComponent(vr);
                            puts("FOUND: ", f, sv[i], vr);
                            if (1 && this.$router.currentRoute.path !== vr) {
                                this.$router.push('/Docs/View/'+vr);
                                this.search = '';
                                return;
                            }
                        }
                    }
                }
                if (!issubmit) break;
            }
        },
        subChange() {
            this.update_s_showAllSub(!this.s_showAllSub);
        },
        ...Vuex.mapMutations('Docs', ['update_s_showAllSub']),
    },
    computed: {
        ...Vuex.mapState('Docs', ['s_searchList', 's_mdIndex', 's_showAllSub']),
        searchDis() { return (this.s_searchList.length==0); },
        searchButDis() { return (this.s_searchList.length==0 || this.search==''); },
        searchPH() {
            return (this.searchDis?'Search Unavailable':'Enter Search');
        },
        searchOptions() {
            return ['Able', 'Baker', 'Charlie'];
        },
        subShowLabel() {
            return this.s_showAllSub?'&minus;':'&plus;';
        },
    }
});

/*============================*/

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
    computed: {
        ...Vuex.mapState('Docs', ['s_mdList']),
    },
    methods: {       
    },
});

Vue.component('jsi-left-menu-item', {
    template:`
    <li class :class="'nav-item side-entry side-h'+getLev(-1)+' pl-'+getLev(0)" :data-sideidx="mitem.idx">
        <span  @click.ctrl="setSMC" @click.shift="setSMC" @click="setSM" class="py-0 side-subs nav-link" :class="{active:isActive}">{{mitem.label}}</span>
        <jsi-left-menu :mlist="mitem.children" :first="first" :max="max" v-if="mitem.children && getDisplay" />
    </li>
</li>
    `,
    props: {
        mitem:  { type: Object },
        first:  { type: Number, default:1},
        max:    { type: Number, default:2},
    },
    methods: {
        getLev(n) {
            return this.mitem.level+n;
        },
        setSMC() {
            puts('SETSMC');
            let nrt = '/Docs/View/'+encodeURIComponent(this.mitem.value);
            nrt = location.origin+location.pathname+'#'+nrt;
            puts('nrt', nrt);
            window.open(nrt, '_blank');
            return false;
        },
        setSM() {
            puts('SETSM', this.mitem.idx);
            let nrt = '/Docs/View/'+encodeURIComponent(this.mitem.value);
            if (this.$router.currentRoute.path !== nrt)
                this.$router.push(nrt);
        },
        ...Vuex.mapMutations('Docs', ['update_s_mdFile']),
    },
    computed: {
        ...Vuex.mapState('Docs', ['s_mdFile']),
        isActive() {
            return (this.s_mdFile==this.mitem.value);
        },
        getDisplay() { return true; },
    },
});

/*============================*/

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
        <span @click="setTM"  data-id="mitem.id" class="py-0 toc-subs nav-link" :class="{active:isActive}">{{mitem.content}}</span>
        <jsi-right-menu :mlist="mitem.children" :first="first" :max="max" v-if="mitem.children && getDisplay" />
    </li>
</li>
    `,
    props: {
        mitem:  { type: Object },
        first:  { type: Number, default:1},
        max:    { type: Number, default:2},
    },
    methods: {
        getLev(n) {
            return this.mitem.level+n;
        },
        setTM() {
            let vr = this.mitem.anchor;
            let nrt = '/Docs/View/'+encodeURIComponent(this.s_mdFile+'#'+this.mitem.anchor);
            if (this.$router.currentRoute.path !== nrt)
                this.$router.push(nrt);
            //this.$router.push(this.mitem.id.hash.substr(1));
        },
    },
    computed: {
        ...Vuex.mapState('Docs', ['s_curAnc', 's_curAncParent', 's_ancChain', 's_mdFile', 's_showAllSub']),
        isActive() {
            return (this.s_curAnc==this.mitem.idx || this.s_ancChain.indexOf(this.mitem.idx)>=0);
        },
        getDisplay() {
            if (this.s_showAllSub)
                return true;
            let ca = this.s_curAnc, cp = this.s_curAncParent, mitem = this.mitem,
                idx = mitem.idx, s_ancChain=this.s_ancChain, mp = mitem.parent
            return (mitem.level==1 || idx==ca || idx == cp || (mp && s_ancChain.indexOf(mp.idx)>0) || s_ancChain.indexOf(idx)>0);
            if (mitem.level==1 || idx==ca || idx == cp) {
                //puts("TRIVIAL:", idx, ca, cp);
                return true;
            }
            if (mp && s_ancChain.indexOf(mp.idx)>0) {
                //puts("CHAIN:", idx, ca, cp);
                return true;
            }
            if (s_ancChain.indexOf(idx)>0) {
                //puts("CHAIN2:", idx, ca, cp);
                return true;
            }
            //puts("NO DISPLAY:", idx, ca, cp);
            return false;
        },
    },
});

Vue.component('jsi-right', {
    template:`
<div>
   <div id="jsi-toc">
        <jsi-right-menu :mlist="s_mdToc"></jsi-right-menu>
    </div>
</div>
    `,
    computed: {
        ...Vuex.mapState('Docs', ['s_mdToc']),
    },
});



let DocsView = {
    template:`
<b-container fluid>
  <b-row rows="1" class="bg-navbar bd-navbar">
        <b-col>
            <jsi-nav-bar></jsi-nav-bar>
        </b-col>
    </b-row>
    <b-row rows="11">
        <b-col cols="1" class="bd-sidebar" style="padding-left:0">
            <jsi-left-menu></jsi-left-menu>
        </b-col>
        <b-col cols="9" class="p-0">
            <div style="height:100%; overflow-y:auto" id="markitup2">
                <vue-markdown-pdq id="markitup" ref="markitup"
                    class="markdown" style="position:relative"
                    toc :source="src"
                    :sub-opts="subOpts" @toc-rendered="tocRendered"
                    :anchor-attrs="anchorAttrs"
                    langPrefix="language language-"       
                />
            </div>
            
        </b-col>
        <b-col cols="2" class="bd-toc p-0">
            <jsi-right></jsi-right>
        </b-col>
    </b-row>
</b-container>
    `,
    props: {
        path: { path:{type:String, default:''} }
    },
    data() {
        return {
            src:'', fetchStat:200, ancs:null, hash:null, revLookup:null, curRoute:'', tocArr:null, 
            anchorAttrs: { target:'_blank', rel:'noopener noreferrer nofollow' },
            subOpts:{
                toc: {anchorLinkBefore:false, anchorLinkSpace:false},
            },
        }
    },
    mounted() {
        this.doLoadFile(this.$route.params.path, this.$route.hash);
    },
    beforeRouteUpdate (to, from, next) {
        puts("TO", to, from);
        if (!to.params.path && from.params.path)
            return;
        this.doLoadFile(to.params.path, to.hash);
        next();
    }, 
    created () { window.addEventListener('scroll', this.handleScroll); },
    destroyed () { window.removeEventListener('scroll', this.handleScroll); },
    computed: {
        ...Vuex.mapState('Docs', ['s_dirty', 's_mdFile', 's_mdDir', 's_curAnc', 's_ancChain', 's_mdAncs', 's_fileMod', 's_mdList']),
    },
    methods: {
       ...Vuex.mapMutations('Docs', ['update_s_dirty', 'update_s_mdToc', 'update_s_mdAncs', 'update_s_curAnc',
            'update_s_ancChain', 'update_s_curAncParent', 'update_s_mdFile', 'update_s_fileMod']),

        doLoadFile(path, hash, from) {
            let cr = this.$route;
            this.hash = hash;
            this.curRoute = path;
            puts('CURROUTE', path);
            let hi, fpath = (path?decodeURIComponent(path):null);
            if (!fpath) {
                if (!this.s_mdList.length) return;
                fpath = this.s_mdList[0].value;
            } else if (path == '' || path == null) {
                 this.$router.push('/Docs/View/'+fpath);
                 return;
            } else if ((hi=fpath.lastIndexOf('#'))>0) {
                this.hash = fpath.substr(hi);
                fpath = fpath.substr(0, hi);
            }
            if (path && hash === undefined && cr.path) {// Initial load: we'll be called back by beforeRouteUpdate
                //this.$router.push('/Docs/View/'+fpath);
                //return;
            }
            puts('HASH', hash);
            puts('LL', fpath);
            let dirty = this.s_dirty;
            this.update_s_dirty(false);
            if (fpath === this.s_mdFile && !dirty)
                this.loadFinish(false);
            else {
                this.update_s_mdFile(fpath);
                this.loadMdFile();
            }
        },
        loadMdFile() {
            let url = this.s_mdDir+this.s_mdFile;
            puts("FETCH", url);
            url += '?mimetype=text/markdown';
            fetch(url).
                then(response => { this.fetchStat = response.status; return response.text()} ).
                then(result => { this.src = result; }).
                then(p => this.loadFinish(true));
        },
        loadFinish(doHi) {
            if (this.fetchStat != 200)
                this.src = "PAGE NOT FOUND";
            if (doHi) {
                this.tocUpdate();
                Prism.highlightAll();
            }
            if (this.hash) {
                let hn = this.hash.substr(1);
                var id = this.revLookup[hn], scrbh = 'auto';
                puts('scrlen', this.src.length);
                if (this.src.length<11000) scrbh = 'smooth';
                if (!id)
                    id = this.revLookup[hn.toLowerCase()];
                if (id)
                    setTimeout(function() {
                        //id.scrollIntoView(true);
                        const yOffset = -50; 
                        const element = id; // document.getElementById(id);
                        const y = element.getBoundingClientRect().top + window.pageYOffset + yOffset;
                        window.scrollTo({top: y, behavior: scrbh});
                    }, 500);
            } else {
                window.scrollTo(0,0);
            }
            this.setTitle();
        },
        setTitle() {
            let t = this.s_mdFile;
            if (t.substr(t.length-3) == '.md')
                t = t.substr(0, t.length-3);
            document.title = t+': Jsi-Docs';
        },
        tocRendered(tocHtml, tocMarkdown, arr) {
            this.tocArr = arr;
        },
        tocUpdate() {
            let arr = this.tocArr;
            if (!arr || !arr.length) return;
           // let l = document.querySelectorAll('a[class=toc-anchor]')
            let ll = {}
            let l = document.querySelectorAll('#markitup a.toc-anchor');
            this.ancs = l;
            for (let k = 0; k<l.length; k++) {
                ll[l[k].hash.substr(1)] = l[k];
            }
            this.revLookup = ll;
            puts("toc CALLBACK");
            
            let bb = arr;
            function pushHdrs(cur, parent) {
                let res = [], i;
                while (idx<bb.length && bb[idx].level>=cur) {
                    i = bb[idx];
                    i.idx = idx;
                    let ni = bb[++idx];
                    i.children =  (ni && ni.level>cur)? pushHdrs(cur+1, i) : null;
                    i.parent = parent;
                    i.id = ll[i.anchor];
                    res.push(i);
                }
                return res;
            }
            
            let idx = 0;
            let sub = pushHdrs(1, null);
            this.update_s_mdToc(sub && sub.length==1 && sub[0].children?sub[0].children:sub);
            this.update_s_mdAncs(arr);
            this.setupAncs(1);
            this.convertHrefs();

            puts('res',sub);
        },
        convertHrefs() {
            let l = document.querySelectorAll('#markitup a');
            for (let i=0; i<l.length; i++) {
                let href = l[i].getAttribute('href');
                if (href.indexOf(':')>=0) continue;
                let buri = l[i].baseURI.split('#')[0];
                //let cl = l[i].classList;
                //if (cl && cl.contains('toc-anchor')) continue;
                let that = this;
                let vr = href;
                if (href[0] == '#')
                    vr = this.s_mdFile+href;
                vr = encodeURIComponent(vr);
                //l[i].href = buri+'#/Docs/View/'+ vr;
                l[i].onclick=function() {
                    that.$router.push('/Docs/View/'+ vr);
                    return false;
                }
            }
        },
        setupAncs(c) { // Update s_ancChain on scroll.
            function getAnc(t, depth) {
                if (depth>100) throw("bad anchor list");
                for (let i = 0; i<t.length; i++)
                    if (t[i].idx === c) return t[i];
                for (let i = 0; i<t.length; i++) {
                    if (t[i].children && c > t[i].idx && ((i==(t.length-1) || c < t[i+1].idx)))
                        return getAnc(t[i].children, depth+1);
                }
                return null;
            }

            let ac = [], toc = this.s_mdAncs, ta = getAnc(toc, 0);
            this.update_s_curAnc(c);
            this.update_s_curAncParent((ta && ta.parent)?ta.parent.idx:-1);
            while (ta && ta.parent && ta.parent.idx>=0 && ac.length<100) {
                ta = ta.parent;
                ac.unshift(ta.idx);
            }
            this.update_s_ancChain(ac);
            //this.scrollTocView(c)
        },
        handleScroll(ev) {
            let sections = this.ancs;
            if (!this.ancs) return;
            let sectionMargin = 1;
            let current = sections.length - 
                [...sections].reverse()
                .findIndex(section => window.scrollY >= section.offsetTop - sectionMargin ) - 1;
            if (current<1) current = 1; //TODO: ignoring H1
            if (current == this.s_curAnc)
                return;
            this.setupAncs(current);
        },
        getTocList(id, first, last) {
            let ii, i, ip, aa = [], bb = [], cc={}, aids = document.querySelectorAll('#markitup a[class=toc-anchor]'),
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
                let label = ip.innerText.trim(); //escapeHtml(ip.innerText.trim(), 1);
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
                let res = [], i;
                while (idx<bb.length && bb[idx].level>=cur) {
                    i = bb[idx];
                    i.idx = idx;
                    let ni = bb[++idx];
                    i.children =  (ni && ni.level>cur)? pushHdrs(cur+1, i) : null;
                    i.parent = parent;
                    res.push(i);
                }
                return res;
            }
            
            let idx = 0;
            let res = pushHdrs(min, null);
            return [res, bb];
        },
    },
    watch: {
        s_fileMod() {
            if (!this.s_fileMod) return;
            puts('Filemod', this.s_fileMod, this.s_mdFile);
            let fm = this.s_fileMod;
            this.update_s_fileMod(null);
            if (fm.indexOf('/'+this.s_mdFile)<0) return;
            this.update_s_dirty(true);
            this.loadMdFile();
        },
    },
};

let store = new Vuex.Store({
    state: {},
    mutation: {},
});

let DocsStore = {
    //strict:true,
    namespaced:true,
    state: {
        s_mdDir:'../md/',
        s_mdFile:null,
        s_mdList:[],
        s_mdToc:null,
        s_mdAncs:null,
        s_mdIndex:null,
        s_searchList:[],
        s_server:null,
        s_fossil:(location.pathname.indexOf('/doc/ckout/')>=0),
        s_curAnc:0,
        s_curAncParent:-1,
        s_ancChain:[],
        s_fileMod:null,
        s_dirty:false,
        s_showAllSub:false,

    },
    mutations: {
        update_s_mdFile(state, val)      { state.s_mdFile = val; },
        update_s_mdToc(state, val)       { state.s_mdToc = val; },
        update_s_mdAncs(state, val)      { state.s_mdAncs = val; },
        update_s_curAnc(state, val)      { state.s_curAnc = val; },
        update_s_mdList(state, val)      { state.s_mdList = val; },
        update_s_mdIndex(state, val)     { state.s_mdIndex = val; },
        update_s_searchList(state, val)  { state.s_searchList = val; },
        update_s_server(state, val)      { state.s_server = val; },
        update_s_ancChain(state, val)    { state.s_ancChain = val; },
        update_s_curAncParent(state, val){ state.s_curAncParent = val; },
        update_s_fileMod(state, val)     { state.s_fileMod = val; },
        update_s_dirty(state, val)       { state.s_dirty = val; },
        update_s_showAllSub(state, val)  { state.s_showAllSub = val; },
    },
};

store.registerModule('Docs', DocsStore);
/*============================*/

let childlst = [ 
    {path: 'Docs', name:'Docs', redirect:'Docs/View', component:Docs, meta:{data:self},
        children: [
            {path: 'View/:path?', name:'Docs.View', component: DocsView, meta:{data:self}},
        ]
    }
];

let rconf = {
 //  mode:'history', 
  routes: [{ path:'/', name:'/', redirect:'/Docs', component:Top, 
      meta:{visible:true, data:self, icon:'fa-bars'},
      children:childlst,
  }]
};

const router = new VueRouter( rconf );


$jsi.websock({onchange:function(fname, extn) {
    // If jsish server, handle notify on file change.
    if (extn == 'md') {
        store.commit('Docs/update_s_fileMod', fname);
    } else
        location.reload();
}});


/*const originalPush = VueRouter.prototype.push;
VueRouter.prototype.push = function push(location) {
  return originalPush.call(this, location).catch(err => {
    if (err.name !== 'NavigationDuplicated') throw err
  });
}*/

Vue.use(VueMarkdownPdq);
let vm = new Vue({
        store,
        router,
    }).$mount('#app');


