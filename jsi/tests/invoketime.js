/*
=!EXPECTSTART!=
2
=!EXPECTEND!=
*/

// 定义一个函数 - add
function add(a, b) {
   add.invokeTimes++;
   return a + b;
};

// 因为函数本身也是对象，这里为函数add定义一个属性，用来记录此函数被调用的次数
add.invokeTimes = 0;
add(1 + 1);
add(2 + 3);
puts(add.invokeTimes); // 2
