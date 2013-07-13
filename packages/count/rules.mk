COUNTB_BIN:=packages/count/countb
COUNTB_OBJ:=packages/count/countb.o
COUNTL_BIN:=packages/count/countl
COUNTL_OBJ:=packages/count/countl.o

$(COUNTB_BIN): $(COUNTB_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

$(COUNTL_BIN): $(COUNTL_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(COUNTB_BIN) $(COUNTL_BIN)
PACKAGES_OBJECTS+=$(COUNTB_OBJ) $(COUNTL_OBJ)
