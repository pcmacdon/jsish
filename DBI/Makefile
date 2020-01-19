CDCMD=../jsish -c
#CDCMD=../jsish ../lib/CData.jsi
#CDOPTS= -noline true
#CC=cc -fsanitize=address 

DBI.so: DBI.c
	$(CC)  `$(CDCMD) -cflags true DBI.so` -ldbi

DBI.c: DBI.jsc DBIinc.c
	$(CDCMD) $(CDOPTS)  DBI.jsc

DBIsh: DBI.c
	$(CC)  `$(CDCMD) -cflags true DBI` -ldbi
	rm -f DBIsh
	jsish -z zvfs DBI

test:
	../jsish --U dbi_mysql.jsi

clean:
	rm -f DBI.c DBI.so DBI DBIsh
