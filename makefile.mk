
OBJECTS := debug.o
OBJS += $(OBJECTS)

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

