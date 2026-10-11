M:=\
    $(DIR_SRC)/modules/pic/pic.ko \

N:=\
    $(DIR_SRC)/modules/pic/pic.ko.map \

O:=\
    $(DIR_SRC)/modules/pic/main.o \
    $(DIR_SRC)/modules/pic/$(ARCH)/pic.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
