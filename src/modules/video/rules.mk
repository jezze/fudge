M:=\
    $(DIR_SRC)/modules/video/video.ko \

O:=\
    $(DIR_SRC)/modules/video/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
