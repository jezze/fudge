BIN_ELFLOAD:=\
    $(DIR_SRC)/elf/elfload \

OBJ_ELFLOAD:=\
    $(DIR_SRC)/elf/elfload.o \

DEP_ELFLOAD:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_ELFLOAD): $(OBJ_ELFLOAD) $(DEP_ELFLOAD)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN_ELFUNLOAD:=\
    $(DIR_SRC)/elf/elfunload \

OBJ_ELFUNLOAD:=\
    $(DIR_SRC)/elf/elfunload.o \

DEP_ELFUNLOAD:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_ELFUNLOAD): $(OBJ_ELFUNLOAD) $(DEP_ELFUNLOAD)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(BIN_ELFLOAD) $(BIN_ELFUNLOAD)
OBJ:=$(OBJ) $(OBJ_ELFLOAD) $(OBJ_ELFUNLOAD)
