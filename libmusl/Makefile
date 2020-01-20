# Makefile to download/compile libmusl
MUSLVER=1.1.24
SOURCEP=musl-$(MUSLVER)
MUSLURL=https://www.musl-libc.org/releases/$(SOURCEP).tar.gz
BUILDDIR=build

all: mkdirs download $(BUILDDIR)/obj/musl-gcc

mkdirs:
	mkdir -p $(BUILDDIR)

download: $(SOURCEP).tar.gz

$(SOURCEP).tar.gz:
	wget $(MUSLURL) || wget http://jsish.org/download/musl-$(MUSLVER).tar.gz
	tar -xzpvf musl-$(MUSLVER).tar.gz

$(BUILDDIR)/obj/musl-gcc:
	(cd $(BUILDDIR) && ../$(SOURCEP)/configure --prefix=$PWD && make && make install)

clean:

cleanall: clean
	rm -rf build $(SOURCEP).tar.gz
	
.PHONY: all depend remake clean cleanall mkdirs download
