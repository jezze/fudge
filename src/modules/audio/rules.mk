M:=\
    $(DIR_SRC)/modules/audio/audio.ko \

O:=\
    $(DIR_SRC)/modules/audio/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
