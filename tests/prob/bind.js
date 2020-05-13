function h(a) { return {a: a, th: this}; }
var i = h.bind({str: "foo"}, 2);

i.call({x: 1});
