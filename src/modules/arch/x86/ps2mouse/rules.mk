M:=\
    $(DIR_SRC)/modules/arch/x86/ps2mouse/ps2mouse.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/ps2mouse/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
