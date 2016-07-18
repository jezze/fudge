BIN_ELFLOAD:=\
    $(DIR_SRC)/elf/elfload \

OBJ_ELFLOAD:=\
    $(DIR_SRC)/elf/elf.o \
    $(DIR_SRC)/elf/elfload.o \
    $(DIR_SRC)/lib/file.o \
    $(DIR_SRC)/fudge/fudge.a \

$(BIN_ELFLOAD): $(OBJ_ELFLOAD) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_ELFUNLOAD:=\
    $(DIR_SRC)/elf/elfunload \

OBJ_ELFUNLOAD:=\
    $(DIR_SRC)/elf/elf.o \
    $(DIR_SRC)/elf/elfunload.o \
    $(DIR_SRC)/fudge/fudge.a \

$(BIN_ELFUNLOAD): $(OBJ_ELFUNLOAD) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_ELFLOAD) $(BIN_ELFUNLOAD)
OBJ:=$(OBJ) $(OBJ_ELFLOAD) $(OBJ_ELFUNLOAD)
