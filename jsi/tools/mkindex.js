#!/usr/bin/env jsish
// Create the Master index file for the wiki.

var flst = [], clst = [], jlst = [], lst = File.glob('*.wiki');

for (var i in lst) {
    if (lst[i].match('^c-'))
        clst.push(lst[i]);
    else if (lst[i].match('^js-'))
        jlst.push(lst[i]);
    else flst.push(lst[i]);
}
puts("<title>Index</title>");
puts("<table><tr><td valign=top>\n\n");
puts("<h4 style='text-align:center'>General</h4>\n");
var dat,title;
for (var i in flst.sort()) {
      if (flst[i] == "jsindex.wiki") continue;
      if (flst[i] == "index.wiki") continue;
      dat = File.read(flst[i]);
      title = dat.match('<title>([^>]+)</title>');
      if (!title)
            console.log("failed on "+flst[i]+"\n");
      puts('  *  <a href='+flst[i]+'>'+title[1]+'</a>');
}
puts("</td><td valign=top>");
puts("\n<h4 style='text-align:center'>Javascript</h4>");
var dat,title, ttl;
for (var i in jlst.sort()) {
    dat = File.read(jlst[i]);
    title = dat.match('<title>([^>]+)</title>');
    if (!title)
    console.log("failed on "+flst[i]);
    puts('  *  <a href='+jlst[i]+'>'+title[1]+'</a>');
}
puts("</td><td valign=top>");
puts("\n<h4 style='text-align:center'>C-API</h4>");
for (var i in clst.sort()) {
    dat = File.read(clst[i]);
    title = dat.match('<title>([^>]+)</title>');
    if (!title)
        console.log("failed on "+flst[i]);
    ttl = title[1]; 
    if (ttl.substr(0,6) == 'C-API:')
        ttl = ttl.substr(6);
    puts('  *  <a href='+clst[i]+'>'+ttl+'</a>');
}
puts("</td></tr></table>");
