BIN:=packages/dep/dep
OBJ:=packages/dep/dep.o packages/dep/parse.o packages/dep/token.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
