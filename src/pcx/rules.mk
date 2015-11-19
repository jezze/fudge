BIN_PCXCMAP:=\
    $(SRC_PATH)/pcx/pcxcmap \

OBJ_PCXCMAP:=\
    $(SRC_PATH)/pcx/pcx.o \
    $(SRC_PATH)/pcx/pcxcmap.o \
    $(SRC_PATH)/fudge/memory.o \
    $(SRC_PATH)/lib/file.o \
    $(SRC_PATH)/lib/gfx.o \

$(BIN_PCXCMAP): $(OBJ_PCXCMAP) $(BIN_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_PCXDATA:=\
    $(SRC_PATH)/pcx/pcxdata \

OBJ_PCXDATA:=\
    $(SRC_PATH)/pcx/pcx.o \
    $(SRC_PATH)/pcx/pcxdata.o \
    $(SRC_PATH)/fudge/memory.o \
    $(SRC_PATH)/lib/file.o \
    $(SRC_PATH)/lib/gfx.o \

$(BIN_PCXDATA): $(OBJ_PCXDATA) $(BIN_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_PCXCMAP) $(BIN_PCXDATA)
CLEAN:=$(CLEAN) $(BIN_PCXCMAP) $(BIN_PCXDATA) $(OBJ_PCXCMAP) $(OBJ_PCXDATA)
