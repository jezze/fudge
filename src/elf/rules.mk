BIN_ELFLOAD:=\
    $(DIR_SRC)/elf/elfload \

OBJ_ELFLOAD:=\
    $(DIR_SRC)/elf/elfload.o \

LDLIBS_ELFLOAD:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_ELFLOAD): $(OBJ_ELFLOAD) $(LDLIBS_ELFLOAD)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN_ELFUNLOAD:=\
    $(DIR_SRC)/elf/elfunload \

OBJ_ELFUNLOAD:=\
    $(DIR_SRC)/elf/elfunload.o \

LDLIBS_ELFUNLOAD:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_ELFUNLOAD): $(OBJ_ELFUNLOAD) $(LDLIBS_ELFUNLOAD)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(BIN_ELFLOAD) $(BIN_ELFUNLOAD)
OBJ:=$(OBJ) $(OBJ_ELFLOAD) $(OBJ_ELFUNLOAD)
