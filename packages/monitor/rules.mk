BIN:=$(PACKAGES_PATH)/monitor/monitor
OBJ:=$(PACKAGES_PATH)/monitor/monitor.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
