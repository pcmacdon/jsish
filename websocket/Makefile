# Makefile to download/compile libwebsockets.a, with or without CMAKE.
#LWSVER=1.7
LWSVER=2.2

TARGET=unix
LWSBASE=src
CFLAGS  = -g -Wall -I$(SOURCEDIR) -I$(TARGET)/ -I../miniz
#-fsanitize=address

ifeq ($(MINIZ),1)
CFLAGS += -I../miniz
endif

SOURCEDIR=$(LWSBASE)/lib
LWSINT=$(LWSBASE)
BUILDDIR=build/$(TARGET)
LWSLIBNAME=libwebsockets.a
LWSLIBBUILD=$(LWSLIBNAME)
CMAKE=cmake
DOCMAKE=0

LWSLIB=build/$(TARGET)/$(LWSLIBNAME)

SOURCES = $(SOURCEDIR)/base64-decode.c $(SOURCEDIR)/handshake.c $(SOURCEDIR)/libwebsockets.c \
	$(SOURCEDIR)/service.c $(SOURCEDIR)/pollfd.c $(SOURCEDIR)/output.c $(SOURCEDIR)/parsers.c \
	$(SOURCEDIR)/context.c $(SOURCEDIR)/alloc.c $(SOURCEDIR)/header.c $(SOURCEDIR)/client.c \
	$(SOURCEDIR)/client-handshake.c $(SOURCEDIR)/client-parser.c $(SOURCEDIR)/sha-1.c \
	$(SOURCEDIR)/server.c $(SOURCEDIR)/server-handshake.c \
	$(SOURCEDIR)/extension.c $(SOURCEDIR)/extension-permessage-deflate.c
	
ifneq ($(LWSVER),"1.7")
SOURCES += $(SOURCEDIR)/ranges.c

ifeq ($(LWSSSL),1)
SOURCES += $(SOURCEDIR)/ssl.c $(SOURCEDIR)/ssl-client.c $(SOURCEDIR)/ssl-server.c $(SOURCEDIR)/ssl-http2.c
endif

endif

#alloc.c.o          client.c.o            client-parser.c.o  handshake.c.o  libwebsockets.c.o  output.c.o   pollfd.c.o  server.c.o            service.c.o
#base64-decode.c.o  client-handshake.c.o  context.c.o        header.c.o     lws-plat-unix.c.o  parsers.c.o  ranges.c.o  server-handshake.c.o  sha-1.c.o


WFILES = $(SOURCEDIR)/lws-plat-win.c 
UFILES = $(SOURCEDIR)/lws-plat-unix.c 

ifeq ($(WIN),1)
CFLAGS +=  -D__USE_MINGW_ANSI_STDIO -I$(SOURCEDIR)/../win32port/win32helpers
SOURCES += $(WFILES)
else
SOURCES += $(UFILES)
endif

OBJLST  = $(SOURCES:.c=.o)
OBJECTS	= $(patsubst $(SOURCEDIR)/%,$(BUILDDIR)/%,$(OBJLST))

USECMAKE=
# Detect cmake and use it if not disabled.
ifeq ($(USECMAKE),)
CMAKEPATH := $(shell which $(CMAKE) )
ifneq ($(CMAKEPATH),)
DOCMAKE=1
endif
endif

LWSFLAGS = -DLWS_WITHOUT_TESTAPPS=1 -DCMAKE_C_FLAGS=-Wno-error -DWITH_SSL=0  \
	-DLWS_WITH_SSL=0 -DLWS_WITH_ZLIB=0 -DLWS_USE_BUNDLED_ZLIB=0 -DLWS_WITHOUT_EXTENSIONS=1

ifeq ($(WIN),1)
LWSFLAGS += -DLWS_USE_BUNDLED_ZLIB=1 -DCMAKE_TOOLCHAIN_FILE=../../$(LWSBASE)/cross-ming.cmake \
	-DCMAKE_INSTALL_PREFIX:PATH=/usr -DLWS_WITH_STATIC=1 -DLWS_WITH_SHARED=0
LWSLIBBUILD=libwebsockets_static.a
endif

all: mkdirs download $(BUILDDIR)/libwebsockets.a

mkdirs:
	mkdir -p $(BUILDDIR)

checkver:
	rm -f src && ln -sf libwebsockets-$(LWSVER) src

download: $(SOURCEDIR)/base64-decode.c

$(SOURCEDIR)/base64-decode.c:
	wget http://jsish.org/download/libwebsockets-$(LWSVER).zip
	unzip libwebsockets-$(LWSVER).zip
	ln -sf libwebsockets-$(LWSVER) src
	rm libwebsockets-$(LWSVER).zip
	$(MAKE)

ifeq ($(DOCMAKE),1)
$(BUILDDIR)/libwebsockets.a:
	( mkdir -p $(BUILDDIR) && cd $(BUILDDIR) && CC=$(CC) AR=$(AR) $(CMAKE) ../../$(LWSBASE) $(LWSFLAGS)   && $(MAKE) CC=$(CC) AR=$(AR) && cp lib/$(LWSLIBBUILD) libwebsockets.a)
else

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS) 

$(BUILDDIR)/libwebsockets.a: $(OBJECTS)
	$(AR) rv $@ $(OBJECTS)

endif

clean:
	rm -f $(OBJECTS) $(BUILDDIR)/libwebsockets.a

cleanall: clean
	rm -rf build
	
.PHONY: all depend remake clean cleanall download
