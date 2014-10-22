
OBJECTS := infer.o hash_table.o rejection.o erp.o trace.o variables.o execute.o mh_sampler.o 
OBJS += $(OBJECTS)

INFER_HEADERS := $(wildcard infer/*.h)

$(OBJECTS): %.o: infer/%.c $(INFER_HEADERS) query/query.h
	$(CC) $(CFLAGS) -o $@ -c $<

