BIN:=$(PACKAGES_PATH)/sleep/sleep
OBJ:=$(PACKAGES_PATH)/sleep/sleep.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
