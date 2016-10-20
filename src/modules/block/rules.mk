M:=\
    $(DIR_SRC)/modules/block/block.ko \

O:=\
    $(DIR_SRC)/modules/block/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
