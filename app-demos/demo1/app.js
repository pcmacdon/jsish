// SIMPLE APP.

let data = {
   str:'', calval:'', rateval:3, tagval:['Fast', 'Easy'],
};

Vue.component('app-view', {
  data() { return data; },
  template:`
    <b-card>
      <b-jumbotron bg-variant="muted" header="Minimal Demo" lead="How would you rate this demo">
        <b-form-rating v-model="rateval" variant="success" class="mb-2"></b-form-rating>
        <b-form-tags v-model="tagval" tag-class="mx-1" class="mb-2"></b-form-tags>
        <b-form-datepicker v-model="calval" class="mb-2"/>
        Rating {{rateval}} : {{tagval.join(', ')}} @ {{calval}}
      </b-jumbotron>
    </b-card>
  `,
});


let vm = new Vue().$mount('#app');

$jsi.websock(); // Hot-reload.

