TOP=../..
CFLAGS= -g -Wall -I$(TOP)/src
LDFLAGS=-shared -fpic 
CCF=$(CC) $(CFLAGS) $(LDFLAGS)
demo:
	$(TOP)/jsish $(TOP)/tools/mkstubs.jsi prefix demo
	$(CCF) -c -DJSI_USE_STUBS demo.c -o demo.o
	$(CCF) -c -DJSI_USE_STUBS demoStubs.c -o demoStubs.o
	$(CCF) -shared -fpic demo.o demoStubs.o -o demo.so
	$(CCF) -DJSI_USE_STUBS -DDEMO_USE_STUBS user.c -o user.so

test:
	$(TOP)/jsish teststubs.jsi
clean:
	rm -f *.so *.o

