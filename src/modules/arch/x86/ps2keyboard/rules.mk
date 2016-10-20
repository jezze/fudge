M:=\
    $(DIR_SRC)/modules/arch/x86/ps2keyboard/ps2keyboard.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/ps2keyboard/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
