# Makefile to download/compile "liblws_TARG-VER.a"
#LWSVER=1.7
LWS_VER=2.0202
TARGET=unix
SDIR=lws-$(LWS_VER)
LWSLIB=$(SDIR)/liblws_$(TARGET)-$(LWS_VER).a
CFLAGS  = -g -Wall

ifeq ($(MINIZ),1)
LWSOPTS = -I../../miniz
endif

all: $(SDIR)/Makefile
	$(MAKE) -C$(SDIR) TARGET=$(TARGET) LWSOPTS=$(LWSOPTS) CC=$(CC)

$(SDIR)/Makefile:
	wget  http://jsish.org/fossil/lws/zip/lws?r=ver-$(LWS_VER) -O lws-$(LWS_VER).zip 
	unzip -oq lws-$(LWS_VER).zip
	mv lws $(SDIR)

clean:
	$(MAKE) -C$(SDIR) clean
	
cleanall: clean
	
.PHONY: all depend remake clean cleanall
