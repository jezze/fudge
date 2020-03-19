M:=\
    $(DIR_SRC)/modules/ipv6/ipv6.ko \

N:=\
    $(DIR_SRC)/modules/ipv6/ipv6.ko.map \

O:=\
    $(DIR_SRC)/modules/ipv6/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
