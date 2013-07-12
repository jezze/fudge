NAMED_BIN:=packages/base/name/named
NAMED_OBJ:=packages/base/name/named.o
NAMEF_BIN:=packages/base/name/namef
NAMEF_OBJ:=packages/base/name/namef.o

$(NAMED_BIN): $(NAMED_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

$(NAMEF_BIN): $(NAMEF_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(NAMED_BIN) $(NAMEF_BIN)
PACKAGES_OBJECTS+=$(NAMED_OBJ) $(NAMEF_OBJ)
