BIN_ELFLOAD:=\
    $(DIR_SRC)/elf/elfload \

OBJ_ELFLOAD:=\
    $(DIR_SRC)/elf/elfload.o \
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_ELFLOAD): $(OBJ_ELFLOAD)
	@echo LD $@ 
	@$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

BIN_ELFUNLOAD:=\
    $(DIR_SRC)/elf/elfunload \

OBJ_ELFUNLOAD:=\
    $(DIR_SRC)/elf/elfunload.o \
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_ELFUNLOAD): $(OBJ_ELFUNLOAD)
	@echo LD $@ 
	@$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

BIN:=$(BIN) $(BIN_ELFLOAD) $(BIN_ELFUNLOAD)
OBJ:=$(OBJ) $(OBJ_ELFLOAD) $(OBJ_ELFUNLOAD)
