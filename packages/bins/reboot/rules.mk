BIN:=$(PACKAGES_BINS_PATH)/reboot/reboot
OBJ:=$(PACKAGES_BINS_PATH)/reboot/reboot.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
