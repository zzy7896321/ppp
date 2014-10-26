
OBJECTS := infer.o rejection.o erp.o execute.o mh_sampler.o 
OBJS += $(OBJECTS)

INFER_HEADERS := $(wildcard infer/*.h) $(COMMON_HEADERS)

$(OBJECTS): %.o: infer/%.c $(INFER_HEADERS) query/query.h
	$(CC) $(CFLAGS) -o $@ -c $<

