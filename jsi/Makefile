EXTFLAGS=-DJSI_STANDALONE
INCS=-I.
WEBSOCKDIR = ../libwebsockets
SQLITEDIR = ../sqlite3
ACFILES	= parser.c
CFLAGS += -Wc++-compat -Wwrite-strings
#TODO: configure option to make these 2 builtin or loadable.

PCFILES = jsiLexer.c jsiFunc.c jsiValue.c jsiRegexp.c jsiPstate.c jsiInterp.c \
    jsiUtils.c jsiProto.c jsiFilesys.c jsiChar.c jsiString.c jsiBool.c \
    jsiNumber.c jsiArray.c jsiLoad.c jsiHash.c jsiOptions.c jsiStubs.c \
    jsiFormat.c jsiJSON.c jsiTclUtil.c jsiCmds.c jsiFileCmds.c jsiObj.c jsiSignal.c\
    jsiTree.c jsiMD5.c jsiDString.c jsiMath.c jsmn.c jsiZvfs.c jsiUtf8.c jsiUserObj.c jsiSocket.c
CFILES = $(PCFILES) $(ACFILES)
EFILES = jsiEval.c
WFILES = win/compat.c win/regex.c win/strptime.c 
WIFILES = win/compat.h win/regex.h

HFILES = parser.h jsiCode.h jsiInt.h jsiPstate.h jsiLexer.h jsiUtf8.h jsmn.h
MEM_DEBUG=1
NOMYSQL=1

ifeq ($(MINIMAL),1)
NOWEBSOCK=1
NOSQLITE=1
NOREADLINE=1
NOSTUBS=0
DB_TEST=0
EXTFLAGS += -DJSI_OMIT_THREADS -DJSI_OMIT_ZVFS -DJSI_OMIT_FILESYS -DJSI_OMIT_LOAD -DJSI_OMIT_STUBS -DJSI_OMIT_SIGNAL
endif

ifeq ($(MEM_DEBUG),1)
EXTFLAGS += -DJSI_MEM_DEBUG=1
endif

ifeq ($(NOSTUBS),1)
EXTFLAGS += -DJSI_OMIT_STUBS
endif

ifeq ($(MUDFLAP),1)
EXTFLAGS += -fmudflap
EXTLDFLAGS += -lmudflap
endif

ifneq ($(NOWEBSOCK),1)
WEBSOCKLIB = $(WEBSOCKDIR)/build/lib/libwebsockets.a
EXTFLAGS += -DHAVE_WEBSOCKET=1
INCS += -I$(WEBSOCKDIR)/lib
CFILES += jsiWebsocket.c 
else
EXTFLAGS += -DWITHOUT_SHA1
endif

ifeq ($(USEMUSL),1)
CC=musl-gcc
CFLAGS += -static -DHAVE_MUSL
USEMINIZ=1
NOREADLINE=1
endif

ifneq ($(NOSQLITE),1)
SQLITELIB = $(SQLITEDIR)/libsqlite3.a
EXTFLAGS += -DHAVE_SQLITE=1
INCS += -I$(SQLITEDIR) 
CFILES += jsiSqlite.c
ifeq ($(DB_TEST),1)
EXTFLAGS += -DJSI_DB_TEST=1
endif
endif

ifneq ($(NOMYSQL),1)
EXTLDFLAGS += -lmysqlclient
EXTFLAGS += -DHAVE_MYSQL=1
CFILES += jsiMySql.c
endif

BUILDDIR = $(PWD)

STATICLIBS = $(WEBSOCKLIB)

ifneq ($(EXTSRC),)
CFILES += demos/$(EXTSRC).c
ifeq ($(EXTNAME),)
EXTNAME=$(EXTSRC)
endif
endif

ifneq ($(EXTNAME),)
EXTFLAGS += -DUSER_EXTENSION=Jsi_Init$(EXTNAME)
endif

ifeq ($(WIN),1)
# *********** WINDOWS *****************
NOREADLINE=1
TCPRE=i686-w64-mingw32
#TCPRE=x86_64-w64-mingw32
CC=$(TCPRE)-gcc
AR=$(TCPRE)-ar
LD=$(TCPRE)-ld
EXEEXT=.exe
CFILES += $(WFILES) $(SQLITEDIR)/sqlite3.c
ifeq ($(NOWEBSOCK),1)
#for windows without websock use miniz
USEMINIZ=1
else
WEBSOCKLIB = $(WEBSOCKDIR)/build/lib/libwebsockets_static.a
EXTLDFLAGS += $(WEBSOCKLIB)  $(WEBSOCKDIR)/build/lib/libZLIB.a -lwsock32 -lws2_32
INCS += -I$(WEBSOCKDIR)/win32port/win32helpers -I$(WEBSOCKDIR)/win32port/zlib
endif
STATICLIBS += $(SQLITELIB)

else
# *********** UNIX **********************
LNKFLAGS += -rdynamic
STATICLIBS += $(SQLITELIB)
EXTLDFLAGS += -ldl $(WEBSOCKLIB)
ifneq ($(USEMUSL),1)
COPTS = -fpic
EXTLDFLAGS += -lpthread
endif
endif

ifneq ($(NOREADLINE),1)
EXTFLAGS += -DHAVE_READLINE=1
EXTLDFLAGS += -lreadline -lncurses
endif

MINIZDIR=../miniz
ifeq ($(USEMINIZ),1)
CFILES += $(MINIZDIR)/miniz.c
INCS += -I$(PWD)/$(MINIZDIR)
else
ifneq ($(WIN),1)
EXTLDFLAGS += -lz
endif
endif

EXTLDFLAGS += $(USERLIB)
OBJS    = $(CFILES:.c=.o) $(EFILES:.c=.o)
DEFIN	= 
CFLAGS	+= -g -Wall $(COPTS) $(DEFIN) $(INCS) $(EXTFLAGS) $(OPTS)
YACC	= bison -v
LDFLAGS = -lm $(EXTLDFLAGS)
SHLEXT=.so
ZIPDIR=zipdir
BLDDIR=$(PWD)
TARGETA	= jsish_$(EXEEXT)
TARGET	= jsish$(EXEEXT)

.PHONY: all clean cleanall

all: $(STATICLIBS) $(TARGET)

help:
	@echo "targets are: mkwin mkmusl stubs ref test testmem release"

mkwin:
	make WIN=1

mkmusl:
	make USEMUSL=1

main.o: .FORCE

.FORCE:

$(TARGETA): parser.c $(OBJS) main.o
	$(AR) r libjsi.a $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(SQLITELIB) main.o $(LNKFLAGS) -o $(TARGETA) $(LDFLAGS)

jsishs$(EXEEXT): parser.c $(OBJS) main.o
	$(CC) $(CFLAGS) main.o -o $@ -L. -ljsi $(LDFLAGS)

shared: libjsi$(SHLEXT) jsish$(EXEEXT)

$(TARGET): $(TARGETA) .FORCE
	rm -f $@
	cp $(TARGETA) $@
ifneq ($(MINIMAL),1)
	fossil info | grep ^checkout | cut -b15- > lib/sourceid.txt
ifeq ($(WIN),1)
	./jsish_ lib/zip.jsi create $@ $(ZIPDIR) lib
else
	$(BLDDIR)/$(TARGETA) lib/zip.jsi create $@ $(ZIPDIR) lib
endif
endif

apps: ledger$(EXEEXT) sqliteui$(EXEEXT)

ledger$(EXEEXT): .FORCE
	cp $(TARGETA) $@
ifeq ($(WIN),1)
	./jsish_ lib/zip.jsi create  $@ ../ledger lib
else
	$(BLDDIR)/$(TARGETA) lib/zip.jsi create  $@ ../ledger lib
endif

sqliteui$(EXEEXT):  .FORCE
	cp $(TARGETA) $@
ifeq ($(WIN),1)
	./jsish_ lib/zip.jsi create  $@ ../sqliteui lib
else
	$(BLDDIR)/$(TARGETA) lib/zip.jsi create  $@ ../sqliteui lib
endif

ifeq ($(WIN),1)
$(WEBSOCKLIB):
	(cd $(WEBSOCKDIR) && mkdir build && cd build && cmake -DWITH_SSL=0 -DCMAKE_TOOLCHAIN_FILE=../cross-ming.cmake ..  && make)
else
$(WEBSOCKLIB):
	(cd $(WEBSOCKDIR) && mkdir build && cd build &&  AR=$(AR) CC=$(CC) LD=$(LD) cmake -DWITH_SSL=0 .. && make)
endif

$(SQLITELIB):
	(cd $(SQLITEDIR) && make CC=$(CC) AR=$(AR) LD=$(LD))

# Create the single amalgamation file jsi.c.
jsi.c: jsi.h $(HFILES) $(CFILES) Makefile
	cat jsi.h > $@
	echo "#define JSI_AMALGAMATION" >> $@
	echo "#ifdef __WIN32" >> $@
	cat $(MINIZDIR)/miniz.c >> $@
	echo "#endif" >> $@
	cat $(WIFILES) $(HFILES) jsiCode.c $(PCFILES) jsiSqlite.c jsiWebsocket.c jsiMySql.c >> $@
	echo "#ifndef JSI_LITE_ONLY" >> $@
	grep -v '^#line' $(ACFILES)  >> $@
	echo "#endif" >> $@
	cat $(WFILES) $(EFILES)  >> $@

single:	jsi_single_src.zip

jsi_single_src.zip: jsi.c
	(cd .. && zip -r jsi/$@ jsi/$< jsi/jsi.h jsi/jsiStubs.h jsi/jsiDString. jsi/c-demos/simple.c)

.FORCE:

depend:
	$(CC) -E -MM -DHAVE_WEBSOCKET=1 -DHAVE_SQLITE $(CFILES) $(EFILES) > .depend

-include .depend

debug: $(OBJS)
	$(CC) -g -Wall $(DEFIN) $(CFILES) $(EFILES) -o $(TARGET) -lm

libjsi$(SHLEXT): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -Wl,--export-dynamic  -shared -o $@ $(LDFLAGS)

mysql:
	touch jsiInterp.c
	make NOMYSQL=0

libmysql: libjsiMySql$(SHLEXT)

libjsiMySql$(SHLEXT): jsiMySql.c
	$(CC) $(CFLAGS) -DHAVE_MYSQL jsiMySql.c -Wl,--export-dynamic  -shared -o $@ $(LDFLAGS) -lmysqlclient

parser.c: parser.y
	$(YACC) -oparser.c -d parser.y

stubs:
	./$(TARGET) tools/mkstubs.js

ref:
	./$(TARGET) tools/mkproto.js > tools/protos.jsi
	make -C www

release: stubs ref jsi.c

test:
	tools/testjs.sh tests

# This requires building with MEM_DEBUG=1 above.
testmem:
	JSI_INTERP_OPTS="{memDebug:1}" tools/testjs.sh tests

testvg:
	tools/testjs.sh -valgrind tests

testledger:
	jsish ../ledger/ledger.jsi

tags:
	geany -g -P geany.tags *.c *.h

clean:
	rm -rf *.o *.a *.output *.stackdump $(SQLITEDIR)/*.o $(MINIZDIR)/*.o win/*.o $(TARGET) $(WEBSOCKDIR)/build $(SQLITELIB) jsi_amalgamation.c

cleanall: clean
	rm -f $(ACFILES) $(TARGETA) output core parser.c parser.h parser.tab.c *.stackdump libjsi.a

