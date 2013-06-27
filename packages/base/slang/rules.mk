BIN:=packages/base/slang/slang
OBJ:=packages/base/slang/slang.o
OBJ+=packages/base/slang/token.o

$(BIN): $(OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
