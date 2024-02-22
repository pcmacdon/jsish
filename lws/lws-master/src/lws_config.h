/* lws_config.h  Generated from lws_config.h.in  */

#ifndef NDEBUG
    #ifndef _DEBUG
        #define _DEBUG
    #endif
#endif

/* #undef LWS_WITH_ESP8266 */
/* #undef LWS_WITH_ESP32 */

/* #undef LWS_WITH_PLUGINS */
/* #undef LWS_WITH_NO_LOGS */

/* The Libwebsocket version */
#define LWS_LIBRARY_VERSION "2.2.2"

#define LWS_LIBRARY_VERSION_MAJOR 2
#define LWS_LIBRARY_VERSION_MINOR 2
#define LWS_LIBRARY_VERSION_PATCH 2
/* LWS_LIBRARY_VERSION_NUMBER looks like 1005001 for e.g. version 1.5.1 */
#define LWS_LIBRARY_VERSION_NUMBER (LWS_LIBRARY_VERSION_MAJOR*1000000)+(LWS_LIBRARY_VERSION_MINOR*1000)+LWS_LIBRARY_VERSION_PATCH

/* The current git commit hash that we're building from */
#define LWS_BUILD_HASH "pmacdona@peter-pc-"

/* Build with OpenSSL support */
/* #undef LWS_OPENSSL_SUPPORT */

/* The client should load and trust CA root certs it finds in the OS */
#define LWS_SSL_CLIENT_USE_OS_CA_CERTS

/* Sets the path where the client certs should be installed. */
#define LWS_OPENSSL_CLIENT_CERTS "../share"

/* Turn off websocket extensions */
/*#define LWS_NO_EXTENSIONS */

/* Enable libev io loop */
/* #undef LWS_USE_LIBEV */

/* Enable libuv io loop */
/* #undef LWS_USE_LIBUV */

/* Enable libevent io loop */
/* #undef LWS_USE_LIBEVENT */

#ifndef __WIN32
/* Build with support for ipv6 */
#define LWS_USE_IPV6 1

/* Build with support for UNIX domain socket */
#define LWS_USE_UNIX_SOCK 1
#endif

/* Turn on latency measuring code */
/* #undef LWS_LATENCY */

/* Don't build the daemonizeation api */
#define LWS_NO_DAEMONIZE

/* Build without server support */
/* #undef LWS_NO_SERVER */

/* Build without client support */
/* #undef LWS_NO_CLIENT */

/* use SHA1() not internal libwebsockets_SHA1 */
/* #undef LWS_SHA1_USE_OPENSSL_NAME */

/* SSL server using ECDH certificate */
/* #undef LWS_SSL_SERVER_WITH_ECDH_CERT */
/* #undef LWS_HAVE_X509_VERIFY_PARAM_set1_host */

/* #undef LWS_HAVE_UV_VERSION_H */

/* CGI apis */
/* #undef LWS_WITH_CGI */

/* whether the Openssl is recent enough, and / or built with, ecdh */
/* #undef LWS_HAVE_OPENSSL_ECDH_H */

/* HTTP Proxy support */
/* #undef LWS_WITH_HTTP_PROXY */

/* HTTP Ranges support */
#define LWS_WITH_RANGES

/* Http access log support */
/* #undef LWS_WITH_ACCESS_LOG */
#define LWS_WITH_SERVER_STATUS 1

/* Maximum supported service threads */
#define LWS_MAX_SMP 1

/* OPTEE */
/* #undef LWS_PLAT_OPTEE */

/* ZIP FOPS */
/* #undef LWS_WITH_ZIP_FOPS */
#define LWS_HAVE_STDINT_H

/* #undef LWS_FALLBACK_GETHOSTBYNAME */

