# Makefile to download/compile "liblws_TARG-VER.a"
#LWSVER=1.7
LWS_VER=2.0202
TARGET=unix
SDIR=lws-$(LWS_VER)

LWS_LIBNAME=$(SDIR)/liblws.a
CFLAGS  = -g -Wall

ifeq ($(MINIZ),1)
LWSOPTS = -I../../miniz
endif

all: $(SDIR)/Makefile
	$(MAKE) -C$(SDIR) TARGET=$(TARGET) LWSOPTS=$(LWSOPTS) CC=$(CC) LWS_LIBNAME=$(LWS_LIBNAME)

$(SDIR)/Makefile:
	wget  http://jsish.org/fossil/lws/zip/lws?r=lws-$(LWS_VER) -O lws-$(LWS_VER).zip 
	unzip -oq lws-$(LWS_VER).zip
	mv lws $(SDIR)
	# TODO FALLBACK: wget  http://github.com/pcmacdon/lws/archive/lws-$(LWS_VER).zip

clean:
	$(MAKE) -C$(SDIR) clean
	
cleanall:
	$(MAKE) -C$(SDIR) cleanall
	
.PHONY: all depend remake clean cleanall
