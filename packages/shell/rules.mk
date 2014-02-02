BIN:=$(PACKAGES_PATH)/shell/shell
OBJ:=$(PACKAGES_PATH)/shell/shell.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
