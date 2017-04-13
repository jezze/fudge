M:=\
    $(DIR_SRC)/modules/arch/x86/uhci/uhci.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/uhci/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
