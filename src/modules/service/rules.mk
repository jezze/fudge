M:=\
    $(DIR_SRC)/modules/service/service.ko \

N:=\
    $(DIR_SRC)/modules/service/service.ko.map \

O:=\
    $(DIR_SRC)/modules/service/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
