M:=\
    $(DIR_SRC)/modules/cpio/cpio.ko \

O:=\
    $(DIR_SRC)/modules/cpio/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
