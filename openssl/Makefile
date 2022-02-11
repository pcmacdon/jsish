# Makefile to download/compile "libcrypto.a"
TARGET=unix
GROOT=OpenSSL_1_1_1-stable
SROOT=openssl-$(GROOT)
ZIPFILE=$(GROOT).zip
SDIR=$(TARGET)/$(SROOT)
DSTLIB=$(SDIR)/libcrypto.a
BLDTARG="linux-x86_64"
MCC=$(CC)

ifeq ($(TARGET),win)
BLDTARG="mingw"
endif

ifeq ($(TARGET),musl)
MCC="$(CC) -static -idirafter /usr/include/ -idirafter /usr/include/x86_64-linux-gnu/"
endif

all: $(ZIPFILE) $(DSTLIB)

$(ZIPFILE):
	wget  http://github.com/openssl/openssl/archive/$(ZIPFILE)

$(DSTLIB):
	mkdir -p $(SDIR) $(TARGET)/include
	(cd $(TARGET) && unzip -oq ../$(ZIPFILE))
	(cd $(SDIR) && \
	  CC=$(MCC) ./Configure  no-shared $(BLDTARG) && \
	  CC=$(MCC) make CC=$(MCC) build_libs && \
	  cp -a include/openssl ../include/ && \
	  cp -a *.a ../ \
	)

clean:
	rm -rf $(TARGET)
	
cleanall:
	rm -rf unix musl win $(ZIPFILE)

.PHONY: all clean cleanall
