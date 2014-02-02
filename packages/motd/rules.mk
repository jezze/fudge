BIN:=$(PACKAGES_PATH)/motd/motd
OBJ:=$(PACKAGES_PATH)/motd/motd.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
