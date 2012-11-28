include packages/base/rules.mk

packages/%: packages/%.c $(LIB)
	$(CC) $(CFLAGS) -Ilib/include -o $@ $< $(LIB) $(EXTRA)
