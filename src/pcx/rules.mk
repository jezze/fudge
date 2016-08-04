BIN_PCXCMAP:=\
    $(DIR_SRC)/pcx/pcxcmap \

OBJ_PCXCMAP:=\
    $(DIR_SRC)/pcx/gfx.o \
    $(DIR_SRC)/pcx/pcx.o \
    $(DIR_SRC)/pcx/pcxcmap.o \

LDLIBS_PCXCMAP:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

$(BIN_PCXCMAP): $(OBJ_PCXCMAP) $(LDLIBS_PCXCMAP)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN_PCXDATA:=\
    $(DIR_SRC)/pcx/pcxdata \

OBJ_PCXDATA:=\
    $(DIR_SRC)/pcx/gfx.o \
    $(DIR_SRC)/pcx/pcx.o \
    $(DIR_SRC)/pcx/pcxdata.o \

LDLIBS_PCXDATA:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

$(BIN_PCXDATA): $(OBJ_PCXDATA) $(LDLIBS_PCXDATA)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(BIN_PCXCMAP) $(BIN_PCXDATA)
OBJ:=$(OBJ) $(OBJ_PCXCMAP) $(OBJ_PCXDATA)
