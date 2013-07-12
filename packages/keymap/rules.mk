KEYMAPSE_BIN:=packages/keymap/keymapse
KEYMAPSE_OBJ:=packages/keymap/keymapse.o
KEYMAPUS_BIN:=packages/keymap/keymapus
KEYMAPUS_OBJ:=packages/keymap/keymapus.o

$(KEYMAPSE_BIN): $(KEYMAPSE_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

$(KEYMAPUS_BIN): $(KEYMAPUS_OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(KEYMAPSE_BIN) $(KEYMAPUS_BIN)
PACKAGES_OBJECTS+=$(KEYMAPSE_OBJ) $(KEYMAPUS_OBJ)
