M:=\
    $(DIR_SRC)/modules/udp/udp.ko \

O:=\
    $(DIR_SRC)/modules/udp/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
