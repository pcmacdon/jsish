# Makefile for jsish: controlled by make.conf from configure.
PREFIX=/usr/local
SQLITE_VER=3300100
LWS_VER=2.0202
WITH_SSL=0
WEBSOCKROOT = lws/src
WEBSOCKSRC = $(WEBSOCKROOT)/src
ACFILES	= src/parser.c
#ACFILES	= src/jsiParser.c
BUILDSYS = $(shell uname -o)
ALLTARGETS = 
EXTRATARGETS =
DESTDIR=
JSICONFS=$(shell find Configs/ -type f -name '*.conf' -exec basename \{\} .conf \; |cut -d_ -f2 | sort | xargs)
BLDDIR=$(PWD)

CFLAGS += -I. -Isrc -Wall -Wsign-compare -Wtype-limits -Wuninitialized -DJSI__MAIN=1
# -pg
#CFLAGS += -g -O3
CFLAGS += -g -Og -O0
#CFLAGS += -g -Og -g3
SLIBCFLAGS = -Wl,--export-dynamic -shared -DJSI_USE_STUBS=1

MAKEFILE=Makefile

PCFILES = src/jsiLexer.c src/jsiFunc.c src/jsiValue.c src/jsiRegexp.c src/jsiPstate.c src/jsiInterp.c \
    src/jsiUtils.c src/jsiProto.c src/jsiFilesys.c src/jsiChar.c src/jsiString.c src/jsiBool.c \
    src/jsiNumber.c src/jsiArray.c src/jsiLoad.c src/jsiHash.c src/jsiOptions.c src/jsiStubs.c \
    src/jsiFormat.c src/jsiJSON.c src/jsiCmds.c src/jsiFileCmds.c src/jsiObj.c src/jsiSignal.c\
    src/jsiTree.c src/jsiCrypto.c src/jsiDString.c src/jsiMath.c src/jsmn.c src/jsiZvfs.c src/jsiUtf8.c src/jsiUserObj.c\
    src/jsiSocket.c src/jsiSqlite.c src/jsiWebSocket.c src/jsiMySql.c src/jsiCData.c src/jsiVfs.c
CFILES = $(PCFILES) $(ACFILES)
EFILES = src/jsiEval.c
WFILES = win/compat.c win/strptime.c 
WIFILES = win/compat.h
REFILES = regex/regex.h regex/tre.h regex/regcomp.c  regex/regerror.c  regex/regexec.c regex/tre-mem.c
HFILES = src/parser.h src/jsiInt.h
PROGRAM=jsish
CONF_ARGS=
CONF=default

MAKECONF=make.conf
MAKECONFDEF=Configs/make_$(CONF).conf

ifeq ($(CONF),default)
-include $(MAKECONF)
endif

ifeq ($(DEFCONFIG_VER),)
-include $(MAKECONFDEF)
MAKECONF=$(MAKCONFDEF)
endif


ifneq ($(JSI_CONFIG_DEFINED),1)
badconfig:
	@echo "ERROR!!! Run 'make CONF=X' where X is one of:"
	@echo "   $(JSICONFS)"
	@exit 1
endif

# Detect when config file changed in incompatible way.
EXPECT_CONFIG_VER=2.0323
#ifneq ($(DEFCONFIG_VER),$(EXPECT_CONFIG_VER))
#badconfigured:
#	mv -f make.conf make.conf.bak
#	@echo "ERROR!!!!!!!!!  renamed incompatiable config file make.conf: Please rerun ./configure"
#	@echo "<$(DEFCONFIG_VER) != $(EXPECT_CONFIG_VER)>"
#endif

# Detect when jsimin is downlevel
#ifneq ($(CHECK_CONFIG_VER),)
#chkconfig:
#ifneq ($(CHECK_CONFIG_VER),$(EXPECT_CONFIG_VER))
#badjsimin:
#	mv -f jsimin jsimin.bak
#	@echo "NOTE!!!!!!!!!  renamed incompatiable jsimin"
#	@echo "<$(CHECK_CONFIG_VER) != $(EXPECT_CONFIG_VER)>"
#endif
#endif


ifeq ($(TARGET),win)
	JSI__REGEX=1
endif

ifeq ($(JSI__REGEX),1)
CFLAGS += -Iregex
CFILES += regex/regcomp.c  regex/regerror.c  regex/regexec.c regex/tre-mem.c
#HFILES += regex/regex.h regex/tre.h
endif

ifeq ($(LINKSTATIC),1)
PROGLDFLAGS += -static
endif

ifeq ($(JSI__SANITIZE),1)
CFLAGS += -fsanitize=address
ASAN_OPTIONS=abort_on_error=1
endif

ifeq ($(WITH_EXT_WEBSOCKET),1)

ifeq ($(BUILDIN_WEBSOCKET),1)

ifeq ($(WITH_SSL),1)
SSL_SFX=ssl_
else
SSL_SFX=
endif

LWS_LIBNAME = liblws_$(SSL_SFX)$(TARGET)-$(LWS_VER).a
LWSLIB = lws/src/$(LWS_LIBNAME)
WEBSOCKLIB = $(LWSLIB)

CFLAGS += -I$(WEBSOCKSRC)
#WEBSOCKLIB = lws/build/$(TARGET)/libwebsockets.a
#CFLAGS += -I$(WEBSOCKSRC)/lib  -I$(WEBSOCKSRC)/build -Iwebsocket/$(TARGET) -Ilws/build/$(TARGET)
STATICLIBS += $(WEBSOCKLIB)

ifeq ($(WITH_SSL),1)
#CFLAGS += -I$(HOME)/usr/include
# WEBSOCKLIB += $(HOME)/usr/lib/libssl.a $(HOME)/usr/lib/libcrypto.a
# CFLAGS += -DLWS_OPENSSL_SUPPORT=1 -I$(HOME)/usr/openssl/include
WEBSOCKLIB += openssl/$(TARGET)/libssl.a openssl/$(TARGET)/libcrypto.a
CFLAGS += -DLWS_OPENSSL_SUPPORT=1 -I$(BLDDIR)/openssl/$(TARGET)/include
ALLTARGETS += openssllib
ifeq ($(TARGET),win)
EXTRALD += -lcrypt32
endif
endif

ALLTARGETS += lwslib

else
WEBSOCKLIB = -llws
endif

PROGFLAGS += -DJSI__WEBSOCKET=1
else
endif

ifeq ($(WITH_EXT_SQLITE),1)

ifeq ($(BUILDIN_SQLITE),1)
SQLITE_LIBNAME = libsqlite3_$(TARGET)-$(SQLITE_VER).a
SQLITELIB = sqlite/src/$(SQLITE_LIBNAME)
CFLAGS += -Isqlite/src
else
SQLITELIB = -lsqlite3
endif

PROGFLAGS += -DJSI__SQLITE=1
ifneq ($(ISSHARED),1)
endif
ifeq ($(DB_TEST),1)
PROGFLAGS += -DJSI_DB_TEST=1
endif
endif

ifeq ($(WITH_EXT_MYSQL),1)
PROGLDFLAGS += -lmysqlclient
PROGFLAGS += -DJSI__MYSQL=1
endif

ifneq ($(EXTNAME),)
CFILES += $(EXTNAME).c
PROGFLAGS += -DJSI_USER_EXTENSION=Jsi_Init$(EXTNAME)
endif

ifeq ($(TARGET),win)
# *********** WINDOWS *****************
WIN=1
CFLAGS += -D__USE_MINGW_ANSI_STDIO=1 -Wno-format

# Setup cross-compiler for windows.
ifeq ($(XCPREFIX),)
XCPREFIX=i686-w64-mingw32-
TCPATH := $(shell which $(XCPREFIX)gcc )
ifeq ($(TCPATH),)
XCPREFIX=x86_64-w64-mingw32-
endif
endif

OPT_SOCKET=0
OPT_READLINE=0

EXEEXT=.exe
CFILES += $(WFILES) sqlite/src/sqlite3.c

ifneq ($(WITH_EXT_WEBSOCKET),1)
#for windows without websock use miniz

ifneq ($(JSI__MINIZ),0)
JSI__MINIZ=1
PROGFLAGS += JSI__MINIZ=1
endif

else
#WEBSOCKLIB=lws/build/$(TARGET)/libwebsockets.a
PROGLDFLAGS += $(WEBSOCKLIB) -lwsock32 -lws2_32
endif

STATICLIBS += $(SQLITELIB) $(WEBSOCKLIB)

ifeq ($(JSI__THREADS),1)
PROGLDFLAGS += -lpthread -static
endif

# ***** END WINDOWS *********

else
# *********** UNIX **********************
ifneq ($(LINKSTATIC),1)
EXTRATARGETS += shared
endif

ifneq ($(BUILDSYS),FreeBSD)
ifneq ($(BUILDSYS),Cygwin)
CFLAGS += -frecord-gcc-switches
endif
endif
STATICLIBS += $(SQLITELIB)
PROGLDFLAGS += $(WEBSOCKLIB)

ifeq ($(JSI__LOAD),1)
LNKFLAGS += -rdynamic
COPTS = -fpic
ifneq ($(BUILDSYS),FreeBSD)
PROGLDFLAGS += -ldl
endif
endif

ifeq ($(JSI__THREADS),1)
PROGLDFLAGS += -lpthread
endif

ifneq ($(TARGET),musl)
endif

endif
# *********** END UNIX **********************
ifeq ($(BUILDSYS),FreeBSD)
CC=$(XCPREFIX)cc
else
CC=$(XCPREFIX)gcc
endif
AR=$(XCPREFIX)ar
LD=$(XCPREFIX)ld

ifeq ($(TARGET),musl)
PROGFLAGS += -DJSI__MUSL
CC=musl-gcc
CFLAGS += -D__MUSL__
endif

CCPATH := $(shell which $(CC) )
ifeq ($(CCPATH),)
error:
	@echo "ERROR: Compiler not found '$(CC)': try setting with configure --xcprefix"
endif

ifeq ($(JSI__READLINE),1)
CFILES += src/linenoise.c
HFILES += src/linenoise.h
ifeq ($(JSI__GNUREADLINE),1)
PROGLDFLAGS += -lreadline -lncurses
endif
endif

MINIZDIR=miniz
ifneq ($(JSI__MINIZ),0)
CFILES += $(MINIZDIR)/miniz.c
CFLAGS += -I$(BLDDIR)/$(MINIZDIR)
else
ifneq ($(WIN),1)
PROGLDFLAGS += -lz
endif
endif

PROGLDFLAGS += $(USERLIB)
OBJS    = $(CFILES:.c=.o) $(EFILES:.c=.o)
DEFIN	= 
CFLAGS	+= $(COPTS) $(DEFIN) $(PROGFLAGS) $(OPTS)
YACC	= bison -v
LDFLAGS = -lm $(PROGLDFLAGS) $(EXTRALD)
SHLEXT=.so

ZIPDIR=zipdir
PROGBINMIN = $(which $(DESTDIR)/jsimin)
PROGBINA   = $(DESTDIR)$(PROGRAM)_$(EXEEXT)
PROGBIN	   = $(DESTDIR)$(PROGRAM)$(EXEEXT)

JSI_PKG_DIRS="$(BLDDIR)/lib,$(PREFIX)/lib/jsi"
CFLAGS += -DJSI_PKG_DIRS=\"$(JSI_PKG_DIRS)\"
CFLAGS += -DJSI_CONF_ARGS=\"$(CONF_ARGS)\"

#.PHONY: all clean cleanall remake

all: jsish.c $(ALLTARGETS) $(STATICLIBS) $(PROGBIN) $(EXTRATARGETS) jsidone

help:
	@echo "targets are: mkwin mkmusl shared jsishs stubs ref test testmem release"

src/main.o: .FORCE

.FORCE:

$(OBJS) : $(MAKEFILE) $(MAKECONF)

$(DESTDIR)libjsi.a:
	$(AR) r $(DESTDIR)libjsi.a $(OBJS)

modules: $(BUILDMODS)

$(PROGBINA): jsiminreq src/parser.c $(OBJS) src/main.o $(DESTDIR)libjsi.a 
	$(AR) r $(DESTDIR)libjsi.a $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(SQLITELIB) src/main.o $(LNKFLAGS) -o $(PROGBINA) $(LDFLAGS)
	test -f jsimin || cp $(PROGBINA) jsimin
	$(MAKE) modules

$(DESTDIR)libjsi$(SHLEXT): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -Wl,--export-dynamic  -shared -o $@

$(DESTDIR)libjsish$(SHLEXT): $(OBJS) $(SQLITELIB) $(WEBSOCKLIB)
	$(CC) $(CFLAGS) $(OBJS) $(SQLITELIB) $(WEBSOCKLIB) -Wl,--export-dynamic  -shared -o $@

$(DESTDIR)jsishs$(EXEEXT): src/parser.c $(OBJS) src/main.o
	$(CC) $(CFLAGS) src/main.o -o $@ -L. -Wl,-rpath=`pwd` -L. -ljsish $(LDFLAGS)

shared: $(DESTDIR)libjsi$(SHLEXT) $(DESTDIR)libjsish$(SHLEXT) $(DESTDIR)jsishs$(EXEEXT)

#jsimin:
#ifeq ($(PROGBINMIN),)
#	./configure
#	@echo "Need to rerun make due to re-configure"
#	$(MAKE)
#	exit 0
#endif

$(PROGBIN): $(PROGBINA)  checkjsiminver zhash .FORCE
ifneq ($(JSI__ZIPLIB),1)
	cp -f $(PROGBINA) $(PROGBIN)
else
	rm -f $@
	cp $(PROGBINA) $@
ifneq ($(wildcard .fslckout),) 
	fossil info | grep ^checkout | cut -b15- > lib/sourceid.txt
	touch -r src/jsi.h lib/sourceid.txt
endif
	./jsimin lib/Zip.jsi create $@ $(ZIPDIR) lib
endif
	@echo "Finished $(TARGET) build of '$(PROGBIN)'."

apps: ledger.zip
#apps: sqliteui$(EXEEXT)

ledger.zip: .FORCE
	rm $@
	(cd ../Ledger && zip -r  ../jsi/$@ .)


sqliteui$(EXEEXT):  .FORCE
	cp $(PROGBINA) $@
	./jsimin lib/Zip.jsi create  $@ ../sqliteui lib

lwslib: $(LWSLIB)

$(LWSLIB): $(MAKECONF)
	$(MAKE) -C lws CFLAGS="$(CFLAGS)" CC=$(CC) AR=$(AR) WIN=$(WIN) TARGET=$(TARGET) LWS_MINIZ=$(JSI__MINIZ) LWS_VER=$(LWS_VER) LWS_SSL=$(WITH_SSL) LWS_LIBNAME=$(LWS_LIBNAME)

$(SQLITELIB): sqlite/Makefile  $(MAKECONF)
	$(MAKE) -C sqlite CFLAGS="$(CFLAGS)" CC=$(CC) AR=$(AR) LD=$(LD) WIN=$(WIN) TARGET=$(TARGET) SQLITE_VER=$(SQLITE_VER) SQLITE_LIBNAME=$(SQLITE_LIBNAME)

openssllib: openssl/$(TARGET)/libcypto.a

openssl/$(TARGET)/libcypto.a:  $(MAKECONF)
	$(MAKE) -C openssl CC=$(CC) AR=$(AR) LD=$(LD) WIN=$(WIN) TARGET=$(TARGET)

src/main.o: src/main.c zhash
	$(CC) -c -o $@ $< $(CFLAGS) -DJSI__ZHASH=`cat zhash.txt`

src/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS) 


.FORCE:

depend:
	$(CC) -E -MM -DJSI__WEBSOCKET=1 -DJSI__SQLITE -DJSI__MYSQL $(CFLAGS) $(CFILES) $(EFILES) | sed 's/^\([^ ]\)/src\/\1/' > .depend

-include .depend

# Supported modules (unix only)
mysql: MySql$(SHLEXT)
sqlite: Sqlite$(SHLEXT)
websocket: WebSocket$(SHLEXT)

dbi:
	(cd src && $(CC) `../jsish -c -cflags true DBI.so`)
	
MySql$(SHLEXT): src/jsiMySql.c
	-$(CC) $(CFLAGS) $(PROGFLAGS) $(MODFLAGS) -DJSI__ISMODULE=1 src/jsiMySql.c $(SLIBCFLAGS) -o $@ $(LDFLAGS) -lmysqlclient

Sqlite$(SHLEXT): src/jsiSqlite.c
	-$(CC) $(CFLAGS) $(PROGFLAGS) $(MODFLAGS) -DJSI__ISMODULE=1 src/jsiSqlite.c $(SLIBCFLAGS) -o $@ $(LDFLAGS) -lsqlite3
	
WebSocket$(SHLEXT): src/jsiWebSocket.c
	-$(CC) $(CFLAGS) $(PROGFLAGS) $(MODFLAGS) -DJSI__ISMODULE=1 src/jsiWebSocket.c $(SLIBCFLAGS) -o $@ $(LDFLAGS) -lwebsockets

src/parser.c: src/parser.y
	$(YACC) -osrc/parser.c -d src/parser.y

src/jsiParser.c: src/jsiParser.y
	-lemon src/jsiParser.y

# Create the single amalgamation file jsi.c
jsi.c: src/jsi.h $(REFILES) $(HFILES) $(CFILES) $(MAKEFILE)
	@cat src/jsi.h > $@
	@echo "#ifndef JSI_H_ONLY" >> $@
	@echo "#ifndef JSI_IN_AMALGAMATION" >> $@
	@echo "#define JSI_IN_AMALGAMATION" >> $@
	@echo "#define JSI_AMALGAMATION" >> $@
	@echo "#define JSI__ALL 1" >> $@
	@echo "struct jsi_Pstate;" >> $@
	@cat src/jsiStubs.h $(REFILES) $(HFILES) | grep -v '^#line' >> $@
	@echo "#if JSI__MINIZ" >> $@
	@cat $(MINIZDIR)/miniz.c >> $@
	@echo "#endif //JSI__MINIZ " >> $@
	@echo "#if JSI__READLINE==1" >> $@
	@cat src/linenoise.c >> $@
	@echo "#endif //JSI__READLINE==1" >> $@
	@echo "#ifndef SQLITE_EXTERNAL_ONLY" >> $@
	@cat sqlite/src/sqlite3.c  >> $@
	@echo "#endif //SQLITE_EXTERNAL_ONLY " >> $@
	@cat lws/src/lwsSingle.c  >> $@
	@cat $(WIFILES)  src/jsiCode.c $(PCFILES) | grep -v '^#line' >> $@
	@echo "#ifndef JSI_LITE_ONLY" >> $@
	@grep -v '^#line' $(ACFILES)  >> $@
	@echo "#endif //JSI_LITE_ONLY" >> $@
	@cat $(WFILES) $(EFILES)  >> $@
	@cat src/main.c  >> $@
	@echo "#endif //JSI_IN_AMALGAMATION" >> $@
	@echo "#endif //JSI_H_ONLY" >> $@
    
# Create the single compile file jsish.c
jsish.c: src/jsi.h $(REFILES) $(HFILES) $(CFILES) $(MAKEFILE) $(MAKECONF)
	@echo '#include "src/jsi.h"' > $@
	@echo "#ifndef JSI_IN_AMALGAMATION" >> $@
	@echo "#define JSI_AMALGAMATION" >> $@
	@echo "#define JSI__ALL 1" >> $@
	@echo "struct jsi_Pstate;" >> $@
	@for ii in src/jsiStubs.h $(REFILES) $(HFILES); do echo '#include "'$$ii'"' >> $@; done
	@echo "#if JSI__MINIZ" >> $@
	@echo '#include "'$(MINIZDIR)/miniz.c'"' >> $@
	@echo "#endif //JSI__MINIZ" >> $@
	@echo "#if JSI__READLINE==1" >> $@
	@echo '#include "'src/linenoise.c'"' >> $@
	@echo "#endif //JSI__READLINE==1" >> $@
	@echo "#ifndef SQLITE_VERSION" >> $@
	@echo '#include "sqlite/src/sqlite3.c"'  >> $@
	@echo "#endif //SQLITE_VERSION" >> $@
	@echo "#ifdef __cplusplus" >> $@
	@echo '#include "lws/src/src/lws.h"'  >> $@
	@echo "#else // __cplusplus" >> $@
	@echo '#include "lws/src/lwsSingle.c"'  >> $@
	@echo "#endif //__cplusplus" >> $@
	@for ii in  src/jsiCode.c $(PCFILES); do echo '#include "'$$ii'"' >> $@; done
	@echo "#ifndef JSI_LITE_ONLY" >> $@
	@for ii in $(ACFILES); do echo '#include "'$$ii'"' >> $@; done
	@echo "#endif //JSI_LITE_ONLY" >> $@
	@for ii in $(WFILES) $(EFILES); do echo '#include "'$$ii'"' >> $@; done
	@echo '#include "src/main.c"'  >> $@
	@echo "#endif //JSI_IN_AMALGAMATION" >> $@

stubs:
	(cd src && ../$(PROGBIN) ../tools/mkstubs.jsi)

ref: docs
	./$(PROGBIN) tools/mkproto.jsi > tools/protos.jsi

docs:
	$(MAKE) -C lib/www/md BLDDIR=$(BLDDIR)

release: stubs ref jsi.c jsish.c testsys test

printconf:
	@echo $(EXPECT_CONFIG_VER)

uchroot: src/uchroot.c
	gcc -g -o uchroot src/uchroot.c && sudo chown root.root uchroot && sudo chmod u+s uchroot
	
zhash:
	test -x jsish && (jsish -e 'Util.hash(JSON.stringify(Zvfs.list()), {type:"sha1"})' > zhash.txt) || (echo '"0000000000000000000000000000000000000000"' > zhash.txt)

test:
	./jsish -t tests

testsys:
	tools/testsys.sh

# This requires building with memdebug.
testmem:
	JSI_INTERP_OPTS='{memDebug:1}' ./jsish -t tests

testwall:
	JSI_INTERP_OPTS='{typeCheck:["strict"], strict:true}' ./jsish -t tests

testvg:
	tools/testjs.sh -jsish ./jsish -valgrind tests

tags:
	geany -g -P geany.tags src/*.c src/*.h

install_unsupported: all
	@echo "WARN: install is unsupported"
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/lib/jsi
	mkdir -p $(PREFIX)/include
	cp jsish $(PREFIX)/bin
	cp jsimin $(PREFIX)/bin
	cp src/jsi.h $(PREFIX)/include
	cp src/jsiStubs.h $(PREFIX)/include
	-cp Sqlite$(SHLEXT) $(PREFIX)/lib/jsi
	-cp WebSocket$(SHLEXT) $(PREFIX)/lib/jsi
	-cp Socket$(SHLEXT) $(PREFIX)/lib/jsi
	-cp MySql$(SHLEXT) $(PREFIX)/lib/jsi

remake: clean all

clean:
	$(MAKE) -C sqlite clean SQLITE_LIBNAME=$(SQLITE_LIBNAME)
	$(MAKE) -C lws clean LWS_LIBNAME=$(LWS_LIBNAME)
	$(MAKE) -C c-demos clean
	rm -rf src/*.o *.a jsish $(MINIZDIR)/*.o win/*.o regex/*.o

cleanall: clean
	rm -f $(ACFILES) $(PROGBINA) core src/parser.c src/parser.h src/parser.tab.c jsimin jsish *.so $(PROGBINMIN)
	$(MAKE) -C sqlite cleanall
	$(MAKE) -C lws cleanall
	$(MAKE) -C c-demos cleanall

JSIMINVER=$(shell test -x ./jsimin && ./jsimin -v | cut -d'.' -f1,2)
JSICURVER=$(shell fgrep 'define JSI_VERSION_M' src/jsi.h | cut -b29- | xargs | sed 's/ /./g')

jsidone:
	@echo ""
	@echo "MAKE COMPLETE!!!"
	@echo "To build from Configs/ try 'make remake CONF=X' where X is one of:"
	@echo "    $(JSICONFS)."

jsiminreq:
ifeq ($(JSIMINVER),)
ifneq ($(CONF),default)
	@echo "WARNING!!!: must build with default target before using CONF"
	exit 1
endif
endif

checkjsiminver:
ifneq ($(JSIMINVER),)
ifneq ($(JSIMINVER),$(JSICURVER))
	rm -f jsimin
ifneq ($(JSICHECK),1)
	@echo "WARNING!!!: removed jsimin due to major/minor version mismatch. run make again"
	exit 1
endif
endif
endif


