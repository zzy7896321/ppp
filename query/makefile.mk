
OBJECTS := query.o
OBJS += $(OBJECTS)

QUERY_HEADER := $(wildcard query/*.h) $(COMMON_HEADERS)

$(OBJECTS): %.o: query/%.c $(QUERY_HEADER)
	$(CC) $(CFLAGS) -o $@ -c $<

