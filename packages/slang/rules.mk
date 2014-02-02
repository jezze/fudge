BIN:=$(PACKAGES_PATH)/slang/slang
OBJ:=$(PACKAGES_PATH)/slang/slang.o $(PACKAGES_PATH)/slang/parse.o $(PACKAGES_PATH)/slang/token.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
