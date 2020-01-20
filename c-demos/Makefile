JSIDIR=../src
ACFLAGS= -g -Wall -I$(JSIDIR) -I.. -DJSI__MEMDEBUG=1
CFLAGS= $(ACFLAGS)  -DJSI__READLINE=1 -I../sqlite/src -L../sqlite/build/unix
LDFLAGS=-lm -ldl -lpthread
SHFLAGS=-shared -fpic 
ALLDEPS=$(JSIDIR)/jsi.c $(JSIDIR)/jsiOne.c Makefile

all: simple minimalsh dyn.so prime.so dbdemo litedemo listdemo cmds demo1 demo2 treedemo

simple: simple.c $(ALLDEPS)
	$(CC) $(CFLAGS) -DJSI__SQLITE=1 simple.c -o simple -lm -ldl -lpthread -lsqlite3

minimalsh: minimal
	rm -f minimalsh
	jsish -z zvfs minimal

minimal: minimal.c $(ALLDEPS)
	$(CC) $(CFLAGS) -DJSI__SQLITE=1 minimal.c -o minimal -lm -ldl -lpthread -lsqlite3

cmds: cmds.c $(JSIDIR)/jsi.c
	$(CC) $(CFLAGS) cmds.c -o cmds -lm -ldl -lpthread

listdemo: listdemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) listdemo.c -o listdemo -lm -ldl -lpthread
	
treedemo: treedemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) treedemo.c -o treedemo -lm -ldl -lpthread

dbdemo: dbdemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) -DJSI__SQLITE=1 dbdemo.c -o dbdemo -lm -ldl -lpthread -lsqlite3

dyn.so: dyn.c $(ALLDEPS)
	$(CC) $(ACFLAGS) -I$(JSIDIR) $(SHFLAGS) dyn.c -o dyn.so

prime.so: prime.c $(ALLDEPS)
	$(CC) $(CFLAGS) $(SHFLAGS) prime.c -o prime.so

demo1: demo1.c $(ALLDEPS)
	$(CC) -o demo1 $(CFLAGS) -DJSI_USER_EXTENSION=Jsi_Initdemo1 demo1.c -DJSI__MAIN=1 -DJSI__MINIZ=1 $(JSIDIR)/jsiOne.c $(LDFLAGS)
        
demo2: demo2.c $(ALLDEPS)
	$(CC) -o demo2 $(CFLAGS) -DJSI_USER_EXTENSION=Jsi_Initdemo2 demo2.c -DJSI__MAIN=1 -DJSI__MINIZ=1 $(JSIDIR)/jsiOne.c $(LDFLAGS)

demo2.so: demo2.c $(ALLDEPS)
	$(CC) $(SHFLAGS) -o demo2.so $(CFLAGS) demo2.c -DJSI__MAIN=1 $(LDFLAGS)


litedemo: litedemo.c $(ALLDEPS)
	$(CC) $(CFLAGS) -DJSI__SQLITE=1 litedemo.c -o litedemo -lsqlite3 -lm -lpthread -ldl

testmem:
	JSI_INTERP_OPTS='{memDebug:1}' $(MAKE) test

test: all
	./listdemo
	./litedemo
	./simple
	echo 'exit(0);' | ./dbdemo -benchmark
	./dyn.js
	#echo 'demo(1,2);exit(0)' | ./demo1
	echo 'demos.foo(1,2);exit(0)' | ./demo2
	$(MAKE) -C stubs
	$(MAKE) -C stubs test

clean:
	rm -rf *.so *.o simple listdemo litedemo dbdemo cmds demo1 demo2 minimal minimalsh treedemo
	$(MAKE) -C stubs clean
	$(MAKE) -C cdata clean

cleanall:
