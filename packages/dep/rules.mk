BIN:=packages/dep/dep
OBJ:=packages/dep/dep.o
OBJ+=packages/dep/parse.o
OBJ+=packages/dep/token.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
