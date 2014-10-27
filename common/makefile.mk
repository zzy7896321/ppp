
OBJECTS := trace.o variables.o ilist.o list.o symbol_table.o mem_profile.o
OBJS += $(OBJECTS)

COMMON_HEADERS := $(wildcard common/*.h)

$(OBJECTS): %.o: common/%.c $(INFER_HEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<
	