
TESTS = $(addprefix $(bindir)/, lda lda_l linear_regression lda_gen)

$(TESTS): $(bindir)/%: test/%.c libppp.a | $(bindir)
	$(CC) $(CFLAGS) -I./ -o $@ $< libppp.a



