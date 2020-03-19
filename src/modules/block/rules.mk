M:=\
    $(DIR_SRC)/modules/block/block.ko \

N:=\
    $(DIR_SRC)/modules/block/block.ko.map \

O:=\
    $(DIR_SRC)/modules/block/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
