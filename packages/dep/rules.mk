BIN:=$(PACKAGES_PATH)/dep/dep
OBJ:=$(PACKAGES_PATH)/dep/dep.o $(PACKAGES_PATH)/dep/parse.o $(PACKAGES_PATH)/dep/token.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
