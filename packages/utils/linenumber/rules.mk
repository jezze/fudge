BIN:=packages/utils/linenumber/linenumber
OBJ:=packages/utils/linenumber/linenumber.o

$(BIN): $(OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
