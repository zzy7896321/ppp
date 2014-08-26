#CC = gcc -Wall -g -ansi -pedantic-errors -std=gnu99

.PHONY: test clean

parse.o: parse.h parse.c list.h ilist.h symbol_table.h
	$(CC) -c parse.c -o parse.o

interface.o: interface.h interface.c list.h ilist.h symbol_table.h parse.h
	${CC} -c interface.c -o interface.o

ilist.o: ilist.h ilist.c
	${CC} -c ilist.c -o ilist.o

list.o: list.h list.c
	${CC} -c list.c -o list.o

symbol_table.o: symbol_table.h symbol_table.c
	${CC} -c symbol_table.c -o symbol_table.o

test: test.c parse.o interface.o ilist.o list.o symbol_table.o
	${CC} test.c parse.o interface.o ilist.o list.o symbol_table.o ../infer/erp.c -o test
#	./test.exe models/flip.model
#	./test.exe models/normal.model
#	./test.exe models/logistic.model
#	./test.exe models/bayesian.model
#	./test.exe models/hmm.model

clean:
	rm -rf *.o *.exe *.dSYM
	rm -f test
