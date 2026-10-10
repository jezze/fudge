M:=\
    $(DIR_SRC)/modules/pic/x86_64/pic.ko \

N:=\
    $(DIR_SRC)/modules/pic/x86_64/pic.ko.map \

O:=\
    $(DIR_SRC)/modules/pic/x86_64/main.o \
    $(DIR_SRC)/modules/pic/x86_64/pic.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
