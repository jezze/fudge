M:=\
    $(DIR_SRC)/modules/tcp/tcp.ko \

O:=\
    $(DIR_SRC)/modules/tcp/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
