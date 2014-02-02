BIN:=$(PACKAGES_PATH)/echo/echo
OBJ:=$(PACKAGES_PATH)/echo/echo.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
