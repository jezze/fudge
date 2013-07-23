BIN:=packages/slang/slang
OBJ:=packages/slang/slang.o
OBJ+=packages/slang/token.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
