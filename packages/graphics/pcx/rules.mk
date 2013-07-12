PCXDATA_BIN:=packages/graphics/pcx/pcxdata
PCXDATA_OBJ:=packages/graphics/pcx/pcxdata.o
PCXCMAP_BIN:=packages/graphics/pcx/pcxcmap
PCXCMAP_OBJ:=packages/graphics/pcx/pcxcmap.o

$(PCXDATA_BIN): $(PCXDATA_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

$(PCXCMAP_BIN): $(PCXCMAP_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(PCXDATA_BIN) $(PCXCMAP_BIN)
PACKAGES_OBJECTS+=$(PCXDATA_OBJ) $(PCXCMAP_OBJ)
