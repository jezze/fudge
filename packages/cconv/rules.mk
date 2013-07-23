BIN:=packages/cconv/cconv8to6
OBJ:=packages/cconv/cconv8to6.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
