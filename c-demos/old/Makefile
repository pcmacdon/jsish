JSIROOT=../..
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

all: minimalsh dyn.so prime.so litedemo listdemo demo1 demo2 treedemo

minimalsh: minimal
	rm -f minimalsh
	$(JSIROOT)/jsish -z zvfs minimal

minimal: minimal.c $(ALLDEPS)
	$(CC) $(CFLAGS) -DJSI__SQLITE=1 minimal.c -o minimal $(LDFLAGS)

listdemo: listdemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) listdemo.c -o listdemo $(LDFLAGS)
	
treedemo: treedemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) treedemo.c -o treedemo $(LDFLAGS)


dyn.so: dyn.c $(ALLDEPS)
	$(CC) $(ACFLAGS) $(SHFLAGS) dyn.c -o dyn.so

prime.so: prime.c $(ALLDEPS)
	$(CC) $(ACFLAGS) $(SHFLAGS) prime.c -o prime.so

demo1: demo1.c $(ALLDEPS)
	$(CC) -o demo1 $(CFLAGS) -DJSI_USER_EXTENSION=Jsi_Initdemo1 demo1.c -DJSI__MAIN=1 -DJSI__MINIZ=1 $(JSIROOT)/jsiOne.c $(LDFLAGS)
        
demo2: demo2.c $(ALLDEPS)
	$(CC) -o demo2 $(CFLAGS) -DJSI_USER_EXTENSION=Jsi_Initdemo2 demo2.c -DJSI__MAIN=1 -DJSI__MINIZ=1 $(JSIROOT)/jsiOne.c $(LDFLAGS)

demo2.so: demo2.c $(ALLDEPS)
	$(CC) $(SHFLAGS) -o demo2.so $(CFLAGS) demo2.c -DJSI__MAIN=1 $(LDFLAGS)


litedemo: litedemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) -DJSI__SQLITE=1 litedemo.c -o litedemo $(LDFLAGS)

testmem:
	JSI_INTERP_OPTS='{memDebug:1}' $(MAKE) test

test: all
	./listdemo
	./litedemo
	./dyn.js
	#echo 'demo(1,2);exit(0)' | ./demo1
	echo 'demos.foo(1,2);exit(0)' | ./demo2

clean:
	rm -rf *.so *.o simple listdemo litedemo dbdemo cmds demo1 demo2 minimal minimalsh treedemo
	$(MAKE) -C stubs clean
	$(MAKE) -C cdata clean

cleanall:
