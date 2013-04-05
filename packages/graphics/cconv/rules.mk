BIN_CCONV8TO6:=packages/graphics/cconv/cconv8to6
OBJ_CCONV8TO6:=packages/graphics/cconv/cconv8to6.o

$(BIN_CCONV8TO6): $(OBJ_CCONV8TO6) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN_CCONV8TO6)
PACKAGES_OBJECTS+=$(OBJ_CCONV8TO6)
