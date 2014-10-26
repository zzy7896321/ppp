
OBJECTS := parse.o interface.o
OBJS += $(OBJECTS)

PARSE_HEADER := $(wildcard parse/*.h) $(COMMON_HEADERS)


$(OBJECTS): %.o: parse/%.c $(PARSE_HEADER)
	$(CC) $(CFLAGS) -o $@ -c $<

