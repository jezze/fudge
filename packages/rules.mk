include packages/base/rules.mk

packages/%.o: packages/%.c
	$(CC) -c $(CFLAGS) -Ilibs/fudge -o $@ $<
