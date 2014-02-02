BIN:=$(PACKAGES_PATH)/help/help
OBJ:=$(PACKAGES_PATH)/help/help.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
