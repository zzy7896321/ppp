
TESTS = $(addprefix $(bindir)/, flip lda mix two_coin biased_coin)

$(TESTS): $(bindir)/%: test/%.c libppp.a | $(bindir)
	$(CC) $(CFLAGS) -I./ -o $@ $< libppp.a



