M:=\
    $(DIR_SRC)/modules/syse/syse.ko \

N:=\
    $(DIR_SRC)/modules/syse/syse.ko.map \

O:=\
    $(DIR_SRC)/modules/syse/main.o \
    $(DIR_SRC)/modules/syse/syse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
