CC = gcc -Wall -g -ansi -pedantic-errors

.PHONY: test clean

parse.o: parse.h parse.c
	$(CC) -c parse.c -o parse.o

test: test.exe
	./test.exe models/flip.model
	./test.exe models/normal.model
	./test.exe models/logistic.model
	./test.exe models/bayesian.model
	./test.exe models/hmm.model

test.exe: test.c parse.o
	$(CC) test.c parse.o -o test.exe

clean:
	rm -rf *.o *.exe *.dSYM
