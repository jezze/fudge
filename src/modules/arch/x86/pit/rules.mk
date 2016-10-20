M:=\
    $(DIR_SRC)/modules/arch/x86/pit/pit.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/pit/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
