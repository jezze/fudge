include packages/base/rules.mk
include packages/desktop/rules.mk
include packages/graphics/rules.mk
include packages/utils/rules.mk

packages/%.o: packages/%.c
	$(CC) -c $(CFLAGS) -Ilibs/fudge -o $@ $<
