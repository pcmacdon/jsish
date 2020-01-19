DeepDoc
====
DeepDoc uses markdeep in conjunction with Server-Side-Include (SSI)
to implement a simple web framework.
This can be served out directly from nginx, and/or
Jsi can be used to generate static html web pages
that load lightning fast.


Local
----
To display use "jsish -W -url DeepDoc

Html
----
To convert all .md files to static .html files use:
~~~~
jsish -g
~~~~
Then the entire directory can be uploaded to a web site.  Test with:
~~~~
jsish -W DeepDoc.html
~~~~

Nginx
----
Nginx can be configured to serve ".md" markdeep files by
adding to nginx.conf:
~~~~
include nginx_deepdoc.conf;"
~~~~
