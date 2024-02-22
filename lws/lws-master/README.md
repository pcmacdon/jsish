lws is a small websocket library for embedded applications, and in particular, jsish.
This is fork/refactor of Andy Green's https://github.com/warmcat/libwebsockets/tree/v2.2-stable
If you need more a feature-full library, you should use libwebsockets instead.

Goals of this refactor are:

 - support only glibc/mingw/musl and openssl
 - simplified application integration via lwsOne.c
 - remove unused features
 - fix bugs and memory leaks

One line download/build:

    wget http://jsish.org/fossil/lws/zip/lws?r=ver-2.0202 -O lws.zip && unzip -oq lws.zip && make -C lws

For the latest release simply omit the ? query.

Source is https://jsish.org/fossil/lws, but report bugs to mirror site: https://github.com/pcmacdon/lws

Peter MacDonald
