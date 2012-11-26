include packages/base/rules.mk

$(PACKAGES): CCFLAGS+=-Ilib/include
$(PACKAGES): %: %.c $(LIB)
	$(CC) $(CCFLAGS) -o $@ $< $(LIB) $(EXTRA)
