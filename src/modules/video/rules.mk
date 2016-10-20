M:=\
    $(DIR_SRC)/modules/video/video.ko \

O:=\
    $(DIR_SRC)/modules/video/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
