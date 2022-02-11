# Makefile to download/compile "liblws_TARG-VER.a"
#LWSVER=1.7
#LWS_VER=2.0202
TARGET=unix
SDIR=lws-master

LWS_LIBNAME=liblws.a
CFLAGS  = -g -Wall

ifeq ($(LWS_MINIZ),1)
LWSOPTS = -I../../miniz
endif

all: $(SDIR)/Makefile $(SDIR)/$(LWS_LIBNAME)

$(SDIR)/$(LWS_LIBNAME): Makefile ../Makefile
	rm -f $(SDIR)/$(LWS_LIBNAME)
	$(MAKE) -C$(SDIR) TARGET=$(TARGET) LWSOPTS=$(LWSOPTS) CFLAGS="$(CFLAGS)" CC=$(CC) LWS_LIBNAME=$(LWS_LIBNAME) MINIZ=$(LWS_MINIZ)

$(SDIR)/Makefile:
	wget  https://github.com/pcmacdon/lws/archive/master.zip 
	unzip -oq master.zip
	ln -sf $(SDIR) src

clean:
	$(MAKE) -C$(SDIR) clean
	
cleanall:
	$(MAKE) -C$(SDIR) cleanall
	
.PHONY: all depend remake clean cleanall
