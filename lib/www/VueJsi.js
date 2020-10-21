
import './js/vue.js';
Vue.config.productionTip=false;
import './js/vue-router.min.js';
import './js/bootstrap-vue.min.js';
import './js/vue-markdown-pdq.min.js';
import './js/prism.js';
import './js/VuePrismEditor.umd.min.js';
import './js/Jsish.js';
import Vuex from './js/vuex.js';
window.Vuex = Vuex;

let opts = window.VueJsiOptions;
if (opts)
    Jsish.conf(opts);

let root = Jsish.config.jsiroot

window.$jsi.loadCSS(root+'css/bootstrap.css');
window.$jsi.loadCSS(root+'css/bootstrap-vue.css');
window.$jsi.loadCSS(root+'css/animate.css');
window.$jsi.loadCSS(root+'css/prism.css');
window.$jsi.loadCSS(root+'css/VuePrismEditor.css');
