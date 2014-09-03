#CC = gcc -Wall -g -ansi -pedantic-errors -std=gnu99 -DDEBUG
#CC = gcc -Wall -g -ansi -pedantic-errors -std=gnu99

## CC should be exported

all: infer.o erp.o hash_table.o rejection.o trace.o variables.o execute.o mh_sampler.o

infer.o: infer.h infer.c
	$(CC) -c infer.c -o infer.o

erp.o: infer.h erp.c
	$(CC) -c erp.c -o erp.o

mh_sampler.o: mh_sampler.h mh_sampler.c
	${CC} -c mh_sampler.c -o mh_sampler.o

rejection.o: rejection.h rejection.c
	${CC} -c rejection.c -o rejection.o

trace.o: trace.c trace.h
	${CC} -c trace.c -o trace.o

variables.o: variables.c variables.h
	${CC} -c variables.c -o trace.o

hash_table.o: hash_table.c hash_table.h hash_table_impl.h
	${CC} -c hash_table.c -o hash_table.o

execute.o: execute.h execute.c
	${CC} -c execute.c -o execute.o

clean:
	rm -f *.o
