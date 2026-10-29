B:=\
    $(DIR_SRC)/snes/snes \

O:=\
    $(DIR_SRC)/snes/apu.o \
    $(DIR_SRC)/snes/cart.o \
    $(DIR_SRC)/snes/cpu.o \
    $(DIR_SRC)/snes/cx4.o \
    $(DIR_SRC)/snes/dma.o \
    $(DIR_SRC)/snes/dsp.o \
    $(DIR_SRC)/snes/input.o \
    $(DIR_SRC)/snes/main.o \
    $(DIR_SRC)/snes/port.o \
    $(DIR_SRC)/snes/ppu.o \
    $(DIR_SRC)/snes/snes.o \
    $(DIR_SRC)/snes/snes_other.o \
    $(DIR_SRC)/snes/spc.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

$(DIR_SRC)/snes/%.o: PP_FLAGS:=$(filter-out -std=c89 -pedantic, $(PP_FLAGS)) -std=c99
$(DIR_SRC)/snes/%.o: CC_FLAGS:=$(filter-out -std=c89 -pedantic, $(CC_FLAGS)) -std=c99

include $(DIR_MK)/bin.mk

ROOTBIN:=$(ROOTBIN) $(B)
