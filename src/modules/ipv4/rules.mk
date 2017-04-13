M:=\
    $(DIR_SRC)/modules/ipv4/ipv4.ko \

O:=\
    $(DIR_SRC)/modules/ipv4/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
