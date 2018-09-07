M:=\
    $(DIR_SRC)/modules/internals/internals.ko \

O:=\
    $(DIR_SRC)/modules/internals/main.o \
    $(DIR_SRC)/modules/internals/backend.o \
    $(DIR_SRC)/modules/internals/protocol.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
