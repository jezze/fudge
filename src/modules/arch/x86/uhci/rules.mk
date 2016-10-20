M:=\
    $(DIR_SRC)/modules/arch/x86/uhci/uhci.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/uhci/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
