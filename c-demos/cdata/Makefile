JSIDIR=../../src
OTHER=
CSTRFN=../../lib/Cdata.jsi 
JSISH=../../jsish
#CSTRPP=$(JSISH) $(CSTRFN) 
CSTRPP=$(JSISH) -c
CCGET=$(CSTRPP) -cflags true
DEPS=$(CSTRFN) $(JSIDIR)/jsi.c $(JSIDIR)/jsi.h Makefile

all: shared

shared: Bee.so Car.so Baker.so Bag.so Tiny.so Vimy.so VarTest.so

static: Bee Car Baker

test: shared
	$(JSISH) -u ext_test.jsi
	$(JSISH) -u cdatatest.jsi
	$(JSISH) -u bagtest.jsi
	$(JSISH) -u tinytest.jsi
	$(JSISH) -u vimytest.jsi

Vimy.so: Vimy.c $(DEPS)
	$(CC) `$(CCGET) $@`

Vimy: Vimy.c $(DEPS)
	$(CC) `$(CCGET) $@`

VarTest.so: VarTest.c $(DEPS)
	$(CC) `$(CCGET) $@`

Tiny: Tiny.c $(DEPS)
	$(CC) `$(CCGET) $@`

Tiny.so: Tiny.c $(DEPS)
	$(CC) `$(CCGET) $@`

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

Baker: Baker.c $(DEPS)
	$(CC) `$(CCGET) $@`

Baker.so: Baker.c $(DEPS)
	$(CC) `$(CCGET) $@`

%.c: %.jsc $(DEPS)
	$(CSTRPP) $<

clean:
	rm -f $(CFILES)


-include .depend


