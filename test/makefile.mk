
TESTS = $(addprefix $(bindir)/, lda lda_l linear_regression lda_gen  \
		naive_gen naive hmm hmm_gen hmm_infer gmm gmm_gen gmm_infer)

$(TESTS): $(bindir)/%: test/%.c libppp.a | $(bindir)
	$(CC) $(CFLAGS) -I./ -o $@ $< libppp.a -lm



