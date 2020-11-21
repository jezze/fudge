M:=\
    $(DIR_SRC)/modules/ethernet/ethernet.ko \

N:=\
    $(DIR_SRC)/modules/ethernet/ethernet.ko.map \

O:=\
    $(DIR_SRC)/modules/ethernet/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/kmod.mk
