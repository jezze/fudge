M:=\
    $(DIR_SRC)/modules/tcp/tcp.ko \

N:=\
    $(DIR_SRC)/modules/tcp/tcp.ko.map \

O:=\
    $(DIR_SRC)/modules/tcp/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/kmod.mk
