# Makefile to download/compile libsqlite.a
TARGET=unix
SOURCEDIR=src
CFLAGS= -fPIC -I$(SOURCEDIR) -I../miniz -I$(TARGET)/ -DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_DBSTAT_VTAB
SQLITE_VER=3300100
SQLURL=https://www.sqlite.org/2019/sqlite-amalgamation-$(SQLITE_VER).zip

BUILDDIR=$(SOURCEDIR)
SQLITE_LIBNAME=libsqlite3.a
LIBNAME=$(BUILDDIR)/$(SQLITE_LIBNAME)
SOURCES = $(SOURCEDIR)/sqlite3.c

OBJLST  = $(SOURCES:.c=.o)
OBJECTS	= $(patsubst $(SOURCEDIR)/%,$(BUILDDIR)/%,$(OBJLST))


all: download $(LIBNAME)

download: $(SOURCEDIR)/sqlite3.c

$(SOURCEDIR)/sqlite3.c:
	wget $(SQLURL)
	unzip sqlite-amalgamation-$(SQLITE_VER).zip
	ln -sf sqlite-amalgamation-$(SQLITE_VER) src
	rm sqlite-amalgamation-$(SQLITE_VER).zip

$(LIBNAME): $(SOURCEDIR)/sqlite3.c Makefile ../Makefile
	$(CC) -c -I$(SOURCEDIR) $(SOURCEDIR)/sqlite3.c $(CFLAGS)
	$(AR) rv $(LIBNAME) sqlite3.o

clean:
	rm -f $(OBJECTS) $(LIBNAME)

cleanall: clean
	rm -rf src/*.a
	
.PHONY: all depend remake clean cleanall download
