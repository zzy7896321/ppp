CC = gcc -Wall -g -ansi -pedantic-errors -std=gnu99

query.o: query.h query.c
	$(CC) -c query.c -o query.o

clean:
	rm -f *.o

test: query.o test.c
	$(CC) test.c query.o -o test
