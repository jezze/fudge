include packages/base/rules.mk

packages/%: packages/%.c $(LIBFUDGE)
	$(CC) $(CFLAGS) -Ilibs/fudge/include -o $@ $< $(LIBFUDGE) $(EXTRA)
