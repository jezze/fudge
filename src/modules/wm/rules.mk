M:=\
    $(DIR_SRC)/modules/wm/wm.ko \

O:=\
    $(DIR_SRC)/modules/wm/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
