JSIDIR=..
CFLAGS= -g -Wall -I$(JSIDIR)
CFLAGS += -Wc++-compat
LDFLAGS=-shared -fpic 

all: simple dyn.so prime.so dbdemo litedemo listdemo

simple: simple.c
	make -C$(JSIDIR) jsi.c
	$(CC) $(CFLAGS) -DHAVE_SQLITE=1 simple.c -o simple -lz -lm -ldl -lpthread -lsqlite3

listdemo: listdemo.c
	make -C$(JSIDIR) jsi.c
	$(CC) $(CFLAGS) listdemo.c -o listdemo -lz -lm -ldl -lpthread

dbdemo: dbdemo.c
	make -C$(JSIDIR) jsi.c
	$(CC) $(CFLAGS) -DHAVE_SQLITE=1 dbdemo.c -o dbdemo -lz -lm -ldl -lpthread -lsqlite3

dyn.so: dyn.c
	$(CC) $(CFLAGS) $(LDFLAGS) dyn.c -o dyn.so

prime.so: prime.c
	$(CC) $(CFLAGS) $(LDFLAGS) prime.c -o prime.so

litedemo: litedemo.c
	make -C$(JSIDIR) jsi.c
	$(CC) $(CFLAGS) -DHAVE_SQLITE=1 litedemo.c -o litedemo -lm -lsqlite3

clean:
	rm -rf *.so *.o simple listdemo litedemo dbdemo

