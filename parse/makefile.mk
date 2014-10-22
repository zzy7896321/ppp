
OBJECTS := parse.o list.o ilist.o symbol_table.o interface.o
OBJS += $(OBJECTS)

PARSE_HEADER := $(wildcard parse/*.h)


$(OBJECTS): %.o: parse/%.c $(PARSE_HEADER)
	$(CC) $(CFLAGS) -o $@ -c $<

