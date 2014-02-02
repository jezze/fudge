BIN:=$(PACKAGES_PATH)/mount/mount
OBJ:=$(PACKAGES_PATH)/mount/mount.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
