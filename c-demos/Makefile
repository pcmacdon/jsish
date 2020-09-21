JSIROOT=..
ACFLAGS= -g -Wall -I$(JSIROOT) -I$(JSIROOT)/src
CFLAGS=$(ACFLAGS)
LIBJSI=libjsish.a
ALDFLAGS=-lm -ldl -lpthread

HASMYSQL=$(shell $(JSIROOT)/jsish -e 'return(MySql!==undefined);')
ifeq ($(HASMYSQL),true)
ALDFLAGS+=-lmysqlclient
endif

HASLIBSAN=$(shell ldd $(JSIROOT)/jsish | grep libasan)
ifneq ($(HASLIBSAN),)
CFLAGS+=-fsanitize=address
endif

LDFLAGS=$(LIBJSI) $(ALDFLAGS)
SHFLAGS=-shared -fpic 
ALLDEPS=$(JSIROOT)/jsi.c $(JSIROOT)/libjsish.so $(JSIROOT)/src/jsi.h Makefile

SLDFLAGS=-Wl,-rpath=`pwd`/$(JSIROOT) -L$(JSIROOT) -ljsish $(ALDFLAGS) 

all: basic cmds dbdemo stubs

libjsi.a: $(JSIROOT)/jsi.c
	$(CC) $(CFLAGS) $< -c -o jsi.o
	$(AR) rv libjsi.a jsi.o

libjsish.a: $(JSIROOT)/jsish.c
	$(CC) $(CFLAGS) $< -I.. -c -o jsish.o
	$(AR) rv libjsish.a jsish.o

libjsish.so: $(JSIROOT)/jsish.c
	$(CC) $(ACFLAGS) $(SHFLAGS) $< -rdynamic -o $@

jsish: $(JSIROOT)/jsish.c
	$(CC) $(ACFLAGS) -fpic -DJSI__MAIN=1 $< -rdynamic -o $@ -lm -ldl -lpthread
	jsish -z create jsish $(JSIROOT)/lib

cmds: cmds.c $(ALLDEPS)
	$(CC) $(CFLAGS) -o $@ $< $(SLDFLAGS)

dbdemo: dbdemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) -o $@ $< $(SLDFLAGS)

basic: basic.c $(ALLDEPS)
	$(CC) $(CFLAGS) -o $@ $< $(SLDFLAGS)

stubs:
	$(MAKE) -C stubs
	
testmem:
	JSI_INTERP_OPTS='{memDebug:1}' $(MAKE) test

test: all
	./basic
	./cmds
	echo 'exit(0);' | ./dbdemo -benchmark
	$(MAKE) -C stubs test
	$(MAKE) -C cextn

clean:
	rm -rf *.so *.o simple listdemo litedemo dbdemo cmds demo1 demo2 minimal minimalsh treedemo
	$(MAKE) -C stubs clean
	$(MAKE) -C cextn clean

cleanall:

.PHONY: all clean cleanall stubs testmem
