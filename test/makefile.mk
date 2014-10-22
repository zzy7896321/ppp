
TESTS = $(addprefix $(bindir)/, flip lda)

$(TESTS): $(bindir)/%: test/%.c libppp.a | $(bindir)
	$(CC) $(CFLAGS) -I./ -o $@ $< libppp.a



