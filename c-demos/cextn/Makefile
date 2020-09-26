JSIROOT=../..
OTHER=
CSTRFN=../../lib/Cextn.jsi 
JSISH=../../jsish
NOLINE=false
#CSTRPP=$(JSISH) $(CSTRFN) 
CSTRPP=$(JSISH) -c -noline $(NOLINE)
CCGET=$(CSTRPP) -cflags true
DEPS=$(CSTRFN) $(JSIROOT)/jsish.c $(JSIROOT)/src/jsi.h Makefile
STATICS=Bee Car Baker

all: shared

shared: Tiny.so Bee.so Car.so Baker.so Bag.so Sum.so

static: $(STATICS)

test: shared
	$(JSISH) -t ext_test.jsi
	$(JSISH) -t bagtest.jsi
	$(JSISH) -t tinytest.jsi
	$(JSISH) -t logtest.jsi

Tiny: Tiny.c $(DEPS)
	$(CC) `$(CCGET) $@`

Tiny.so: Tiny.c $(DEPS)
	$(CC) `$(CCGET) $@`

Tinysh: Tiny.c $(DEPS)
	$(CC) `jsish -c -cflags true Tiny`
	jsish -z zvfs Tiny

Bag: Bag.c $(DEPS)
	$(CC) `$(CCGET) $@`

Bag.so: Bag.c $(DEPS)
	$(CC) `$(CCGET) $@`

Bee: Bee.c $(DEPS)
	$(CC) `$(CCGET) $@`

Bee.so: Bee.c $(DEPS)
	$(CC) `$(CCGET) $@`

Car: Car.c $(DEPS)
	$(CC) `$(CCGET) $@`

Car.so: Car.c $(DEPS)
	$(CC) `$(CCGET) $@`

Carsh: Car
	jsish -z zvfs Car

Sum.so: Sum.c $(DEPS)
	$(CC) `$(CCGET) $@`

Baker: Baker.c $(DEPS)
	$(CC) `$(CCGET) $@`

Baker.so: Baker.c $(DEPS)
	$(CC) `$(CCGET) $@`

%.c: %.jsc $(DEPS)
	$(CSTRPP) $<

clean:
	rm -f *.c *.so Carsh $(STATICS)

cleanall:

-include .depend


