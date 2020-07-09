# Makefile to download/compile libwebsockets.a, with or without CMAKE.
#LWSVER=1.7
LWSVER=2.2
SOURCEDIR=lwl
TARGET=unix
LWSLIB=liblws_$(TARGET)
CFLAGS  = -g -Wall -I../miniz

ifeq ($(MINIZ),1)
CFLAGS += -I../miniz
endif

all: download
	$(MAKE) -Clws TARGET=$(TARGET)
	$(MAKE) -Clws TARGET=$(TARGET) liblws

download: lws/Makefile

lws/Makefile:
	wget  http://jsish.org/fossil/lws/zip/lws -O lws.zip 
	unzip lws.zip

clean:
	rm -f $(LWSLIB)

cleanall: clean
	rm -rf build
	
.PHONY: all depend remake clean cleanall download
