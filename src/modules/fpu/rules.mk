M:=\
    $(DIR_SRC)/modules/fpu/fpu.ko \

N:=\
    $(DIR_SRC)/modules/fpu/fpu.ko.map \

O:=\
    $(DIR_SRC)/modules/fpu/main.o \
    $(DIR_SRC)/modules/fpu/fpu.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
