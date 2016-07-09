/*
=!EXPECTSTART!=
extra args, expected "cmds(val:string|regexp='*', all:boolean=false)" 
=!EXPECTEND!=
*/
Interp.conf({typeCheck:'error'});
function foo() {
  var i = Info; // Prevents static type check detections.
  i.cmds(1,2,3,4);
}
try { foo(); } catch(e) { puts(e); }
