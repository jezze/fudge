B:=\
    $(DIR_SRC)/gameboy/gameboy \

O:=\
    $(DIR_SRC)/gameboy/audio.o \
    $(DIR_SRC)/gameboy/cpu.o \
    $(DIR_SRC)/gameboy/gameboy.o \
    $(DIR_SRC)/gameboy/video.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

$(DIR_SRC)/gameboy/audio.o: CC_FLAGS:=$(filter-out -msoft-float, $(CC_FLAGS))
$(DIR_SRC)/gameboy/cpu.o: CC_FLAGS:=$(filter-out -pedantic, $(CC_FLAGS))
$(DIR_SRC)/gameboy/gameboy.o: CC_FLAGS:=$(filter-out -pedantic, $(CC_FLAGS))

include $(DIR_MK)/bin.mk
