NAMED_BIN:=packages/name/named
NAMED_OBJ:=packages/name/named.o
NAMEF_BIN:=packages/name/namef
NAMEF_OBJ:=packages/name/namef.o

$(NAMED_BIN): $(NAMED_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

$(NAMEF_BIN): $(NAMEF_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(NAMED_BIN) $(NAMEF_BIN)
PACKAGES_OBJECTS+=$(NAMED_OBJ) $(NAMEF_OBJ)
