BIN:=$(PACKAGES_PATH)/dump/dump
OBJ:=$(PACKAGES_PATH)/dump/dump.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
