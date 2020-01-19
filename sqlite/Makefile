# Makefile to download/compile libsqlite.a
TARGET=unix
SOURCEDIR=src
CFLAGS= -I$(SOURCEDIR) -I$(LWSBASE)/build -I../miniz -I$(TARGET)/ -DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_DBSTAT_VTAB
SQLVER=3300100
SQLURL=https://www.sqlite.org/2019/sqlite-amalgamation-$(SQLVER).zip

BUILDDIR=build/$(TARGET)
LIBNAME=libsqlite3.a
LWSLIBBUILD=$(LIBNAME)
CMAKE=cmake

LWSLIB=build/$(TARGET)/$(LIBNAME)

SOURCES = $(SOURCEDIR)/sqlite3.c

OBJLST  = $(SOURCES:.c=.o)
OBJECTS	= $(patsubst $(SOURCEDIR)/%,$(BUILDDIR)/%,$(OBJLST))

all: mkdirs download $(BUILDDIR)/libsqlite3.a

mkdirs:
	mkdir -p $(BUILDDIR)

download: $(SOURCEDIR)/sqlite3.c

$(SOURCEDIR)/sqlite3.c:
	wget $(SQLURL) || wget http://jsish.org/download/sqlite-amalgamation-$(SQLVER).zip
	unzip sqlite-amalgamation-$(SQLVER).zip
	ln -sf sqlite-amalgamation-$(SQLVER) src
	rm sqlite-amalgamation-$(SQLVER).zip


$(BUILDDIR)/libsqlite3.a: $(SOURCEDIR)/sqlite3.c Makefile
	$(CC) -c -I$(SOURCEDIR) $(SOURCEDIR)/sqlite3.c $(CFLAGS)
	$(AR) rv $(BUILDDIR)/libsqlite3.a sqlite3.o

clean:
	rm -f $(OBJECTS) $(BUILDDIR)/libsqlite3.a

cleanall: clean
	rm -rf build
	
.PHONY: all depend remake clean cleanall download
