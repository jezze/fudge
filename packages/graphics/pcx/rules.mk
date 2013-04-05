BIN_DATA:=packages/graphics/pcx/pcxdata
OBJ_DATA:=packages/graphics/pcx/pcxdata.o
BIN_CMAP:=packages/graphics/pcx/pcxcmap
OBJ_CMAP:=packages/graphics/pcx/pcxcmap.o

$(BIN_DATA): $(OBJ_DATA) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

$(BIN_CMAP): $(OBJ_CMAP) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN_DATA) $(BIN_CMAP)
PACKAGES_OBJECTS+=$(OBJ_DATA) $(OBJ_CMAP)
