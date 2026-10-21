B:=\
    $(DIR_SRC)/pop/pop \

O:=\
    $(DIR_SRC)/pop/data.o \
    $(DIR_SRC)/pop/midi.o \
    $(DIR_SRC)/pop/opl3.o \
    $(DIR_SRC)/pop/options.o \
    $(DIR_SRC)/pop/pop.o \
    $(DIR_SRC)/pop/port.o \
    $(DIR_SRC)/pop/sdl.o \
    $(DIR_SRC)/pop/seg000.o \
    $(DIR_SRC)/pop/seg001.o \
    $(DIR_SRC)/pop/seg002.o \
    $(DIR_SRC)/pop/seg003.o \
    $(DIR_SRC)/pop/seg004.o \
    $(DIR_SRC)/pop/seg005.o \
    $(DIR_SRC)/pop/seg006.o \
    $(DIR_SRC)/pop/seg007.o \
    $(DIR_SRC)/pop/seg008.o \
    $(DIR_SRC)/pop/seg009.o \
    $(DIR_SRC)/pop/seqtbl.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/image/image.a \
    $(DIR_LIB)/compression/compression.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

$(DIR_SRC)/pop/%.o: PP_FLAGS:=$(filter-out -std=c89 -pedantic, $(PP_FLAGS)) -std=c99
$(DIR_SRC)/pop/%.o: CC_FLAGS:=$(filter-out -std=c89 -pedantic, $(CC_FLAGS)) -std=c99

include $(DIR_MK)/bin.mk
