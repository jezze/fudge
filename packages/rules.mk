include packages/base/rules.mk

packages/%: packages/%.c
	$(CC) $(CFLAGS) -Ilib/include -o $@ $< $(LIB) $(EXTRA)
