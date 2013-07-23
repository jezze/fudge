MODLOAD_BIN:=packages/mod/modload
MODLOAD_OBJ:=packages/mod/modload.o
MODUNLOAD_BIN:=packages/mod/modunload
MODUNLOAD_OBJ:=packages/mod/modunload.o

$(MODLOAD_BIN): $(MODLOAD_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODUNLOAD_BIN): $(MODUNLOAD_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(MODLOAD_BIN) $(MODUNLOAD_BIN)
PACKAGES_OBJECTS+=$(MODLOAD_OBJ) $(MODUNLOAD_OBJ)
