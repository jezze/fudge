M:=\
    $(DIR_SRC)/modules/audio/audio.ko \

N:=\
    $(DIR_SRC)/modules/audio/audio.ko.map \

O:=\
    $(DIR_SRC)/modules/audio/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
