
OBJECTS := query.o
OBJS += $(OBJECTS)

QUERY_HEADER := $(wildcard query/*.h)

$(OBJECTS): %.o: query/%.c $(QUERY_HEADER) infer/variables.h infer/hash_table.h infer/trace.h
	$(CC) $(CFLAGS) -o $@ -c $<

