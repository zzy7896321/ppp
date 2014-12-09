
OBJECTS := query.o string_query.o observation.o
OBJS += $(OBJECTS)

QUERY_HEADER := $(wildcard query/*.h) $(COMMON_HEADERS)

$(OBJECTS): %.o: query/%.c $(QUERY_HEADER)
	$(CC) $(CFLAGS) -o $@ -c $<

