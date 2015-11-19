BIN_ELFLOAD:=\
    $(SRC_PATH)/elf/elfload \

OBJ_ELFLOAD:=\
    $(SRC_PATH)/elf/elf.o \
    $(SRC_PATH)/elf/elfload.o \
    $(SRC_PATH)/fudge/ascii.o \
    $(SRC_PATH)/fudge/memory.o \
    $(SRC_PATH)/lib/file.o \

$(BIN_ELFLOAD): $(OBJ_ELFLOAD) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_ELFUNLOAD:=\
    $(SRC_PATH)/elf/elfunload \

OBJ_ELFUNLOAD:=\
    $(SRC_PATH)/elf/elf.o \
    $(SRC_PATH)/elf/elfunload.o \
    $(SRC_PATH)/fudge/memory.o \

$(BIN_ELFUNLOAD): $(OBJ_ELFUNLOAD) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_ELFLOAD) $(BIN_ELFUNLOAD)
CLEAN:=$(CLEAN) $(BIN_ELFLOAD) $(BIN_ELFUNLOAD) $(OBJ_ELFLOAD) $(OBJ_ELFUNLOAD)
