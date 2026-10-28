B:=\
    $(DIR_SRC)/nes/nes \

O:=\
    $(DIR_SRC)/nes/apu.o \
    $(DIR_SRC)/nes/axrom.o \
    $(DIR_SRC)/nes/cnrom.o \
    $(DIR_SRC)/nes/colordreams.o \
    $(DIR_SRC)/nes/controller.o \
    $(DIR_SRC)/nes/cprom.o \
    $(DIR_SRC)/nes/cpu6502.o \
    $(DIR_SRC)/nes/emulator.o \
    $(DIR_SRC)/nes/gnrom.o \
    $(DIR_SRC)/nes/mapper.o \
    $(DIR_SRC)/nes/mapper185.o \
    $(DIR_SRC)/nes/mixer.o \
    $(DIR_SRC)/nes/mmc1.o \
    $(DIR_SRC)/nes/mmc3.o \
    $(DIR_SRC)/nes/mmu.o \
    $(DIR_SRC)/nes/nes.o \
    $(DIR_SRC)/nes/port.o \
    $(DIR_SRC)/nes/ppu.o \
    $(DIR_SRC)/nes/utils.o \
    $(DIR_SRC)/nes/uxrom.o \
    $(DIR_SRC)/nes/vrc1.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

$(DIR_SRC)/nes/%.o: PP_FLAGS:=$(filter-out -std=c89 -pedantic, $(PP_FLAGS)) -std=c99
$(DIR_SRC)/nes/%.o: CC_FLAGS:=$(filter-out -std=c89 -pedantic, $(CC_FLAGS)) -std=c99 -Wno-parentheses

include $(DIR_MK)/bin.mk
