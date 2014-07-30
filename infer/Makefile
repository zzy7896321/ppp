#CC = gcc -Wall -g -ansi -pedantic-errors -std=gnu99 -DDEBUG
CC = gcc -Wall -g -ansi -pedantic-errors -std=gnu99


all: infer.o erp.o mh_sampler.o

infer.o: infer.h infer.c
	$(CC) -c infer.c -o infer.o

erp.o: infer.h erp.c
	$(CC) -c erp.c -o erp.o

mh_sampler.o: mh_sampler.h mh_sampler.c
	${CC} -c mh_sampler.c -o mh_sampler.o

clean:
	rm -f *.o
