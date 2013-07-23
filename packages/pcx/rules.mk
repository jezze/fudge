PCXDATA_BIN:=packages/pcx/pcxdata
PCXDATA_OBJ:=packages/pcx/pcxdata.o
PCXCMAP_BIN:=packages/pcx/pcxcmap
PCXCMAP_OBJ:=packages/pcx/pcxcmap.o

$(PCXDATA_BIN): $(PCXDATA_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

$(PCXCMAP_BIN): $(PCXCMAP_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(PCXDATA_BIN) $(PCXCMAP_BIN)
PACKAGES_OBJECTS+=$(PCXDATA_OBJ) $(PCXCMAP_OBJ)
