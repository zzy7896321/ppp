CC = gcc -Wall -g -ansi -pedantic-errors
AR = ar -cvq

all: libppp.a

libppp.a: parse_module query_module infer_module
	$(AR) libppp.a parse/parse.o query/query.o infer/infer.o infer/erp.o

.PHONY: parse_module query_module infer_module clean

parse_module:
	$(MAKE) -C parse

query_module:
	$(MAKE) -C query

infer_module:
	$(MAKE) -C infer

clean:
	rm -rf libppp.a *.o *.exe *.dSYM
	$(MAKE) -C parse clean
	$(MAKE) -C query clean
	$(MAKE) -C infer clean

flip.exe: flip.c libppp.a
	$(CC) flip.c libppp.a -o flip.exe -lm
	
lda.exe: lda.c libppp.a
	$(CC) lda.c libppp.a -o lda.exe -lm
