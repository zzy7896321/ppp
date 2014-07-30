CC = gcc -Wall -g -ansi -pedantic-errors

all: infer.o erp.o

infer.o: infer.h infer.c
	$(CC) -c infer.c -o infer.o

erp.o: infer.h erp.c
	$(CC) -c erp.c -o erp.o

clean:
	rm -f *.o
