BIN:=packages/utils/wait/waitkey
OBJ:=packages/utils/wait/waitkey.o

$(BIN): $(OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
