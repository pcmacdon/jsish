JSIROOT=..
ACFLAGS= -g -Wall -I$(JSIROOT) -I$(JSIROOT)/src
CFLAGS=$(ACFLAGS)
LIBJSI=libjsish.a
JSISH=$(JSIROOT)/jsish
ALDFLAGS=-lm -ldl -lpthread
-include $(JSIROOT)/make.conf

HASMYSQL=$(shell $(JSIROOT)/jsish -e 'return(MySql!==undefined);')
ifeq ($(HASMYSQL),true)
ALDFLAGS+=-lmysqlclient
endif

HASLIBSAN=$(shell ldd $(JSISH) | grep libasan)
ifneq ($(HASLIBSAN),)
CFLAGS+=-fsanitize=address
endif

LDFLAGS=$(LIBJSI) $(ALDFLAGS)
SHFLAGS=-shared -fpic 
ALLDEPS=$(JSIROOT)/jsi.c $(JSIROOT)/libjsish.so $(JSIROOT)/src/jsi.h Makefile

SLDFLAGS=-Wl,-rpath=`pwd`/$(JSIROOT) -L$(JSIROOT) -ljsish $(ALDFLAGS) 

all: basic cmds dbdemo stubs dyn.so dyno.so dynspec.so

libjsi.a: $(JSIROOT)/jsi.c
	$(CC) $(CFLAGS) $< -c -o jsi.o
	$(AR) rv libjsi.a jsi.o

libjsish.a: $(JSIROOT)/jsish.c
	$(CC) $(CFLAGS) $< -I.. -c -o jsish.o
	$(AR) rv libjsish.a jsish.o

libjsish.so: $(JSIROOT)/jsish.c
	$(CC) $(ACFLAGS) $(SHFLAGS) $< -rdynamic -o $@

jsishs: $(JSIROOT)/jsish.c
	$(CC) $(ACFLAGS) -fpic -DJSI__MAIN=1 $< -rdynamic -o $@ -lm -ldl -lpthread
	jsish -z create $@ $(JSIROOT)/lib

cmds: cmds.c $(ALLDEPS)
	$(CC) $(CFLAGS) -o $@ $< $(SLDFLAGS)

dbdemo: dbdemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) -o $@ $< $(SLDFLAGS)

basic: basic.c $(ALLDEPS)
	$(CC) $(CFLAGS) -o $@ $< $(SLDFLAGS)

dyn.so: dyn.c $(ALLDEPS)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

dyno.so: dyno.c $(ALLDEPS)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

dynspec.so: dynspec.c $(ALLDEPS)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

stubs:
	$(MAKE) -C stubs
	
testmem:
	JSI_INTERP_OPTS='{memDebug:1}' $(MAKE) test

test: all
	./basic
	./cmds
	$(JSISH) --T dyn-test.jsi
	$(JSISH) --T dyno-test.jsi
	$(JSISH) --T dynspec-test.jsi
	echo 'exit(0);' | ./dbdemo -benchmark
	$(MAKE) -C stubs test
	$(MAKE) -C cextn

clean:
	rm -rf *.so *.o simple listdemo litedemo dbdemo cmds demo1 demo2 minimal minimalsh treedemo
	$(MAKE) -C stubs clean
	$(MAKE) -C cextn clean

cleanall:

.PHONY: all clean cleanall stubs testmem
