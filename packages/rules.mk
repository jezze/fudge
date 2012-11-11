include packages/base/rules.mk
include packages/net/rules.mk
include packages/test/rules.mk

$(PACKAGES): CCFLAGS+=-Ilib/include
$(PACKAGES): %: %.c $(LIB)
	$(CC) $(CCFLAGS) -o $@ $< $(LIB) $(EXTRA)
