# Makefile to create lwsOne.c and liblws.a
# TODO: create miniz.c
LWS_VER=2.0202
LWS_SSL=0
LWS_MINIZ=0   # 1=external provide, 2=internally provided
LWS_CFLAGS=

TARGET=unix
LWSBASE=src
CFLAGS=-g -Wall -I$(SD) $(LWS_CFLAGS)

SD=$(LWSBASE)
LWSINT=$(LWSBASE)
LWS_LIBNAME=liblws.a

SOURCES = $(SD)/base64-decode.c $(SD)/handshake.c $(SD)/lws.c \
	$(SD)/service.c $(SD)/pollfd.c $(SD)/output.c $(SD)/parsers.c \
	$(SD)/context.c $(SD)/alloc.c $(SD)/header.c $(SD)/client.c \
	$(SD)/client-handshake.c $(SD)/client-parser.c $(SD)/sha-1.c \
	$(SD)/server.c $(SD)/server-handshake.c \
	$(SD)/extension.c $(SD)/extension-permessage-deflate.c $(SD)/ranges.c



ifeq ($(LWS_MINIZ),1)
CFLAGS += -Iminiz
endif
ifeq ($(LWS_MINIZ),2)
CFLAGS += -DLWS_MINIZ=1
endif

SSLSOURCES += $(SD)/ssl.c $(SD)/ssl-client.c $(SD)/ssl-server.c
#$(SD)/ssl-http2.c $(SD)/http2.c

ifeq ($(LWS_SSL),1)
CFLAGS += -DLWS_OPENSSL_SUPPORT=1 -DLWS_WITH_SSL=1 
#-DLWS_USE_HTTP2=1
CFLAGS += -I$(HOME)/usr/openssl/include
endif


WFILES = $(SD)/lws-plat-win.c 
UFILES = $(SD)/lws-plat-unix.c 

ifeq ($(WIN),1)
CFLAGS +=  -D__USE_MINGW_ANSI_STDIO -I$(SD)/../win32port/win32helpers
endif

all: lwsOne.c liblws

# Create the single amalgamation file lwsSingle.c
lwsSingle.c: $(SD)/lws.h $(SOURCES) $(SSLSOURCES) $(MAKEFILE)
	cat $(SD)/lws.h > $@
	echo "#ifndef LWS_IN_AMALGAMATION" >> $@
	echo "#define LWS_IN_AMALGAMATION" >> $@
	echo "#define _GNU_SOURCE"  >> $@
	echo "#define LWS_AMALGAMATION" >> $@
	echo "#if LWS_MINIZ==1" >> $@
	cat miniz/miniz.c >> $@
	echo "#endif //LWS_MINIZ==1 " >> $@
	cat $(SOURCES) | grep -v '^#line' >> $@
	echo "#if LWS_WITH_SSL==1" >> $@
	cat $(SSLSOURCES) | grep -v '^#line' >> $@
	echo "#endif //LWS_WITH_SSL==1 " >> $@
	echo "#ifndef WIN32" >> $@
	cat $(WFILES)  >> $@
	echo "#else // WIN32" >> $@
	cat $(UFILES)  >> $@
	echo "#endif //WIN32" >> $@
	echo "#endif //LWS_IN_AMALGAMATION" >> $@

# Create the single compile file lwsOne.c
lwsOne.c: $(SD)/lws.h   $(SOURCES) $(SSLSOURCES) $(MAKEFILE)
	echo '#include "$(SD)/lws.h"' > $@
	echo "#define LWS_AMALGAMATION" >> $@
	echo "#if LWS_MINIZ==1" >> $@
	echo '#include "'miniz/miniz.c'"' >> $@
	echo "#endif //LWS_MINIZ==1" >> $@
	for ii in  $(SOURCES); do echo '#include "'$$ii'"' >> $@; done
	echo "#if LWS_WITH_SSL==1" >> $@
	for ii in  $(SSLSOURCES); do echo '#include "'$$ii'"' >> $@; done
	echo "#endif //LWS_WITH_SSL==1" >> $@
	echo "#ifdef WIN32" >> $@
	for ii in $(WFILES); do echo '#include "'$$ii'"' >> $@; done
	echo "#else // WIN32" >> $@
	for ii in $(UFILES); do echo '#include "'$$ii'"' >> $@; done
	echo "#endif //WIN32" >> $@


liblws: $(LWS_LIBNAME)

$(LWS_LIBNAME): lwsOne.c
	$(CC) -c -o $@ lwsOne.c $(CFLAGS)


clean:
	rm -f liblws*.a

cleanall: clean
	rm -f lwsOne.c lwsSingle.c
	
.PHONY: all depend remake clean cleanall
