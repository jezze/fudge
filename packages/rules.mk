include packages/base/rules.mk
include packages/net/rules.mk
include packages/test/rules.mk

packages/%: CCFLAGS+=-Ilib/include

$(PACKAGES): %: %.c $(LIB)
	$(CC) -s $(CCFLAGS) -o $@ $< $(LIB) $(EXTRA)
