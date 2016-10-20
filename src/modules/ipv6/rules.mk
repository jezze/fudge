M:=\
    $(DIR_SRC)/modules/ipv6/ipv6.ko \

O:=\
    $(DIR_SRC)/modules/ipv6/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
