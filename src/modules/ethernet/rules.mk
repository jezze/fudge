M:=\
    $(DIR_SRC)/modules/ethernet/ethernet.ko \

O:=\
    $(DIR_SRC)/modules/ethernet/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
