include packages/base/rules.mk
include packages/net/rules.mk
include packages/test/rules.mk

$(PACKAGES): %: %.c $(LIB)
	$(CC) -s $(CCFLAGS) -o $@ $< $(LIB)

packages/%: CCFLAGS+=-Ilib/include
