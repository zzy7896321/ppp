CC = gcc -g -ansi -std=gnu99
AR = ar -cvq

all: libppp.a

libppp.a: parse_module query_module infer_module debug.o
	$(AR) libppp.a parse/parse.o query/query.o infer/infer.o infer/erp.o infer/mh_sampler.o debug.o

.PHONY: parse_module query_module infer_module clean

debug.o: debug.h debug.c
	${CC} -c debug.c -o debug.o

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
	rm flip

flip.exe: flip.c libppp.a
	$(CC) flip.c libppp.a -o flip.exe -lm

flip: flip.c libppp.a
	mkdir -p bin && ${CC} flip.c libppp.a -o bin/flip -lm

simple: simple.c libppp.a
	mkdir -p bin && ${CC} simple.c libppp.a -o bin/simple -lm
