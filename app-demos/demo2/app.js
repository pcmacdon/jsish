// APP WITH ROUTER.
Vue.component('app-view', {template: '<router-view />' } );

let data = {
   str:'', calval:'', rateval:3, tagval:['Fast', 'Easy'],
    value: 45,
};

const App = {
  data() { return data; },
  template:`
    <b-card>
      <div>
        <b-progress :value="value" :max="max" show-progress animated></b-progress>
        <b-progress class="mt-2" :max="max" show-value>
          <b-progress-bar :value="value * (6 / 10)" variant="success"></b-progress-bar>
          <b-progress-bar :value="value * (2.5 / 10)" variant="warning"></b-progress-bar>
          <b-progress-bar :value="value * (1.5 / 10)" variant="danger"></b-progress-bar>
        </b-progress>
    
        <b-button class="mt-3" @click="value = Math.random() * 100">Click me</b-button>
      </div>
      <b-jumbotron bg-variant="info" header="Demo-2" lead="Jsi Web-App Using Bootstrap-Vue">
        <p>For more information visit <b-link href="https://github.com/pcmacdon/jsish" class="text-white">jsish.org</b-link></p>
        <b-button variant="primary" href="#/Sub">Next></b-button>
      </b-jumbotron>
    </b-card>
  `,
};

const Sub = {
  data() { return data; },
  template:`
    <b-card>
      <b-jumbotron bg-variant="muted" header="Demo Rating" lead="How would you rate this demo">
        <b-form-rating v-model="rateval" variant="success" class="mb-2"></b-form-rating>
        <b-form-tags v-model="tagval" tag-class="mx-1" class="mb-2"></b-form-tags>
        <b-form-datepicker v-model="calval" class="mb-2"/>
        Rating {{rateval}} : {{tagval.join(', ')}} @ {{calval}}
        <p>Use browser back-button to return, or click <span class="lightcolor">Back</span> below</p>
        <b-button variant="primary" href="#/">Back</b-button>
      </b-jumbotron>
    </b-card>
  `,
};

let router = new VueRouter({routes: [
   { path:'/',    name:'/',   component:App, },
   { path:'/Sub', name:'Sub', component:Sub, },
   { path:'*',    redirect:'/' }
]});
new Vue({ router }).$mount('#app');

//$jsi.loadCSS(window.jsiapp+'app.css');
$jsi.websock(); // Hot-reload.

