BIN:=packages/desktop/wm/wm
OBJ:=packages/desktop/wm/wm.o

$(BIN): $(OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
