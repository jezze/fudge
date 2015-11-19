BIN_PCXCMAP:=\
    $(DIR_SRC)/pcx/pcxcmap \

OBJ_PCXCMAP:=\
    $(DIR_SRC)/pcx/pcx.o \
    $(DIR_SRC)/pcx/pcxcmap.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \
    $(DIR_SRC)/lib/gfx.o \

$(BIN_PCXCMAP): $(OBJ_PCXCMAP) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_PCXDATA:=\
    $(DIR_SRC)/pcx/pcxdata \

OBJ_PCXDATA:=\
    $(DIR_SRC)/pcx/pcx.o \
    $(DIR_SRC)/pcx/pcxdata.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \
    $(DIR_SRC)/lib/gfx.o \

$(BIN_PCXDATA): $(OBJ_PCXDATA) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_PCXCMAP) $(BIN_PCXDATA)
CLEAN:=$(CLEAN) $(BIN_PCXCMAP) $(BIN_PCXDATA) $(OBJ_PCXCMAP) $(OBJ_PCXDATA)
