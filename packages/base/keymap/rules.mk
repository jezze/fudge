BIN_KEYMAPSE:=packages/base/keymap/keymapse
OBJ_KEYMAPSE:=packages/base/keymap/keymapse.o
BIN_KEYMAPUS:=packages/base/keymap/keymapus
OBJ_KEYMAPUS:=packages/base/keymap/keymapus.o

$(BIN_KEYMAPSE): $(OBJ_KEYMAPSE) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

$(BIN_KEYMAPUS): $(OBJ_KEYMAPUS) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN_KEYMAPSE) $(BIN_KEYMAPUS)
PACKAGES_OBJECTS+=$(OBJ_KEYMAPSE) $(OBJ_KEYMAPUS)
