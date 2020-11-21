M:=\
    $(DIR_SRC)/modules/ipv4/ipv4.ko \

N:=\
    $(DIR_SRC)/modules/ipv4/ipv4.ko.map \

O:=\
    $(DIR_SRC)/modules/ipv4/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/kmod.mk
