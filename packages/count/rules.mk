COUNTC_BIN:=packages/count/countc
COUNTC_OBJ:=packages/count/countc.o
COUNTL_BIN:=packages/count/countl
COUNTL_OBJ:=packages/count/countl.o

$(COUNTC_BIN): $(COUNTC_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

$(COUNTL_BIN): $(COUNTL_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(COUNTC_BIN) $(COUNTL_BIN)
PACKAGES_OBJECTS+=$(COUNTC_OBJ) $(COUNTL_OBJ)
