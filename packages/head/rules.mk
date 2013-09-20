HEADB_BIN:=packages/head/headb
HEADB_OBJ:=packages/head/headb.o
HEADL_BIN:=packages/head/headl
HEADL_OBJ:=packages/head/headl.o

$(HEADB_BIN): $(HEADB_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

$(HEADL_BIN): $(HEADL_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(HEADB_BIN) $(HEADL_BIN)
PACKAGES_OBJECTS+=$(HEADB_OBJ) $(HEADL_OBJ)
