M:=\
    $(DIR_SRC)/modules/vbe/vbe.ko \

N:=\
    $(DIR_SRC)/modules/vbe/vbe.ko.map \

O:=\
    $(DIR_SRC)/modules/vbe/main.o \
    $(DIR_SRC)/modules/vbe/videomode.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
