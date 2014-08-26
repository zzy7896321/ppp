CC = gcc -g -ansi -std=gnu99 -DDEBUG
AR = ar -cvq

export ${CC}

#all: libppp.a
all: flip

PARSE_OBJ = parse/parse.o parse/list.o parse/ilist.o parse/symbol_table.o parse/interface.o
INFER_OBJ = infer/infer.o infer/hash_table.o infer/rejection.o infer/erp.o infer/trace.o infer/variables.o

libppp.a: parse_module query_module infer_module debug.o
	$(AR) libppp.a ${PARSE_OBJ} query/query.o ${INFER_OBJ} debug.o

.PHONY: parse_module query_module infer_module clean

debug.o: debug.h debug.c
	${CC} -c debug.c -o debug.o

parse_module: ${PARSE_OBJ}
	$(MAKE) -C parse

query_module:
	$(MAKE) -C query

infer_module: ${INFER_OBJ}
	$(MAKE) -C infer

clean:
	rm -rf libppp.a *.o *.exe *.dSYM
	$(MAKE) -C parse clean
	$(MAKE) -C query clean
	$(MAKE) -C infer clean
	rm -rf bin/

flip: flip.c libppp.a
	mkdir -p bin && ${CC} flip.c libppp.a -o bin/flip -lm

