BIN:=$(PACKAGES_PATH)/hello/hello
OBJ:=$(PACKAGES_PATH)/hello/hello.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
