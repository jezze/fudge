M:=\
    $(DIR_SRC)/modules/hda/hda.ko \

N:=\
    $(DIR_SRC)/modules/hda/hda.ko.map \

O:=\
    $(DIR_SRC)/modules/hda/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
